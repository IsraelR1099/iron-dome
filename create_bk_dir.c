#include "notify.h"

/* create_tmp_pathectory receives the directory to check (src_dir).
 * The backup argument is the one pointing to the user's HOME directory with backup included.
 * */

void	create_tmp_pathectory(char *src_dir, char *backup, bool first_time)
{
	char			tmp_path[4096] = {0};
	char			tmp_src[256];
	char			msg_error[256];
	//size_t			remaining_space;
	DIR				*dp;
	struct dirent	*entry;

	memset(tmp_src, 0, sizeof(tmp_src));
	strncpy(tmp_src, src_dir, sizeof(tmp_src));
	tmp_src[sizeof(tmp_src) - 1] = '\0';
	if (endswith(src_dir, "/"))
		tmp_src[strlen(tmp_src) - 1] = '\0';
	snprintf(tmp_path, sizeof(tmp_path), "%s%s.bak", backup, tmp_src);
	create_dir(tmp_path);
	if ((dp = opendir(src_dir)) == NULL)
	{
		snprintf(msg_error, sizeof(msg_error), "Error opendir: %s.", src_dir);
		perror(msg_error);
		exit (EXIT_FAILURE);
	}
	while ((entry = readdir(dp)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0
			|| strcmp(entry->d_name, "..") == 0)
			continue ;
		if (entry->d_type == DT_DIR)
		{
			printf("src dir %s y backup %s\n", tmp_src,backup);
		}
		else
		{
			printf("archivo es de %s es: %s\n", entry->d_name, src_dir);
			snprintf(tmp_path, sizeof(tmp_path), "%s/" src_dir, entry->d_name);
			create_backup_file(tmp_path, )
		}
	}
	if (closedir(dp) < 0)
	{
		snprintf(msg_error, sizeof(msg_error), "Error closedir");
		perror(msg_error);
		exit (EXIT_FAILURE);
	}
	(void)first_time;
}
