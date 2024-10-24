#include "notify.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void	set_backup_path(char *file_name, char *backup_path)
{
		int	i;
		int	j;

		i = 0;
		j = 0;
		while (file_name[i] != '\0')
		{
			if (file_name[i] == '/')
			{
				snprintf(backup_path + j, 5, ".bak");
				j += 4;
			}
			backup_path[j] = file_name[i];
			i++;
			j++;
		}
}

void	create_backup_dir(char *main_dir, t_dir **track_dir)
{
	int	i;
	int	j;

	i = 0;
	while (track_dir[i] != NULL)
	{
		if (strcmp(track_dir[i]->base_dir, main_dir) == 0)
		{
			j = 0;
			while (track_dir[i]->tree[j] != NULL)
			{
				memset(track_dir[i]->tree[j]->backup_path, 0, 1024);
				set_backup_path(track_dir[i]->tree[j]->file_name, track_dir[i]->tree[j]->backup_path);
				j++;
			}
		}
		i++;
	}
}
