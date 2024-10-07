#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void	mask_signals(void) {
 	sigset_t	mask;

 	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("sigprocmask");
		exit(1);
	}
}
