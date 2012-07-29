#include <stdio.h>
#include <unistd.h>

#include "pipeneg.h"

int
main(int argc, char *argv[])
{
  char buf[1000];
  ssize_t n_read, n_written, n_written_tot;

  pipe_setup_for_binary (STDIN_FILENO);
  
  while ((n_read = read (STDIN_FILENO, buf, sizeof(buf))) > 0) {
    n_written_tot = 0;
    while (n_written_tot < n_read) {
      n_written = write (STDOUT_FILENO, buf + n_written_tot, n_read - n_written_tot);
      if (n_written <= 0)
	return 1;
      n_written_tot += n_written;
    }
  }
  return 0;
}

