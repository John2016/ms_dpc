srun --job-name=spectra_cluster -w ib-node[4-27,29-40,42-56] -l --kill --input=ms_input.txt --output=ms_out.txt --error=ms_error.txt ./main &
