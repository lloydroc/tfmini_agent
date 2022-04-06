#include "become_daemon.h"

int // returns 0 on success non-zero on error
become_daemon()
{
    // chdir to "/" and don't close fds
    return daemon(0, 1);
}

int
write_pidfile(char *file)
{
    pid_t pid;
    FILE *fp;
    fp = fopen(file, "w");
    if(fp == NULL)
    {
    return 1;
    }
    pid = getpid();
    fprintf(fp, "%d\n", pid);
    fclose(fp);
    return 0;
}
