#include "notify.h"

void	create_backup_directory(char *src_dir, char *backup)
{
	struct dirent	*entry;
	DIR				*dp;
	char			tmp_path[2048] = {0};
	char			tmp_file_path[4096] = {0};
	char			tmp_bk_file[4096] = {0};
	char			tmp_src_dir[4096] = {0};
	char			msg_error[256];
	const char		*dir_name;
	size_t			len;

	strncpy(tmp_src_dir, src_dir, sizeof(tmp_src_dir) - 1);
	tmp_src_dir[sizeof(tmp_src_dir) - 1] = '\0';
	len = strlen(tmp_src_dir);
	if (len > 0 && tmp_src_dir[len - 1] == '/')
		tmp_src_dir[len - 1] = '\0';
	dir_name = strrchr(tmp_src_dir, '/');
	if (dir_name)
			dir_name++;
	else
		dir_name = src_dir;
	if (endswith(backup, "/"))
		snprintf(tmp_path, sizeof(tmp_path), "%s%s.bak", backup, dir_name);
	else
		snprintf(tmp_path, sizeof(tmp_path), "%s/%s.bak", backup, dir_name);
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
		if (endswith(src_dir, "/"))
			snprintf(tmp_file_path, sizeof(tmp_file_path), "%s%s", src_dir, entry->d_name);
		else
			snprintf(tmp_file_path, sizeof(tmp_file_path), "%s/%s", src_dir, entry->d_name);
		snprintf(tmp_bk_file, sizeof(tmp_bk_file), "%s/%s.bak", tmp_path, entry->d_name);
		if (entry->d_type == DT_DIR)
			create_backup_directory(tmp_file_path, tmp_path);
		else
			create_relative_file(tmp_file_path, tmp_bk_file);
	}
	if (closedir(dp) < 0)
	{
		snprintf(msg_error, sizeof(msg_error), "Error closedir");
		perror(msg_error);
		exit (EXIT_FAILURE);
	}
}
