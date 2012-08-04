#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "pipeneg.h"

/* Call this on the fd before reading from it */
void
pipe_setup_for_binary (int fd)
{
  struct flock fl;

  fl.l_type   = F_RDLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start  = BINARY_FORMAT_MAGIC_COOKIE;
  fl.l_len    = 0;
  fl.l_pid    = getpid();
    
  fcntl (fd, F_SETLK, &fl);
}

static int
check_format (int fd)
{
  struct flock fl;
  int res;
  struct stat statbuf;

  /* Check for pipe or always FORMAT_TEXT */
  
  if (fstat (fd, &statbuf) != 0)
    return FORMAT_TEXT;

  if (!S_ISFIFO (statbuf.st_mode))
    return FORMAT_TEXT;
  
  fl.l_type   = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start  = 0;
  fl.l_len    = 0;
  fl.l_pid    = getpid();
  res = fcntl (fd, F_GETLK, &fl);

  if (res != 0) {
    return FORMAT_TEXT;
  }

  if (fl.l_type == F_UNLCK) {
    return -1; /* Unknown */
  } else if (fl.l_start == BINARY_FORMAT_MAGIC_COOKIE)
    return FORMAT_BINARY;
  else
    return FORMAT_TEXT;
}

StreamFormat
pipe_negotiate_format (int fd)
{
  int format;
  char buf = '<';
  int nread;

  write(fd, &buf, 1);

  /* Early check to avoid unecessary wait for non-pipes
     or if the lock is already set. */
  format = check_format (fd);

  if (format == -1) {
    /* Wait until byte is consumed, we then know the lock status is up-to-date */

    do {
      if (ioctl(fd, FIONREAD, &nread) < 0)
	break;
      if (nread > 0)
	usleep (1000);
    } while (nread > 0);
    
    format = check_format (fd);
  }

  if (format == -1)
    return FORMAT_TEXT;
  return format;
}
