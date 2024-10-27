#include "notify.h"

void	ft_free(t_dir **track_dir)
{
	int	i;
	int	j;

	i = 0;
	if (!track_dir)
		return ;
	while (track_dir[i])
	{
		j = 0;
		while (track_dir[i]->tree[j])
		{
			free(track_dir[i]->tree[j]);
			track_dir[i]->tree[j] = NULL;
			j++;
		}
		free(track_dir[i]->tree);
		free(track_dir[i]);
		track_dir[i] = NULL;
		i++;
	}
	free(track_dir);
	track_dir = NULL;
}
