#pragma once
#include <string>
#include <vector>
#include <map>
// #include <unordered_map>
#include "parallel_struct.h"

using namespace std;

/* operation of files related */
void printdir(char * dir, vector<string> &file_names, char * ancestor_dir, char * file_type);

/* data translation of assignments related */
//string encode_assign(dict_full assign_node);
string encode_assign(const dict_full * assign_node);
dict_full decode_assign(string assign_str);

/* LSH related */
vector<int> generate_rand_paras(int length, int para_1_max_mass);

/* partition related */
multimap<vector<int>, int > partition_heuristic(vector<spectra> dataset);

typedef multimap<vector<int>, int> heu_hashmap;
heu_hashmap merge_par_heu(vector<heu_hashmap> patch_hashmap);

vector<dict_idx> trans_table_dict(heu_hashmap merged_map);