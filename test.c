
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <time.h>
#include <unistd.h>
#include "autojump.h"

int main()
{
  autojump_init();

  autojump_jumpstat();
  printf("-------\n");

  autojump_directory_changed("/usr");
  autojump_directory_changed("/home");
  sleep(5);
  autojump_directory_changed("/bin");

  autojump_jumpstat();

  autojump_directory_changed("/tmp");

  autojump_jumpstat();
  autojump_exit();

  return 0;
}
