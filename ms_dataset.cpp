#include <assert.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "dpc_func.h"
#include "ms_dataset.h"
#include "graph.h"

using namespace std;

/* load data from disk through file name */
void ms_dataset::load_data_file(string file_names)
{
	
	cout << "reading file ... " << file_names << endl;
	char FILE_NAME[128];
	int j;
	for (j = 0; j < file_names.length(); j++)
	{
		FILE_NAME[j] = file_names.at(j);
	}
	FILE_NAME[j] = '\0';
	
	ifstream in(FILE_NAME);
	if(! in.is_open()){
		cout << "Error opening file " << FILE_NAME << endl; 
		exit(1); 
	}	

	int spectra_num = 0;
	string file_name_current(file_names);
	char line_buff[120];

	string spectra_name;
	double precursor_mz;
	int charge;
	vector<peak> ions_tmp;

	while( !in.eof() )
	{
		/*
		BEGIN IONS
		TITLE=TCGA-AA-3715-01A-22_W_VU_20120821_A0218_2E_R_FR01.7.7.2
		RTINSECONDS=907.0923
		PEPMASS=604.794006347656 616.510498046875
		CHARGE=2+
		193.8873444 3.6720590591
		END IONS
		*/
		in.getline (line_buff, 120);
		
		if (strstr(line_buff,"BEGIN IONS") != NULL)
		{
			ions_tmp.clear();
		}
		else if (strstr(line_buff,"TITLE") != NULL)
		{
			/* or extract spectra name */
			spectra_name.assign(line_buff);
		}
		else if (strstr(line_buff, "PEPMASS") != NULL)
		{
			// istringstream ss(line_buff);
			//ss >> precursor_mz;
			double tmp;
			sscanf(line_buff, "PEPMASS=%lf %lf", &precursor_mz, &tmp);
		}
		else if (strstr(line_buff, "CHARGE") != NULL)
		{
			// istringstream ss(line_buff);
			//ss >> charge;
			sscanf(line_buff, "CHARGE=%d+", &charge);
		}
		else if (line_buff[0] >= '0' && line_buff[0] <= '9')
		{
			double mass_value;
			double intensity;
			/* read data */
			istringstream ss(line_buff);
			ss >> mass_value >> intensity;
			//ss >> intensity;
			peak peak_tmp(mass_value, intensity);

			// num_peaks_tmp ++;
			ions_tmp.push_back(peak_tmp);
		}
		else if (strstr(line_buff,"END IONS") != NULL)
		{
			/* collect all data and write back */
			spectra spe_tmp(file_name_current, spectra_name, precursor_mz, charge, ions_tmp);
			this->data.push_back(spe_tmp);
			this->data_idxes.push_back(spectra_num);
			spectra_num++;
		}
		else 
			continue;
	}
	in.close();
	
	this->data_size = this->data.size();
	return;
}

/* load data from memory */
void ms_dataset::load_data_mem(vector<spectra> &spectras_tmp, vector<int> &idxes_tmp)
{
	/* to find a way to initialize the vector directly from existed vector */
	for (int i = 0; i < data_size; ++i)
	{
		this->data_idxes.push_back(idxes_tmp[i]);
		this->data.push_back(spectras_tmp[i]);
	}
	this->data_size = this->data.size();
	return;
}


void ms_dataset::append_data(spectra single_data, int single_idx)
{
	this->data.push_back(single_data);
	this->data_idxes.push_back(single_idx);
	this->data_size = this->data.size();
	return;
}


void ms_dataset::generate_graph(int method)
{
	// cout << "in generate_graph" << endl;
	for (int i = 0; i < data_size; ++i)
	{
		ms_graph.insert_vertex(i);
	}
	// edge_num = 0;
	for (int i = 0; i < data_size; ++i)
	{
		for (int j = i + 1; j < data_size; ++j)
		{
			double dis_tmp = get_point_dis(this->data[i], this->data[j], method);
			// 第一个阈值
			if (dis_tmp > 0.001)
			{
				cout << "distance larger than 0.0001" << endl;
				ms_graph.insert_edge(i, j, dis_tmp);
				ms_graph.insert_edge(j, i, dis_tmp);
			}
		}
	}
	return;
}

void ms_dataset::get_dc_reservoir()
{
	/* after generate graph, the top 0.02 distance */

	dc = 0.1;
}
	

void ms_dataset::get_dc_merge()
{
	/* recv all dc from slave nodes, and calculate the mean_dc*/

}

// get rho
void ms_dataset::get_rho(double dc, bool is_guass)
{
	// this->rho = new vector<double>[this->data.size()];

	edge<double> *pmove;
	for (int i = 0; i < data_size; ++i)
	{
		double rho_tmp = 0.0;
		pmove = this->ms_graph.vertex_table[i].padj_edge;
		while(pmove)
		{
			/* weight is distance, not similarity*/
			rho_tmp = rho_tmp + exp(-pow((pmove->dist_weight / dc), 2));
			// if is_guass = false, then use other function
			pmove = pmove->next_edge;
		}
		this->rho.push_back(rho_tmp);
	}
	return;
}

void ms_dataset::assign_rho(vector<double> rho_o)
{
	assert(this->rho.size() == 0);
	assert(this->data.size() == rho_o.size());

	this->rho = rho_o;
	return;
}

void ms_dataset::refine_rho(vector<double> &rho_global)
{
	for (int i = 0; i < data_size; ++i)
	{
		rho_new[i] = rho_global[this->data_idxes[i]];
	}
	return;
}

// get delta and upslope
void ms_dataset::get_delta()
{
	// this->delta = new double[this->data.size()];
	// this->upslope = new int[this->data.size()];
	edge<double> *pmove;
	for (int i = 0; i < data_size; ++i)
	{
		double delta_tmp = 1.0;		// the maimum value
		int upslope_tmp = -1;
		pmove = this->ms_graph.vertex_table[i].padj_edge;
		while(pmove)
		{
			if (rho_new[pmove->dest_node] > rho_new[i] && pmove->dist_weight < delta_tmp)
			{
				delta_tmp = pmove->dist_weight;
				upslope_tmp = pmove->dest_node;
			}
		}
		
		this->delta.push_back( delta_tmp );
		this->upslope.push_back( upslope_tmp );
	}
	return;
}

void ms_dataset::assign_delta(vector<double> delta_o, vector<int> upslope_o)
{
	assert(this->delta.size() == 0);
	assert(this->data.size() == delta_o.size());
	assert(delta_o.size() == upslope_o.size());

	this->delta = delta_o;
	this->upslope = upslope_o;

	return;
}

void ms_dataset::decide_dpc(int method)
{
	switch(method)
	{
		/* double threshold */
		case 1:
			this->decision = decide_double_thres(this->delta, this->rho, this->cluster_num, this->data_size);
		/* decide by intervals */
		case 2:
			this->decision = decide_by_gap(this->delta, this->rho, this->cluster_num, this->data_size);
		/* decide by multipler thresholds */
		case 3:
			this->decision = decide_multi_thres(this->delta, this->rho, this->upslope, this->cluster_num, this->data_size);
		/* decide by painting */
		case 4:
			this->decision = decide_by_graph(this->delta, this->rho, this->cluster_num, this->data_size);
		default:
			break;
	}
	return;
}

void ms_dataset::assign_cluster()
{    
    vector<rho_cluster> data_rho;

    int num = this->rho.size();
    for (int i = 0; i < num; ++i)
    {
        rho_cluster tmp(this->rho[i], i);
        data_rho.push_back(tmp);
    }

    sort(data_rho.begin(), data_rho.end(), comp_rho);

    for (int i = 0; i < num; ++i)
    {
        if (this->decision[data_rho[i].order] == -1)
        {
            this->decision[data_rho[i].order] = this->decision[this->upslope[data_rho[i].order]];
        }
    }

    return;
}


void ms_dataset::get_halo()
{
	// get_halo(this->decision, this->graph, this->rho, this->dc, this->cluster_num);
	return;
}

void ms_dataset::filldata_by_idx(const ms_dataset& global_ds)
{
	assert(this->data_idxes.size() != 0 && this->data.size() == 0);

	int count = this->data_idxes.size();
	for (int i = 0; i < count; ++i)
	{
		int idx_tmp = this->data_idxes[i];
		spectra spe_tmp(global_ds.data[idx_tmp]);
		this->data.push_back(spe_tmp);
	}
	return;
}