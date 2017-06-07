/*
 * serial version of ms_dpc for easier debugging
 * 2017-6-7
 * @author: Yuan
 */

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <assert.h>
#include <vector>
#include <math.h>
#include <algorithm>
#include <map>
#include "ms_dataset.h"
#include "parallel_struct.h"
#include "utils.h"

int main(int argc, char** argv)
{
	int num_procs = 10;

	/* load data from files specified */
	char input_dir[100];
	cout << "input directory: ";
	cin >> input_dir;

	vector<string> file_names;
	char current_dir[2] = "\0";
	char file_type[4] = "mgf";
	printdir(input_dir, file_names, current_dir, file_type);

	int file_num = file_names.size();
	if (file_num == 0)
	{
		cout << "None file available! Retry";
		return 0;
	}
	cout << "number of files: " << file_num << endl;

	ms_dataset global_ms;
	int nsample_global;
	for (int i = 0; i < file_names.size(); ++i)
	{
		global_ms.load_data_file(file_names[i]);
	}
	nsample_global = global_ms.data_size;

	/* data preprocessing */
	for (int i = 0; i < nsample_global; i++)
	{
		global_ms.data[i].select_peaks();
		global_ms.data[i].normalize_spectra();
	}
	
	double dc = 1.0;		// defined by hand

	/* split the dataset into several parts according to the highest peaks */
	typedef multimap<vector<int>, int> heu_hashmap;
	heu_hashmap hhm_ms; 

	vector<dict_full> ms_assign;
	hhm_ms = partition_heuristic(global_ms.data);

	cout << "hash table size: " << hhm_ms.size() << endl;
		 
	/* allocate assignments */
	vector<dict_idx> assign_distri = trans_table_dict(hhm_ms);
	cout << "after trans_table_dict, size of unique hash-table key: " << assign_distri.size() << endl;
	
	/* 0425 write the hash table into file, according to syntax as key1 - key2 - table_size */
	char OUT_HT[64] = "hash_table_test.csv";
	ofstream out(OUT_HT);
	if(out.is_open()){
		out << "precursor value * 0.33,mz value * 2, size of hash table" << endl;
		for (int i = 0; i < assign_distri.size(); ++i)
		{
			out << assign_distri[i].hash_key[0] << "," << assign_distri[i].hash_key[1] << "," << assign_distri[i].idx.size() << endl;
		}
		out.close();
	}

	ms_assign = alloc_assign_full(assign_distri, num_procs - 1);
	cout << "after alloc_assign_full, ms_assign size: " << ms_assign.size() << endl;
	/* 注意，这一步结束后，数据尚未填充 */
	for (size_t i = 0; i < ms_assign.size(); i++)
	{
		ms_assign[i].fullfill_data(global_ms);
		ms_assign[i].generate_idxf();
	}
	double after_assign_time = MPI_Wtime();
	cout << "Time of assignment planing: " << after_assign_time - after_preprocess_time << "s" << endl;

	/* encode, send, recieve, decode */
	vector<ms_dataset> data_local;
	vector<double> rho_final(nsample_global, 0.0);
	cout << "proc " << myid << " has collected all data assigned to it! " << endl;
	// cout << "proc 1 verify: " << data_local.size() << " & " << data_local[0].data.size() << " & " << data_local[0].data[0].ions.size() << endl;
	double time_after_receive = MPI_Wtime();
	cout << "Time of data receiving on proc " << myid << " is " << time_after_receive - time_before_receive << "s" << endl;

	// vector<report_rho> rho_sendback;
	/* ÓÉÓÚ¶¨ÒåÁËdict_fullÕâ¸ö¸´ÔÓÀà£¬ÕâÀï¿ÉÒÔÖ±½Ó»Ø´«vector */
	vector<double> rho_sendback;
	for (int i = 0; i < data_local.size(); ++i)
	{
		// cout << "before graph, parameters: " << data_local[i].data.size() << endl;
		data_local[i].generate_graph(5);		// dot
		data_local[i].get_rho(dc, true);		// dc and is_guass

		rho_sendback.reserve(rho_sendback.size() + data_local[i].rho.size());
		rho_sendback.insert(rho_sendback.end(), data_local[i].rho.begin(), data_local[i].rho.end());
	}

	for (int j = 0; j < sendback_length; ++j)
	{
		if (rho_tmp[j] > rho_final[idx_tmp[j]])
		{
			// cout << "larger rho, " << j << endl;
			rho_final[idx_tmp[j]] = rho_tmp[j];
		}
	}

	/* delta */
	vector<double> delta_sendback;
	vector<int> upslope_sendback;
	for (int i = 0; i < data_local.size(); ++i)
	{
		/* code */
		/* change the local rho value based on rho_final */
		data_local[i].refine_rho(rho_final);
		data_local[i].get_delta();

		// report_delta delta_tmp;
		// fullfill it

		// rho_sendback.push_back(delta_tmp);
		delta_sendback.insert(delta_sendback.end(), data_local[i].delta.begin(), data_local[i].delta.end());
		upslope_sendback.insert(upslope_sendback.end(), data_local[i].upslope.begin(), data_local[i].upslope.end());
	}

	vector<double> delta_final(nsample_global, 1);			// global maximum value should be assigned
	vector<int> upslope_final(nsample_global, -1);

	double time_before_delta = MPI_Wtime();
	for (int i = 1; i < num_procs; ++i)
	{
		vector<int> idx_tmp = ms_assign[i].idx_full;
		int sendback_length = idx_tmp.size();

		vector<double> delta_tmp(sendback_length, 10);		
		vector<int> upslope_tmp(sendback_length, -1);

		MPI_Recv(delta_tmp.data(), sendback_length, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &status);
		MPI_Recv(upslope_tmp.data(), sendback_length, MPI_INT, i, i, MPI_COMM_WORLD, &status);

		/* merge this rho_report into rho_mat*/
		for (int j = 0; j < sendback_length; ++j)
		{
			if (delta_tmp[j] < delta_final[idx_tmp[j]])
			{
				// cout << "less delta: " << j << endl;
				delta_final[idx_tmp[j]] = delta_tmp[j];
				upslope_final[idx_tmp[j]] = upslope_tmp[j];
			}
		}
	}
	double time_after_delta = MPI_Wtime();
	cout << "Time of delta calculation and merging on master node is: " << time_after_delta - time_before_delta << "s" << endl;

	/* decision */
	global_ms.decide_dpc(3);		// multipler of rho and delta
	// cout << "after decide dpc" << endl;
	 
	global_ms.assign_cluster();
	 
	double time_after_cluster = MPI_Wtime();
	cout << "Time of making decision and assigning labels is: " << time_after_cluster - time_before_cluster << "s" << endl;
	
	// end_time=clock();
	// cout << "Assignment done! Using time: " << double(end_time - decision_time) / CLOCKS_PER_SEC << "s" << endl;
	
	// cout << "Total uing time: " << ((double)(end_time - start_time)) / CLOCKS_PER_SEC << "s" << endl;
	// cout << "decision size: " << decision.size() << endl;
	/* write to file */
	FILE *output;
	char OUTPUT_NAME[100] = "ms_result_";
	strcat(OUTPUT_NAME, input_dir);
	strcat(OUTPUT_NAME, ".csv");
	cout << "out file name: " << OUTPUT_NAME << endl;
	// cout << "n_sample: " << nsample_global << endl;
	// cout << "data_names size: " << data_names.size() << " data_names sample: " << data_names[0] << endl;
	// cout << "decision size: " << decision.size() << " decision sample: " << decision[0] << endl;
	ofstream out(OUTPUT_NAME);
	if(out.is_open()){
		cout<<"Prepare to write the result ... ... "<<endl;
		out << "file_name;spectra_name;cluster;rho;delta" << endl;
		for (int i = 0; i < global_ms.data.size()	; ++i)
		{
			// out<< i << endl;
			out << global_ms.data[i].file_name << ";" << global_ms.data[i].spectra_name;
			out << ";" << global_ms.decision[i] << ";" << global_ms.rho[i] << ";" << global_ms.delta[i] << endl;
		}
		out.close();
	}
	double end_time = MPI_Wtime();
	cout << "All time cost: " << end_time - start_time << "s" << endl;

	return 0;
}