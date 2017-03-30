#pragma once
#include "spectra.h"
#include "graph.h"

using namespace std;

class ms_dataset
{
public:
	// if num is not avaliable, would be set to 0
	ms_dataset()
	{
		cluster_num = 0;
	}

	ms_dataset(vector<int> idx_o)
	{
		data_idxes = idx_o;
		cluster_num = 0;
	}

	ms_dataset(int num, int group_num = -1, int bucket_num = -1)
	{
		data_size = num;
		fun_group_num = group_num;
		hash_bucket_num = bucket_num;
	}

	~ms_dataset()		// delete operation
	{

  	}

	void load_data_file(string file_names);
	void load_data_mem(vector<spectra> &data_tmp, vector<int> &idxes_tmp);
	void append_data(spectra single_data, int single_idx);
	void generate_graph(int method);

	void get_dc_reservoir();
	void get_dc_merge();	// on master node

	void get_rho(double dc, bool is_guass);
	// void get_rho_merge();	// master node
	void assign_rho(vector<double> rho_o);
	void refine_rho(vector<double> &rho_global);

	void get_delta();
	// void get_delta_merge();
	void assign_delta(vector<double> delta_o, vector<int> upslope_o);

	// poly-morphic
	void decide_dpc(int method);
	void assign_cluster();

	void get_halo();
	
public:
	// location in the global hash table
	int fun_group_num;
	int hash_bucket_num;

	// data and data related info
	int data_size;
	int data_dim;		// if exists
	vector<int> data_idxes;
	vector<spectra> data;

	// middle variables
	graph<int, double> ms_graph;

	// results vectors
	double dc;
	vector<double> rho;
	vector<double> rho_new;
	vector<double> delta;
	vector<int> upslope;

	int cluster_num;
	vector<int> decision;
};