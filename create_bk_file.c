#include "notify.h"

static void	set_backup_path(const char *file, const char *backup_dir, char *backup_path)
{
	char	tmp_file[256] = {0};
	char	tmp_bk_path[1024] = {0};
	size_t	i;
	size_t	k;

	if (file[0] == '/')
			strncpy(tmp_file, file + 1, sizeof(tmp_file) - 1);
	else
		strncpy(tmp_file, file, sizeof(tmp_file) - 1);
	tmp_file[sizeof(tmp_file) - 1] = '\0';
	snprintf(backup_path, 1024, "%s", backup_dir);
	snprintf(tmp_bk_path, 1024, "%s%s.bak", backup_dir, tmp_file);
	i = strlen(backup_dir);
	k = i;
	while (tmp_bk_path[k] != '\0')
	{
		backup_path[i] = tmp_bk_path[k];
		if (backup_path[i] == '/')
		{
			strncpy(backup_path + i, ".bak/", 6);
			i += 4;
		}
		i++;
		k++;
	}
	backup_path[i] = '\0';
}

void	create_backup_file(char *file, char *dst_dir)
{
	char	backup_path[1024] = {0};
	char	err_msg[2048] = {0};
	char	*last_slash;
	char	buf[16384] = {0};
	size_t	n;
	FILE	*src;
	FILE	*dst;

	printf("\033[33mCreating backup file for %s\n\033[0m", file);
	if (file[0] != '/')
	{
			create_relative_file(file, dst_dir);
			return ;
	}
	set_backup_path(file, dst_dir, backup_path);
	last_slash = strrchr(backup_path, '/');
	if (last_slash != NULL)
	{
		*last_slash = '\0';
		create_dir(backup_path);
		*last_slash = '/';
	}
	src = fopen(file, "r");
	if (!src)
	{
		snprintf(err_msg, sizeof(err_msg), "Failed to open file %s", file);
		perror(err_msg);
		exit(EXIT_FAILURE);
	}
	dst = fopen(backup_path, "w");
	if (!dst)
	{
		snprintf(err_msg, sizeof(err_msg), "Failed to open file %s", backup_path);
		perror(err_msg);
		fclose(src);
		exit(EXIT_FAILURE);
	}
	while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
	{
		if (fwrite(buf, 1, n, dst) != n)
		{
			snprintf(err_msg, sizeof(err_msg), "Failed to write to file %s", backup_path);
			perror(err_msg);
			fclose(src);
			fclose(dst);
			exit(EXIT_FAILURE);
		}
	}
	fclose(src);
	fclose(dst);
	printf("Backup file created: %s\n", backup_path);
}
