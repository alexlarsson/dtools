#include <stdio.h>
#include <unistd.h>

#include "pipeneg.h"

int
main(int argc, char *argv[])
{
  StreamFormat format;

  format = pipe_negotiate_format (STDOUT_FILENO);

  if (format == FORMAT_TEXT)
    printf ("Some text data\n");
  else
    printf ("Some binary data\n");

  return 0;
}

