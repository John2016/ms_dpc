#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>

#include <vector>
#include <algorithm>

#include <string.h>

// the below two is linux-related
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

// #include "ms_info_head.h"

using namespace std;

void printdir(char * dir, vector<string> &file_names, char * ancestor_dir, char * file_type)
{
	if (strstr(dir, file_type) != NULL) {
		file_names.push_back(string(dir));
		return;
	}

	DIR *Dp;
	struct dirent *enty;
	struct stat statbuf;

	if (NULL == (Dp = opendir(dir)))
	{
		fprintf(stderr, "can not open dir:%s\n", dir);
		return;
	}

	int status = chdir(dir);

	char dir_tmp[120];
	char names_tmp[120];

	while (NULL != (enty = readdir(Dp)))
	{
		lstat(enty->d_name, &statbuf);

		if (S_ISDIR(statbuf.st_mode))
		{
			// cout << 
			if (0 == strcmp(".", enty->d_name) || 0 == strcmp("..", enty->d_name))
			{
				continue;
			}

			strcpy(names_tmp, ancestor_dir);
			/*
			if (ancestor_dir[0] != '\0')
			{
			strcat(names_tmp, "/");
			}
			*/
			strcat(names_tmp, dir);
			strcat(names_tmp, "/");

			printdir(enty->d_name, file_names, names_tmp, file_type);
			// printdir(enty -> d_name, file_names);
		}
		else
		{
			// only mgf file be kept
			if (strstr(enty->d_name, file_type) != NULL)
			{
				// strcpy(names_tmp, enty->d_name);
				strcpy(dir_tmp, ancestor_dir);
				strcat(dir_tmp, dir);
				strcat(dir_tmp, "/");

				strcpy(names_tmp, enty->d_name);
				/*
				if (ancestor_dir[0] != '\0')
				{
				strcat(names_tmp, "/");
				}
				*/
				// strcat(dir_tmp, dir);
				// strcat(dir_tmp, "/");
				strcat(dir_tmp, names_tmp);

				file_names.push_back(string(dir_tmp));
				// file_names.push_back(string(enty->d_name));
				// printf("%*s%s\n",depth," ",enty->d_name);
			}

		}
	}

	status = chdir("..");

	closedir(Dp);
}