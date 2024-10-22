#include "notify.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static bool	check_file_mtime(char *file, char *main_dir, t_dir **track_dir)
{
	int			i;
	int			j;
	struct stat	st;
	char		error_msg[1024];

	i = 0;
	if (DEBUG)
	{
		printf("check_file_mtime: file: %s, main_dir: %s\n", file, main_dir);
	}
	while (track_dir[i] != NULL)
	{
		if (strcmp(track_dir[i]->base_dir, main_dir) == 0)
		{
			j = 0;
			while (track_dir[i]->tree[j] != NULL)
			{
				printf("track_dir[i]->tree[j]->file_name: %s\n", track_dir[i]->tree[j]->file_name);
				if (strcmp(track_dir[i]->tree[j]->file_name, file) == 0)
				{
					if (DEBUG)
					{
						printf("encontrado: %s\n", file);
					}
					if (stat(file, &st) < 0)
					{
						snprintf(error_msg, sizeof(error_msg), "Error stat: %s", file);
						perror(error_msg);
						exit(EXIT_FAILURE);
					}
					if (DEBUG)
					{
						printf("st.st_mtime: %ld, baseline_mtime: %lld\n", st.st_mtime, track_dir[i]->tree[j]->baseline_mtime);
					}
					if (st.st_mtime > track_dir[i]->tree[j]->baseline_mtime)
					{
						printf("file %s has been modified\n", file);
						track_dir[i]->tree[j]->baseline_mtime = st.st_mtime;
						return (true);
					}
				}
				j++;
			}
		}
		i++;
	}
	return (false);
}

bool	check_dir_mtime(char *main_dir, t_dir **track_dir)
{
	DIR				*dp;
	struct dirent	*entry;
	char			error_msg[1024];
	char			full_path[1024];

	if ((dp = opendir(main_dir)) == NULL)
	{
		snprintf(error_msg, sizeof(error_msg), "Error opendir: %s", main_dir);
		perror(error_msg);
		exit(EXIT_FAILURE);
	}
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
			|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
		{
			printf("open recursive dir: %s\n", entry->d_name);
		}
		else
		{
			snprintf(full_path, sizeof(full_path), "%s/%s", main_dir, entry->d_name);
			if (check_file_mtime(full_path, main_dir, track_dir) == true)
			{
				printf("file %s has been modified\n", entry->d_name);
				return (true);
			}
		}
	}
	closedir(dp);
	return (false);
}
