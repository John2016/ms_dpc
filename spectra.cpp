#include <math.h>
#include "spectra.h"

bool comp_inten(const peak & p1, const peak & p2)
{
	return p1.intensity > p2.intensity;
}

void spectra::append_peak(peak new_ion)
{
	ions.push_back(new_ion);
	return;
}

void spectra::select_peaks()
{
	int k = this->precursor_mz / 50;
	sort(this->ions.begin(), this->ions.end(), comp_inten);
	// sort(data_rho.begin(), data_rho.end(), comp_rho);
	int num_erase = this->ions.size() - k;
	this->ions.erase(this->ions.end() - num_erase, this->ions.end());
	return;
}

void spectra::normalize_spectra()
{
	/* make the sum of intensity to 1000 */
	double sum_inten = 0.0;
	for (int i = 0; i < this->ions.size(); ++i)
	{
		sum_inten += (this->ions[i].intensity);
	}
	double scale_magnin = 1000.0 / sum_inten;
	for (int i = 0; i < this->ions.size(); ++i)
	{
		this->ions[i].intensity *= scale_magnin;
		this->ions[i].intensity = 1 + log(this->ions[i].intensity);
	}
	return;
}