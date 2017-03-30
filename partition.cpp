#include <map>
// #include <hash_map>
// #include <unordered_map>
#include <vector>
#include <algorithm>
#include <set>
#include <math.h>

#include "ms_dataset.h"
#include "parallel_struct.h"

using namespace std;

map<int, int> single_idx;
map<vector<int>, int> group_idx;

// hash_map<int, int> ms_map;

/*
struct str_hash{
	size_t operator()(const string& str) const
	{

		return size_t(_h);
	}
}
*/

/*
struct compare_str{
	bool operator()(const char* p1, const char* p2) const{
		return strcmp(p1, p2) == 0;
	}
}
*/

/*
 * get the hash value for single data
 * & get the hash value for local ms_dataset
 * template should be used for the type of data point is different
 * at least spectra type
int ms_hash(vector<int> point)
{
	/* get hash value for every preprocessed data point 
	hash_map<int, set<int> > hashed_ms;
	// hashed value & corresponded idxes
}

hash_map<int, set<int> > multims_hash(ms_dataset local_ms, int aaa, int bbb)
{
	set<int> ms_hash_set;                                                                                                        a
	for (int i = 0; i < count; ++i)
	{
		int hash_value = ms_hash(local_ms[i])
	}
	

	return local_hm;
}
*/

// the merged 
/*
hash_map<vector<int>, set<int> > merge_group_ht(vector<hash_map<int, set<int> > > group_hm_tmp)
{
	/* merge results from different hash functions in one group 
	hash_map<vector<int>, set<int> > merged_hm;		// multi-hashed_values and data_idx

	return merged_hm; 
}
*/

/* marge 
typedef vector<hash_map<vector<int>, set<int> > > global_ms_ht;
global_ms_ht merge_multig_ht(global_ms_ht ht_ori, global_ms_ht ht_new)
{
	int m_groups = ht_ori.size();
	global_ms_ht ht_merged;
	for (int i = 0; i < m_groups; ++i)
	{
		
	}

	return ht_merged;
}
*/

/* algorithms above is LSH related
 * algorithms below is partiton based on heuristic method
 */
/* method description:
 * 将ms_dataset里面的数据按照某种方法划分为多个块
 * 以multimap的形式给出
 * 对LSH同样适用
 */

/* the hash function */
vector<int> hash_func(int idx, int num_group, vector<spectra> &dataset)			// 这里的num_group是广义的意思，此处为选第几高的峰
{
	double tolerance_pre = 3;
	double tolerance_peak = 0.5;

	vector<int> key(2, -1);
	key[0] = round(dataset[idx].precursor_mz / tolerance_pre);
	key[1] = round(dataset[idx].ions[num_group].mass_value / tolerance_peak);

	return key;

}

typedef multimap<vector<int>, int> heu_hashmap;
heu_hashmap partition_heuristic(vector<spectra> dataset)
// unordered_multimap<vector<int>, int> partition_heuristic(vector<spectra> dataset)
// hash_multimap<vector<int>, int > partition_heuristic(vector<spectra> dataset)
{
	int group_num_heu = 4;			// top k  peaks to be selected

	multimap<vector<int>, int > par_heu;
	int count = dataset.size();
	for (int i = 0; i < count; ++i)
	{
		for (int j = 0; j < group_num_heu; ++j)
		{
			par_heu.insert(make_pair(hash_func(i, j, dataset), i));
		}
	}
	return par_heu;
} 



/* merge hashmaps collected from different nodes */
heu_hashmap merge_par_heu(vector<heu_hashmap> patch_hashmap)
{
	heu_hashmap merged_map;
	int count = patch_hashmap.size();
	for (int i = 0; i < count; ++i)
	{
		/* code */
		heu_hashmap::iterator it = patch_hashmap[i].begin();
		while(it != patch_hashmap[i].end())
		{
			merged_map.insert(make_pair((*it).first, (*it).second));
			it++;
			/*
			* hint
			* mm.insert(pair<float, char*>(3.0f, "apple"));
			*/

		}
	}
	return merged_map;
}

/* 将heu_hashmap对象转变为可用于分配任务的dict_idx格式 */
vector<dict_idx> trans_table_dict(heu_hashmap merged_map)
{
	vector<dict_idx> idx_table;
	heu_hashmap::iterator it = merged_map.begin();

	while(it != merged_map.end())
	{
		vector<int> key_curr = (*it).first;
		heu_hashmap::iterator beg_curr = merged_map.lower_bound(key_curr);
		heu_hashmap::iterator end_curr = merged_map.upper_bound(key_curr);

		vector<int> idx_tmp;
		while(beg_curr != end_curr)
		{
			idx_tmp.push_back((*it).second);
			beg_curr++;
			it++;
		}
		// 这里，只需要确定end_curr的值等于下一个key的beg_curr就正确
		// 实验表明，上述假设正确，upper_bound等于下一个元素的lower_bound
		dict_idx dict_tmp(-1, idx_tmp);
		idx_table.push_back(dict_tmp);
	}
	return idx_table;
}

/* 分配任务，在原结构上写入num_proc信息
* 分配中保证负载均衡
* 目前的方法就是随机分配，能在一定程度上保证负载均衡
*/
void alloc_assign(vector<dict_idx> &idx_table, int num_proc_comp)
{
	/* to fullfill the dict_idx class, try balancing the load of every proc*/
	/* note that the group_num is not important anymore */

	vector<vector<int> > global_datasize(num_proc_comp + 1, vector<int>());

	int count = idx_table.size();
	/* the simplest method, balance load by randomly distributed */
	for (int i = 0; i < count; ++i)
	{
		idx_table[i].num_proc = i % num_proc_comp + 1;
		global_datasize[idx_table[i].num_proc].push_back(idx_table[i].idx.size());
	}

	/* more effective method by optimization and planning */

	/*
	cout << "load of every nodes: " << endl;
	int load_tmp = 0;
	for (int i = 1; i < num_proc_comp + 1; i++)
	*/
	return;
}

/* allocate assigments, results as dict_full */
vector<dict_full> alloc_assign_full(vector<dict_idx> idx_table, int num_proc_comp)
{
	vector<dict_full> dict_full_vec;
	for (int i = 0; i < num_proc_comp + 1; i++)
	{
		dict_full dict_tmp(i);
		dict_full_vec.push_back(dict_tmp);
	}


	// vector<vector<int> > global_datasize(num_proc_comp + 1, vector<int>());

	int count = idx_table.size();
	/* the simplest method, balance load by randomly distributed */
	for (int i = 0; i < count; ++i)
	{
		int proc_assign = i % num_proc_comp + 1;
		ms_dataset dset_tmp(idx_table[i].idx);
		dict_full_vec[proc_assign].multi_datasets.push_back(dset_tmp);
	}
	return dict_full_vec;
}


