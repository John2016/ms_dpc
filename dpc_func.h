#pragma once

#include "spectra.h"

struct decision_pair
{
	double gamma;
	long order;

	decision_pair(double gamma, long order) : gamma(gamma), order(order) {}
};

struct rho_cluster
{
	double rho;
	long order;

	rho_cluster(double rho, long order) : rho(rho), order(order) {}
};

double get_point_dis(spectra pt1, spectra pt2, int method);

double get_pdis_dotp(spectra &pt1, spectra &pt2);

vector<int> decide_double_thres(vector<double> &delta, vector<double> &rho, int &cluster_num, int n_sample);

vector<int> decide_by_gap(vector<double> &delta, vector<double> &rho, int &cluster_num, int n_sample);

vector<int> decide_multi_thres(vector<double> &delta, vector<double> &rho, int &cluster_num, int n_sample);

vector<int> decide_by_graph(vector<double> &delta, vector<double> &rho, int &cluster_num, int n_sample);

void get_halo(vector<int> &decision, vector< vector<double> > &data_distance, vector<bool> &cluster_halo, vector<double> &rho, double dc, int cluster_num);

bool comp_rho(const rho_cluster & pt1, const rho_cluster &pt2);