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

	peak(const peak& pk)
	{
		this->mass_value = pk.mass_value;
		this->intensity = pk.intensity;
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

	spectra(const spectra& spe)
	{
		this->file_name = spe.file_name;
		this->spectra_name = spe.spectra_name;

		this->precursor_mz = spe.precursor_mz;
		this->charge = spe.charge;

	 	this->ions = spe.ions;
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






