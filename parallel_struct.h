#pragma once
#include <iostream>

#include "spectra.h"
#include "ms_dataset.h"

using namespace std;
/* 
 * definitions of several struct used in info translation
 */

/* info from master to slave */
/*
struct buckets_with_data
{
	int buckets_num;
	vector<int> idxes;
	vector<peak> data;
};
*/

/*
struct alloc_info
{
	int group_num;
	vector<buckets_with_data> assignments;
};
*/

/* 哈希key为索引的字典 */
struct dict_idx
{
	int num_proc;
	// 标识记录哈希码，这里是两位vector，建议用template替换
	vector<int> hash_key;
	int para_1;		// group_num
	vector<int> idx;
	ms_dataset data();
	// vector<spectra> data;
	// or vector<ms_dataset> data_all;

	dict_idx()
	{

	}
	
	dict_idx(int proc, vector<int> keys, vector<int> idxes)
	{
		num_proc = proc;
		hash_key = keys;
		idx = idxes;
	}
};

/* dict stored by num_proc */
struct dict_full
{
	int num_proc;
	int para_1;
	vector<ms_dataset> multi_datasets;
	vector<int> idx_full;			// not neccesory on slave nodes

	dict_full(int proc)
	{
		num_proc = proc;
	}

	void generate_idxf()
	{
		for (size_t i = 0; i < this->multi_datasets.size(); i++)
		{
			idx_full.insert(idx_full.end(), multi_datasets[i].data_idxes.begin(), multi_datasets[i].data_idxes.end());
			return;
		}
	}

	void append_datasets(ms_dataset ds)
	{
		this->multi_datasets.push_back(ds);
		return;
	}

	void fullfill_data(const ms_dataset& global_ds)
	{
		int count = this->multi_datasets.size();
		for (int i = 0; i < count; ++i)
		{
			/* for every datasets, idxes are existed, while data is missing */
			this->multi_datasets[i].filldata_by_idx(global_ds);
		}
		return;
	}
};

/* info sent back to master node from slave node */
struct report_rho
{
	// int group_num;
	vector<int> idx;
	vector<double> dpc_para;
};

struct report_delta
{
	// int group_num;
	vector<int> idx;
	vector<double> dpc_para_1;
	vector<int> dpc_para_2;
};

/*
template<para_type_1, para_type_2>
struct report_info
{
	// int group_num;
	int valid_num_signal;	// to indicate how many vector are meaningful
	vector<para_type_1> idx;
	vector<para_type_2> dpc_para;
};
*/

void alloc_assign(vector<dict_idx> &idx_table, int num_proc_comp);

vector<dict_full> alloc_assign_full(vector<dict_idx> idx_table, int num_proc_comp);