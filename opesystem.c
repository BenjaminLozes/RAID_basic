#include "raid_defines.h"

void edit_file(virtual_disk_t *system, char* filename) {
    store_to_host(system, filename, "tempfile.tmp");
    delete_file(system, filename);
    pid_t child = fork();
    pid_t wait_pid;
    int child_cr;
    if(child == 0) {
        execlp("subl", "subl", "tempfile.tmp", 0);

        exit(0);
    }
    else {
        do {
            wait_pid = wait(&child_cr);
            printf("wait: %d et pid: %d et ppid: %d\n", (int) wait_pid, getpid(), getppid());
            sleep(1);
        } while(wait != child);
        write_file(system, "tempfile.tmp");
        int k = 0;
        while (k < system->number_of_files)
            if (strcmp(system->inodes[k].filename, "tempfile.tmp") == 0)
                strcpy(system->inodes[k].filename, filename);
        execlp("rm", "tempfile.tmp", 0);
    }
}

    /*execlp("gedit", "gedit", "tempfile.tmp", 0);
    write_file(system, "tempfile.tmp");
    int k = 0;
    while (k < system->number_of_files)
        if (strcmp(system->inodes[k].filename, "tempfile.tmp") == 0)
            strcpy(system->inodes[k].filename, filename);
    execlp("rm", "tempfile.tmp", 0);
}*/