#pragma once
#include<string>
#include<vector>
#include<algorithm>

using namespace std;

class peak
{
public:
	double mass_value;
	double intensity;

public:
	peak()
	{

	}

	peak(double out_mv, double out_inten)
	{
		mass_value = out_mv;
		intensity = out_inten;
	}

	~peak()
	{

	}

};

class spectra
{
public:
	string file_name;
	string spectra_name;
	double precursor_mz;
	int charge;

	vector<peak> ions;

public:
	spectra()
	{

	}

	spectra(string fname, string sname, double mz, int charge_o, vector<peak> ions_o)
	{
		file_name = fname;
		spectra_name = sname;
		precursor_mz = mz;
		charge = charge_o;
		ions = ions_o;
	}

	~spectra()
	{

	}

	void append_peak(peak new_ion);

	/* select the top k peaks */
	void select_peaks();

	/* normalize peaks */
	void normalize_spectra();

};






