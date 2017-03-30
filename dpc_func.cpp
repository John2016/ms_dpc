#include<math.h>
#include<assert.h>
#include <set>

#include <iostream>

#include "ms_dataset.h"
#include "dpc_func.h"

#define RHO_RATE 0.1
#define DELTA_RATE 0.1

using namespace std;

double get_point_dis(spectra pt1, spectra pt2, int method)
{
	switch(method){
		/* E2 */
		case 1:
            // return e2_dis();        // parameter of this func should reset
			return 0.0;
		/* cosine */
		case 2:
            return 0.0;
		/* jaccard */
		case 3:
            // return jaccard_dis();   // same to the latest one
			return 0.0;
		/* pearson */
		case 4:
            return 0.0;
        /* dot product */
        case 5:
            return get_pdis_dotp(pt1, pt2);
		default:
			break;
	}
	return 0.0;
}

double get_pdis_dotp(spectra &pt1, spectra &pt2)
{
    // both points should be filtered and normalized
    double tolerance = 0.5;
    vector<double> union_vec;
    vector<double> union_pt1;
    vector<double> union_pt2;

    int count_1 = pt1.ions.size();
    for (int i = 0; i < count_1; ++i)
    {
        union_vec.push_back(pt1.ions[i].mass_value);
        union_pt1.push_back(pt1.ions[i].intensity);
        union_pt2.push_back(0.0);
    }

    /* fullfill the union vector with normalized intensity */
    int count_2 = pt2.ions.size();
    for (int i = 0; i < count_2; ++i)
    {
		int j;
        for (j = 0; j < count_1; ++j)
        {
            if (abs(pt2.ions[i].mass_value - pt1.ions[j].mass_value) <= 0.5)
            {
                union_pt2[j] = pt2.ions[i].intensity;
                continue;
            }
            else
                continue;
        }
        if (j == count_1)
        {
            /* new item */
            union_vec.push_back(pt2.ions[i].mass_value);
            union_pt2.push_back(pt2.ions[i].intensity);
            union_pt1.push_back(0.0);
        }
    }

    assert(union_vec.size() == union_pt1.size());
    assert(union_vec.size() == union_pt2.size());

    /* calculation of dot product */
    double dot_product = 0.0;
	double vec_size[2] = { 0.0 };
    for (int i = 0; i < union_vec.size(); ++i)
    {
        dot_product += union_pt1[i] * union_pt2[i];
        vec_size[0] += pow(union_pt1[i], 2);
        vec_size[1] += pow(union_pt2[i], 2);
    }

    vec_size[0] = sqrt(vec_size[0]);
    vec_size[1] = sqrt(vec_size[1]);
    
    dot_product = dot_product / (vec_size[0] * vec_size[1]);

	/* it should be note that distance rather than similarity should be returned */
    return 1 - dot_product;
}


/* Euclidean distance calculation */
double e2_dis( vector<double> &pt1, vector<double> &pt2){
    double tmp=0;
	int feature_dim = 10;
	for (int i = 0; i < feature_dim; ++i)
    {
        tmp += powf(pt1[i]-pt2[i],2);
    }

    return powf(tmp, 0.5);
}

double jaccard_dis(vector<int> ms1, vector<int> ms2)
{
    vector<int> v_u;
    vector<int> v_i;

    sort(ms1.begin(), ms1.end());
    sort(ms2.begin(), ms2.end());

    // set_union(ms1.begin(),ms1.end(),ms2.begin(),ms2.end(),back_inserter(v_u));
    // set_intersection(ms1.begin(),ms1.end(),ms2.begin(),ms2.end(),back_inserter(v_i));

    double similarity = (double)v_i.size() / v_u.size();

    return 1 - similarity;
}

vector<int> decide_double_thres(vector<double> &delta, vector<double> &rho, int &cluster_num, int n_sample)
{
    int counter = 0;
    vector<int> decision(n_sample, -1);
    
    double max_rho = *(max_element(rho.begin(), rho.end()));
    double min_rho = *(min_element(rho.begin(), rho.end()));
    double max_delta = *(max_element(delta.begin(), delta.end()));
    double min_delta = *(min_element(delta.begin(), delta.end()));

    /* but why, this may be high false positive */ 
    double rho_thres;
    rho_thres = RHO_RATE * (max_rho - min_rho) + min_rho;
    double delta_thres;
    delta_thres = DELTA_RATE * (max_delta - min_delta) + min_delta;

    for (int i = 0; i < n_sample; ++i)
    {
        if (rho[i] > rho_thres && delta[i] > delta_thres)
        {
            decision[i] = counter;
            counter++;
        }
    }

    cluster_num = counter;
    cout<<"number of clusters: "<< cluster_num << endl;
    
    return decision;
}

bool comp_dv(const void *a,const void *b)
{
    struct decision_pair *aa = (decision_pair *)a;
    struct decision_pair *bb = (decision_pair *)b;

    return aa->gamma - bb->gamma;
}

bool comp_gamma(const decision_pair& pt1, const decision_pair&pt2)
{
    return pt1.gamma > pt2.gamma;
}

void normalize_paras(vector<double> &delta, vector<double> &rho, int n_sample)
{
	double max_rho = *(max_element(rho.begin(), rho.end()));
	double min_rho = *(min_element(rho.begin(), rho.end()));
	double max_delta = *(max_element(delta.begin(), delta.end()));
	double min_delta = *(min_element(delta.begin(), delta.end()));

	double rho_range = max_rho - min_rho;
	double delta_range = max_delta - min_delta;

	for(int i = 0; i < n_sample; i++)
	{
		delta[i] = (delta[i] - min_delta) / delta_range + min_delta;
		rho[i] = (rho[i] - min_rho) / rho_range + min_rho;
	}
	return;
}


vector<int> decide_by_gap(vector<double> &delta, vector<double> &rho, int &cluster_num, int n_sample){
     
    // int counter = 0;
    vector<int> decision(n_sample, -1);
    vector<decision_pair> decision_value;

    for (int i = 0; i < n_sample; ++i)
    {
        decision_pair tmp(delta[i] * rho[i], i);
        decision_value.push_back(tmp);
    }

    sort(decision_value.begin(), decision_value.end(), comp_gamma);
    //qsort(&decision_value, decision_value.size(), sizeof(decision_value), comp_dv);
    //sort(decision_value.begin(), decision_value.end(),comp);
    for (int i = 1; i < n_sample; ++i)
    {
        double meandif=((decision_value[i].gamma - decision_value[i+1].gamma)+
                       (decision_value[i+1].gamma - decision_value[i+2].gamma)+
                       (decision_value[i+2].gamma - decision_value[i+3].gamma))/3;

        if (-(decision_value[i].gamma - decision_value[i-1].gamma) / decision_value[i].gamma > 0.5 && meandif / decision_value[i].gamma < 0.1)
        {
            cluster_num=i;
            break;
        }
    }

    for (int i = 0; i < cluster_num - 1; ++i)
    {
        decision[decision_value[i].order] = i;
    }
    cout<<"number of clusters: "<< cluster_num << endl;

    return decision;
}

/* mean value and std value calculation, threshold = mean_gamma + std_gamma */
vector<int> decide_multi_thres(vector<double> &delta, vector<double> &rho, int &cluster_num, int n_sample)
{
    vector<int> decision(n_sample, -1);
    vector<decision_pair> decision_value;

	normalize_paras(delta, rho, n_sample);

    for (int i = 0; i < n_sample; ++i)
    {
        decision_pair tmp(delta[i] * rho[i], i);
        decision_value.push_back(tmp);
    }

    /* mean+std is the break point of guassian function */
    double mean_gamma = 0;
    for (int i = 0; i < n_sample; ++i)
    {
        mean_gamma += decision_value[i].gamma;
    }
    mean_gamma /= n_sample;

    double std_gamma = 0;
    for (int i = 0; i < n_sample; ++i)
    {
        std_gamma += powf((decision_value[i].gamma - mean_gamma), 2);
    }
    std_gamma /= n_sample;
    std_gamma = powf(std_gamma, 0.5);

    double thres_gamma = mean_gamma + std_gamma;
    int counter = 0;
    for (int i = 0; i < n_sample; ++i)
    {
        if (decision_value[i].gamma > thres_gamma)
        {
            decision[i] = counter;
            counter++;
        }
    }

    cluster_num = counter;
    cout<<"number of clusters: "<< cluster_num << endl;

    return decision;
}

vector<int> decide_by_graph(vector<double> &delta, vector<double> &rho, int &cluster_num, int n_sample){

    /* draw by opencv */
	normalize_paras(delta, rho, n_sample);

	vector<int> decision;
	return decision;
}

bool comp_rho(const rho_cluster & pt1, const rho_cluster &pt2)
{
    return pt1.rho > pt2.rho;
}


/* assignment recursively */
int assign_cluster_recursive(int index){
    /*
        This can not be done once for all
        for data belonging to one cluster can not date from data belonging to other cluster
    */
	return 0;
}


void get_halo(vector<int> &decision, vector< vector<double> > &data_distance, vector<bool> &cluster_halo, vector<double> &rho, double dc,int cluster_num){
    /*
     *  in my opinion
     *  halo can be got by the therom "high delta but low density"
     *  which is not so complicated
    */

    vector<double> density_bound(cluster_num, 0.0);
    int n_sample = decision.size();
    for (int i = 0; i < n_sample - 1; ++i)
    {
        double avrg_rho;
        for (int j = i+1; j < n_sample; ++j)
        {
            if (decision[i] != decision[j] && data_distance[i][j]<dc)
            {
                avrg_rho = (rho[i] + rho[j]) / 2;
                if (avrg_rho > density_bound[decision[i]])
                {  
                    density_bound[decision[i]] = avrg_rho;
                }
                if (avrg_rho>density_bound[decision[j]])
                {
                    density_bound[decision[j]] = avrg_rho;
                }
            }
        }
    }
    for (int i = 0; i < n_sample; ++i)
    {
        if (rho[i] <= density_bound[decision[i]])
        {
            cluster_halo.push_back(false);
        }
        else cluster_halo.push_back(true);
    }
    return;
}
