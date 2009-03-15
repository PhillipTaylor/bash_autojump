
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <time.h>
#include <unistd.h>
#include "autojump.h"

int main()
{
  autojump_init();

  autojump_directory_changed("/home/phill");

  sleep(5);

  autojump_directory_changed("/etc");

  autojump_jumpstat();
  sleep(3);

  autojump_directory_changed("/home/phill");

  sleep(7);

  autojump_directory_changed("/opt/localinstall");

  autojump_jumpstat();

  printf("j => %s\n", autojump_jump("j"));

  printf("ill => %s\n", autojump_jump("ill"));

  printf("et => %s\n", autojump_jump("et"));

  autojump_directory_changed("/etc/sysconfig");

  sleep(6);

  autojump_directory_changed("/etc");

  printf("et => %s\n", autojump_jump("et"));

  autojump_jumpstat();
  printf("all => %s\n", autojump_jump("all"));

  printf("ill => %s\n", autojump_jump("ill"));

  autojump_jumpstat();
  autojump_exit();

  return 0;
}
