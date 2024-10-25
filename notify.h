#ifndef H_NOTIFY
#define H_NOTIFY

#ifndef DEBUG
#define DEBUG 0
#endif

#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>


struct	inotify_args
{
	int		argc;
	char	**argv;
	int		bk_interval;
};

typedef struct	s_file_info
{
	char		*file;
	char		backup_path[1024];
	char		file_name[1024];
	long long	baseline_mtime;
}	t_file_info;

typedef struct s_dir_tree
{
	char				*base_dir;
	char				file_name[1024];
	long long			baseline_mtime;
	struct t_dir_tree	*next;
}	t_dir_tree;

//Free  the memory of the array t_dir[nbr of directories]
//Free the memory of the array t_file_info[nbr of files]

typedef struct s_dir
{
	char				base_dir[1024];
	long long			baseline_mtime;
	t_file_info			**tree;
}	t_dir;

bool	check_modification_time(char *dir, long long baseline_mtime);
t_dir	**count_dirs(int argc, t_file_info *info);
void	track_dir_mtime(char *dir, t_dir **dir_tree);
bool	check_dir_mtime(char *main_dir, t_dir **track_dir);
void	set_backup_dir(char *main_dir, t_dir **track_dir);
void	ft_free(t_dir **track_dir);
char	*get_home_dir(char *home_dir, size_t len);
void	create_dir(char *dir);
bool	endswith(char *s, char *suffix);
void	create_backup_directory(char *src_dir, char *backup, bool first_time);

#endif
