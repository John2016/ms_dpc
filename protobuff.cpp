/* generate protobuf */
#include <string>
#include "parallel_struct.h"
#include "ms_dataset.h"
#include "spectra.h" 
#include "assignment.pb.h"

using namespace std;

string encode_assign(const dict_full * assign_node)
{
	// cout << "in encode function" << endl;
	// cout << "in encode function" << assign_node->multi_datasets[0].data.size() << " & " << assign_node->multi_datasets[0].data[0].ions.size() << endl;
	// cout << assign_node.multi_datasets.size() << " & " << assign_node.multi_datasets[0].data.size() << endl;
	// cout << assign_node.multi_datasets[0].data[0]
	assignment assign_proto;
	assign_proto.set_num_proc(assign_node->num_proc);
	for (int i = 0; i < assign_node->multi_datasets.size(); ++i)
	{
		// cout << "in the first recurrent" << endl;
		/* for every datasets in this assignments */
		ms_data * datasets = assign_proto.add_multi_dataset();
		for (int j = 0; j < assign_node->multi_datasets[i].data_idxes.size(); ++j)
		{
			// cout << "test point 1" << endl;
			/* for every data in this datasets, i.e. spectra */
			datasets->add_idxes(assign_node->multi_datasets[i].data_idxes[j]);
			// cout << "test point 1.1" << endl;
			spectra_p * spec = datasets->add_data();
			// cout << "test point 1.2" << endl;
			// cout << "test string: "<< assign_node->multi_datasets[i].data[j].file_name << endl;
			spec->set_file_name(assign_node->multi_datasets[i].data[j].file_name);
			// cout << "test point 1.3" << endl;
			spec->set_spectra_name(assign_node->multi_datasets[i].data[j].spectra_name);
			// cout << "test point 2" << endl;
			for (int k = 0; k < assign_node->multi_datasets[i].data[j].ions.size(); ++k)
			{
				// cout << "test point 3" << endl;
				/* for every peak in this spectra */
				peak_p * ions_peak = spec->add_ions();
				ions_peak->set_mz_value(assign_node->multi_datasets[i].data[j].ions[k].mass_value);
				ions_peak->set_intensity(assign_node->multi_datasets[i].data[j].ions[k].intensity);
			}
		}
	}

	// cout << "after the whole recurrent" << endl;
	string assign_str;
	assign_proto.SerializeToString(& assign_str);

	// cout << "before encode return, to verify: " << assign_proto.multi_dataset_size() << " & " << assign_proto.multi_dataset(0).data_size()<< endl;

	return assign_str;
}


dict_full decode_assign(string assign_str)
{
	// cout << "in decode function" << endl;
	// cout << "\tstring size: " << assign_str.size() << endl;
	assignment assign_proto;
	assign_proto.ParseFromString(assign_str);
	// cout << "\tafter parse: " <<assign_proto.multi_dataset(0).data_size() << " & " << assign_proto.multi_dataset(0).data(0).ions_size() << endl;
	dict_full assign_node(assign_proto.num_proc());
	int count_dataset = assign_proto.multi_dataset_size();		// ?
	for (int i = 0; i < count_dataset; ++i)
	{
		/* for every datasets belonging to this proc */
		ms_dataset datasets_tmp;
		int count_data = assign_proto.multi_dataset(i).data_size();
		// cout << "\t count_data: " << count_data << endl;
		for (int j = 0; j < count_data; ++j)
		{
			/* for every idxes of each data(spectra) */
			datasets_tmp.data_idxes.push_back(assign_proto.multi_dataset(i).idxes(j) );

			/* for every data(spectra) in this dataset */
			spectra spectra_tmp;
			spectra_tmp.file_name = assign_proto.multi_dataset(i).data(j).file_name();
			spectra_tmp.spectra_name = assign_proto.multi_dataset(i).data(j).spectra_name();
			int count_peak = assign_proto.multi_dataset(i).data(j).ions_size();
			for (int k = 0; k < count_peak; ++k)
			{
				/* for every peaks in this spectra data */
				peak peak_tmp;
				peak_tmp.mass_value = assign_proto.multi_dataset(i).data(j).ions(k).mz_value();
				peak_tmp.intensity = assign_proto.multi_dataset(i).data(j).ions(k).intensity();
				spectra_tmp.ions.push_back(peak_tmp);
			}
			datasets_tmp.data.push_back(spectra_tmp);
			// cout << "\tdata size: " << datasets_tmp.data.size() << endl;
		}
		assign_node.multi_datasets.push_back(datasets_tmp);
		// cout << "\tafter push back: " << assign_node.multi_datasets[i].data.size() << endl;
	}
	// cout << "\tbefore return: size " << assign_node.multi_datasets.size() << endl;
	// cout << "\ttest" << assign_node.multi_datasets[0].data.size() << " & " << assign_node.multi_datasets[0].data[0].ions.size() << endl;
	return assign_node;
}
