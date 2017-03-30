/*
 * author: yuan
 * time: 2017.2.20
 * all old and useless codes are removed
 * all varialbles and classes are regulized
 * based on min_hash_mainly.cpp
 */
/*
 * modified: 2017.3.16
 * class was used to make the code more efficient
 */
/* 
 * modified: 2017.3.22
 * derived type in MPI
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
// #include <unordered_map>

#include "mpi.h"

#include "ms_dataset.h"
#include "parallel_struct.h"
#include "utils.h"
#include "assignment.pb.h"
 
/* the whole program is based on mpi */
int main(int argc, char** argv)
{
	double start_time = MPI_Wtime();
	int myid,num_procs;
    MPI_Status status;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    //  MPI_Get_processor_name(processor_name,&namelen);
	 
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	//GOOGLE_PROTOBUF_VERIFY_VERSION;

	// int m_groups;
	// int k_funcs;
	// data normalization related
	// int min_mass_global;
	// int max_mass_global;
	/* after these four variables are assigned */
	// MPI_Bcast(&m_groups, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// MPI_Bcast(&k_funcs, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	// MPI_Bcast(&min_mass_global, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// MPI_Bcast(&max_mass_global, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	char input_dir[100];
	if (myid == 0)
	{
		if (num_procs == 1) {
			cout << "error! procs is not enough! " << endl;
			return 0;
		}

		// load all data, get basical statistical info, memory sharing later
		cout << "input directory: ";
		cin >> input_dir;
	}
	MPI_Bcast(input_dir, 100, MPI_CHAR, 0, MPI_COMM_WORLD);

	vector<string> file_names;
	char current_dir[2] = "\0";
	char file_type[4] = "mgf";
	printdir(input_dir, file_names, current_dir, file_type);
	// after this, file_names are kept in all nodes

	if(myid == 0)
	{
		if (file_names.size() == 0)
		{
			cout << "None file available! Retry";
			return 0;
		}
		cout << "number of files: " << file_names.size() << endl;
		cout << "number of procs: " << num_procs << endl;
	}
	int file_num = file_names.size();

	/* assignments of partition distributed to all nodes
	 * hash values of every data point calculated
	 */
	/* begin and end pointer could be calculated locally for every node 
	 * note that when file_num is less than num_procs, this method still work
	 */
	int begin_slave_idx;
	int end_slave_idx;

	int smaller_num_file = int(floor(double(file_num) / (num_procs - 1)));
	int larger_num_file  = smaller_num_file + 1;

	int seperate_idx = file_num % (num_procs - 1);

	if (myid > 0 && myid <= seperate_idx)
	{
		begin_slave_idx = (myid - 1) * larger_num_file;
		end_slave_idx = myid * larger_num_file;
	}
	else if (myid > seperate_idx)
	{
		begin_slave_idx = seperate_idx * larger_num_file + (myid - seperate_idx - 1) * smaller_num_file;
		end_slave_idx = seperate_idx * larger_num_file + (myid - seperate_idx) * smaller_num_file;
	}
		 
	if (myid == 0)
	{
		/* generate ramdom permutation parameters*/
		// vector<int> aaa = generate_rand_paras(m_groups * k_funcs, max_mass_global);
		// vector<int> bbb = generate_rand_paras(m_groups * k_funcs, max_mass_global);
	}
	// MPI_Bcast(aaa.data(), m_groups * k_funcs, MPI_INT, 0, MPI_COMM_WORLD);
	// MPI_Bcast(bbb.data(), m_groups * k_funcs, MPI_INT, 0, MPI_COMM_WORLD);
	
	/* load the whole dataset to "data" on master node */
	ms_dataset global_ms;
	int nsample_global;
	// vector<string> data_names;
	double before_loading_time = MPI_Wtime();
	if(myid ==0)
	{
		for (int i = 0; i < file_names.size(); ++i)
		{
			global_ms.load_data_file(file_names[i]);
			// load_ms_data(global_ms, data_names, file_names[i]);
		}
	}
	else
	{
		for (int i = begin_slave_idx; i < end_slave_idx; ++i)
		{
			global_ms.load_data_file(file_names[i]);
			// load_ms_data(global_ms, data_names, file_names[i]);
		}
	}
	nsample_global = global_ms.data_size;
	double after_loading_time = MPI_Wtime();
	cout << "Time of loading data: " << after_loading_time - before_loading_time << "s" << endl;
	cout << "Total number of data: " << nsample_global << endl;

	for (int i = 0; i < global_ms.data_size; i++)
	{
		global_ms.data[i].select_peaks();
		global_ms.data[i].normalize_spectra();
	}
	double after_preprocess_time = MPI_Wtime();
	cout << "Time of data preprocessing: " << after_preprocess_time - after_loading_time << "s" << endl;
	
	MPI_Bcast(&nsample_global, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	/* dc estimation should be put here, maybe parallely */
	// dc = getdc_reservoir(vector< > &data)
	// MPI_Bcast(&dc, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	double dc = 0.1;		// hand defined

	/* key variables */
	// vector<hash_table> ms_complete_ht;
	/*
	typedef vector<hash_map<vector<int>, set> > global_ms_ht;
	vector<hash_map<vector<int>, set> > ms_ht;	// size is m
	*/
	typedef multimap<vector<int>, int> heu_hashmap;
	heu_hashmap hhm_ms; 
	if(myid > 0)
	{
		// for (int i = 0; i < m_groups; ++i)
		// {
		
		// now, i decide to do the partition process only on master node 
		// hhm_ms = partition_heuristic(global_ms->data);
			/*
			hash_map<int, set> ms_hm_tmp;
			vector<hash_map<int, set> > group_hm_tmp;
			for (int j = 0; j < k_funcs; ++j)
			{
				int aaa_current = aaa[i * m_groups + j];
				int bbb_current = bbb[i * m_groups + j];

				ms_hm_tmp = multims_hash(global_ms->data, aaa_current, bbb_current);
				group_hm_tmp.push_back(ms_hm_tmp);
			}
			hash_map<vector<int>, set> ms_hm_group = merge_group_ht(group_hm_tmp);
			ms_ht.push_back(ms_hm_group);
		}
		*/
		// MPI_Send(hhm_ms);
		// merged hash value of local data
	}



	// hash table analysis and assignments distribution
	vector<dict_full> ms_assign;
	if (myid == 0)
	{
		/* code */
		// vector<heu_hashmap> hhm_vec;
		// heu_hashmap hhm_ms_tmp;
		// vector<hash_map<vector<int>, set> > ms_ht_proc;
		// for (int i = 1; i < num_procs; ++i)
		// {
			// MPI_Recv(hhm_ms_tmp);
			//ms_ht = merge_ms_ht(ms_ht, ms_ht_proc);
		// }
		// hhm_ms = merge_par_heu(hhm_vec);

		hhm_ms = partition_heuristic(global_ms.data);

		cout << "hash table analysis done!\n";
		cout << "hash table size: " << endl;
		/*
		for (int i = 0; i < m_groups; ++i)
		{
			cout << "\tgroup number: " << i << endl;
			cout << "\thash table size: " << ms_ht[i].size() << endl;
			// cout << "max and min num in hash_table" << ms_complete_ht[i].max_capacity << " & " << ms_complete_ht[i].min_capacity;
		}
		*/

		// allocate assignments
		vector<dict_idx> assign_distri = trans_table_dict(hhm_ms);
		ms_assign = alloc_assign_full(assign_distri, num_procs - 1);
		for (size_t i = 0; i < ms_assign.size(); i++)
		{
			ms_assign[i].generate_idxf();
		}
		double after_assign_time = MPI_Wtime();
		cout << "Time of assignment planing: " << after_assign_time - after_preprocess_time << "s" << endl;

		// vector<vector<int> > idx_in_distri(num_procs, vector<int>());
		// get_paras_distri(assign_distri, idx_in_distri);
		// int count_assign = assign_distri.size();
		// we should send all dataset to procs onnce
		for (int i = 1; i < num_procs; ++i)
		{
			// 6vector<ms_dataset> assign_data; 
			// fullfill the data set, ms_assign[i]
			string assign_str = encode_assign(ms_assign[i]);
			cout << "proc " << i << ": assign size is " << assign_str.size() << endl;
			MPI_Send(assign_str.c_str(), assign_str.size(),MPI_CHAR, i, i, MPI_COMM_WORLD);
			// MPI_Send(ms_assign[i].multi_datasets, 1, i, i, MPI_COMM_WORLD);
			/* 这里，ms_assign[i]包含一个ms_dataset数组和一个完整的索引向量，但目前ms_dataset仅包含索引没有数据 */
		}
		double after_distribute_time = MPI_Wtime();
		cout << "Time of distribution and data translation: " << after_distribute_time - after_assign_time << "s" << endl;
	}

	/* rho calculations on slave procs */
	vector<ms_dataset> data_local;
	vector<double> rho_final(nsample_global, 0.0);
	if (myid > 0)
	{
		double time_before_receive = MPI_Wtime();
		int assign_length;
		MPI_Probe(0, myid, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_CHAR, &assign_length);
		char assign_str[assign_length];
		MPI_Recv(assign_str, assign_length, MPI_CHAR, 0, myid, MPI_COMM_WORLD, &status);
		dict_full data_tmp = decode_assign(string(assign_str));
		for (size_t i = 0; i < data_tmp.multi_datasets.size(); i++)
		{
			data_local.push_back(data_tmp.multi_datasets[i]);
		}
		cout << "proc " << myid << " has collected all data assigned to it! " << endl;
		double time_after_receive = MPI_Wtime();
		cout << "Time of data receiving on proc " << myid << " is " << time_after_receive - time_before_receive << "s" << endl;

		// vector<report_rho> rho_sendback;
		/* 由于定义了dict_full这个复杂类，这里可以直接回传vector */
		vector<double> rho_sendback;
		for (int i = 0; i < data_local.size(); ++i)
		{
			data_local[i].generate_graph(5);		// dot
			data_local[i].get_rho(dc, true);		// dc and is_guass

			rho_sendback.reserve(rho_sendback.size() + data_local[i].rho.size());
			rho_sendback.insert(rho_sendback.end(), data_local[i].rho.begin(), data_local[i].rho.end());
		}
		MPI_Send(rho_sendback.data(), rho_sendback.size(), MPI_DOUBLE, 0, myid, MPI_COMM_WORLD);
		double time_after_rho = MPI_Wtime();
		cout << "Time of rho calculation and bcast on proc " << myid << " is " << time_after_rho - time_after_receive << "s" << endl;

		MPI_Bcast(rho_final.data(), nsample_global, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		/* we have recv global rho */
	}
	
	/* rho calculation */
	if (myid == 0)
	{
		/* recv the rho value from slave nodes and merge all rho */
		/* two key variables */
		// vector<double> rho_mat;		// n_sample * m_groups
		/* 理论上，每一个数据点的rho值都会出现k次，k是选定的最高的几个峰的个数 */
		/* 其实这个也同样适合m_group，因为我们的操作只是要求最大值，是否对齐不重要 */
		// vector<double> rho_final(nsample_global, 0.0);
		double time_before_rho = MPI_Wtime();
		for (int i = 1; i < num_procs; ++i)
		{
			vector<int> idx_tmp = ms_assign[i].idx_full;
			int sendback_length = idx_tmp.size();

			vector<double> rho_tmp(sendback_length, -1);
			
			MPI_Recv(rho_tmp.data(), sendback_length, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &status);

			/* merge this rho_report into rho_mat*/
			for (int j = 0; j < sendback_length; ++j)
			{
				if (rho_tmp[j] > rho_final[idx_tmp[j]])
				{
					rho_final[idx_tmp[j]] = rho_tmp[j];
				}
				/*
				int group_tmp = rho_report[i]->group_num;
				int size_tmp = rho_report[i]->idx.size()
				for (int j = 0; j < size_tmp; ++j)
				{
					rho_mat[ rho_report[i]->idx[j], group_tmp ] = rho_report[i]->dpc_para[j];
				}
				*/
			}
		}
		double time_after_rho = MPI_Wtime();
		cout << "Time of rho collection and merging on master proc is: " << time_after_rho - time_before_rho << "s" << endl;


		/* after we recv all rho data, merge them by the "max-rho" principles */
		/*
		vector<double> rho_final(n_sample, -1);
		for (int i = 0; i < n_sample; ++i)
		{
			for (int j = 0; j < m_groups; ++j)
			{
				if (rho_mat[i][j] > rho_final[i])
				{
					rho_final[i] = rho_mat[i][j];
				}
			}
		}
		*/

		// we should add this rho_final to the class
		MPI_Bcast(rho_final.data(), nsample_global, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		global_ms.assign_rho(rho_final);
	}
	
	/* delta calculation */
	vector<double> delta_final(nsample_global, 10);			// global maximum value should be assigned
	vector<int> upslope_final(nsample_global, -1);
	if (myid == 0)
	{
		/* recv the delta value from slave nodes and merge all delta */
		/* two key variables */
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
					delta_final[idx_tmp[j]] = delta_tmp[j];
					upslope_final[idx_tmp[j]] = upslope_tmp[j];
				}
			}
		}
		double time_after_delta = MPI_Wtime();
		cout << "Time of delta calculation and merging on master node is: " << time_after_delta - time_before_delta << "s" << endl;

		MPI_Bcast(delta_final.data(), nsample_global, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(upslope_final.data(), nsample_global, MPI_INT, 0, MPI_COMM_WORLD);
		
		global_ms.assign_delta(delta_final, upslope_final);
	}

	/* delta calculations on slave procs and send */
	if (myid > 0)
	{
		// vector<report_delta> delta_sendback;
		double time_before_delta = MPI_Wtime();
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
		MPI_Send(delta_sendback.data(), delta_sendback.size(), MPI_DOUBLE, 0, myid, MPI_COMM_WORLD);
		MPI_Send(upslope_sendback.data(), upslope_sendback.size(), MPI_INT, 0, myid, MPI_COMM_WORLD);
		// maybe mpi_pack should be used here
		double time_after_delta = MPI_Wtime();
		cout << "Time of delta calculation on proc " << myid << " is " << time_after_delta - time_before_delta << "s" << endl;

		// vector<double> delta_final(nsample_global, -1);
		MPI_Bcast(delta_final.data(), nsample_global, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		/* we have recv global rho */
	}

	/* decison make and clustering recursively */
	if(myid == 0)
	{
		// decide and assign 
		/* making decison */
		double time_before_cluster = MPI_Wtime();
		global_ms.decide_dpc(3);		// multipler of rho and delta
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
		cout << "n_sample: " << nsample_global << endl;
		// cout << "data_names size: " << data_names.size() << " data_names sample: " << data_names[0] << endl;
		// cout << "decision size: " << decision.size() << " decision sample: " << decision[0] << endl;
		ofstream out(OUTPUT_NAME);
		if(out.is_open()){
			cout<<"Prepare to write the result ... ... "<<endl;
			out << "file_name,spectra_name,cluster" << endl;
			for (int i = 0; i < global_ms.data.size()	; ++i)
			{
				// out<< i << endl;
				out << global_ms.data[i].file_name << "," << global_ms.data[i].spectra_name << "," << global_ms.decision[i] << endl;
			}
			out.close();
		}
	}
	
	MPI_Finalize();
	return 0;
}
