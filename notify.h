#ifndef H_NOTIFY
#define H_NOTIFY

#ifndef DEBUG
#define DEBUG 0
#endif

#include <stdbool.h>

struct	inotify_args
{
	int		argc;
	char	**argv;
};

typedef struct	s_file_info
{
	char		*file;
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

#endif
