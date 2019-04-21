#include "raid_defines.h"

virtual_disk_t init_disk_raid5(int ndisk, char* dirname) {
    printf("> init_disk_raid5 for %d disks from %s.\n", ndisk, dirname);
    virtual_disk_t system;
    system.number_of_files = 0;
    system.ndisk = ndisk;
    system.raidmode = CINQ;
    system.super_block.raid_type = system.raidmode;
    system.super_block.nb_blocks_used=0;
    system.storage = (FILE **) malloc(ndisk * sizeof(FILE*));
    int pathlen = strlen(dirname) + 4;
    char filename[pathlen];
    for (int k = 0; k < ndisk; k++) {
        snprintf(filename, pathlen, "%s/d%d", dirname, k);
        // AJOUTER LES FILENAMES A L'INODE DE SYSTEM
        printf("storage[%d]: %s\n", k, filename);
        system.storage[k] = fopen(filename, "ab+");
    }
    printf("< init_disk_raid5 done.\n");
    return system;
}

int switch_raid5_off(virtual_disk_t *system) {
    for(int k=0; k<system->ndisk; k++)
        if(fclose(system->storage[k]) != 0)
            return ERREUR;
    return OK; 
}

int switch_raid5_on(virtual_disk_t *system, char* dirname) {
    char filename[strlen(dirname) + 4];
    for (int k = 0; k < system->ndisk; k++) {
        snprintf(filename, strlen(dirname) + 4, "%s/d%d", dirname, k);
        system->storage[k] = fopen(filename, "ab+");
    }
    return OK;
}

