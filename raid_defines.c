#include "raid_defines.h"

int switch_raid5_off(virtual_disk_t *system) {
    printf("> Switching off the system.\n");
    for(int k=0; k<system->ndisk; k++) {
        if(fclose(system->storage[k]) != 0)
            return ERREUR;
        printf("storage number %d closed.\n", k);
    }
    free(system->storage);
    return OK; 
}

int delete_system(virtual_disk_t *system) {
    for(int _=0; _<system->number_of_files; _++)
        delete_file(system, system->inodes[_].filename);
    switch_raid5_off(system);
    return OK;
}

void update_firstFreeeByte(virtual_disk_t *system) {
    //printf("> new ffbyte is (nb_file:%d) : %d + %d.\n", system->number_of_files, system->inodes[system->number_of_files-1].first_byte, compute_nstripe(system, system->inodes[system->number_of_files - 1].nblock));
    if(system->number_of_files == 0)
        system->super_block.first_free_byte = FIRST_BYTE;
    else
        system->super_block.first_free_byte = system->inodes[system->number_of_files - 1].first_byte + compute_nstripe(system, system->inodes[system->number_of_files - 1].nblock);
    system->super_block.nb_blocks_used = 0;
    for(int _=0; _<system->number_of_files; _++)
        system->super_block.nb_blocks_used += system->inodes[_].nblock;
    uint parity = ((uint)system->super_block.raid_type) ^ system->super_block.nb_blocks_used ^ system->super_block.first_free_byte;
    int postemp1 = ftell(system->storage[1]);
    int postemp2 = ftell(system->storage[2]);
    int postempLast = ftell(system->storage[system->ndisk - 1]);
    fseek(system->storage[1], 0L, SEEK_SET);
    fwrite(&system->super_block.nb_blocks_used, sizeof(uint), 1, system->storage[1]);
    fseek(system->storage[2], 0L, SEEK_SET);
    fwrite(&system->super_block.first_free_byte, sizeof(uint), 1, system->storage[2]);
    fseek(system->storage[system->ndisk - 1], 0L, SEEK_SET);
    fwrite(&parity, sizeof(uint), 1, system->storage[system->ndisk - 1]);
    fseek(system->storage[1], postemp1, SEEK_SET);
    fseek(system->storage[2], postemp2, SEEK_SET);
    fseek(system->storage[system->ndisk - 1], postempLast, SEEK_SET);
    printf("<-> New RAID first_free_byte is %d.\n", system->super_block.first_free_byte);
}

int get_free_space(virtual_disk_t *system) {
    return get_file_size(system->storage[0]) - system->super_block.first_free_byte;
}

int switch_raid5_on(virtual_disk_t *system, char* dirname) {
    printf("> Switching on the system for the repertory: %s\n", dirname);
    int pathlen = strlen(dirname) + 4;
    char filename[pathlen];
    for (int k = 0; k < system->ndisk; k++) {
        snprintf(filename, pathlen, "%s/d%d", dirname, k);
        printf("storage %s opened.\n", filename);
        system->storage[k] = fopen(filename, "r+");
    }
    return OK;
}

char* string_raid(enum raid type) {
    if(type == CINQ)
        return "cinq";
    return "autre";
}

void dump_system(virtual_disk_t *system) {
    printf(">>>> System <<<<\n");
    printf("- number_of_files: %d\n", system->number_of_files);
    printf("- raid_type: %s\n", string_raid(system->raidmode));
    printf("- ndisk: %d\n", system->ndisk);
    printf("- free bytes: %d\n", get_free_space(system));
    printf("- super_block:\n\t- nb_used: %d\n\t- first_free: %d\n", system->super_block.nb_blocks_used, system->super_block.first_free_byte);
    for(int k=0; k<system->number_of_files; k++) {
        printf("- inodes[%d]:\n", k);
        printf("\tfilename: %s\n", system->inodes[k].filename);
        printf("\tfirst_byte: %d\n", system->inodes[k].first_byte);
        printf("\tblock: %d\n", system->inodes[k].nblock);
        printf("\tsize: %d\n", system->inodes[k].size);
    }
    printf("<<<<         >>>>\n");
}

virtual_disk_t init_disk_raid5(int ndisk, char *dirname) {
    printf("> init_disk_raid5 for %d disks from %s.\n", ndisk, dirname);
    virtual_disk_t system;
    system.number_of_files = 0;
    system.ndisk = ndisk;
    system.raidmode = CINQ;
    system.super_block.raid_type = system.raidmode;
    system.super_block.nb_blocks_used = 0;
    system.super_block.first_free_byte = FIRST_BYTE;
    system.storage = (FILE **)malloc(ndisk * sizeof(FILE *));
    switch_raid5_on(&system, dirname);
    uint parity = ((uint)system.super_block.raid_type ) ^ system.super_block.nb_blocks_used ^ system.super_block.first_free_byte;
    fwrite(&system.super_block.raid_type, sizeof(enum raid), 1, system.storage[0]);
    fwrite(&system.super_block.nb_blocks_used, sizeof(uint), 1, system.storage[1]);
    fwrite(&system.super_block.first_free_byte, sizeof(uint), 1, system.storage[2]);
    fwrite(&parity, sizeof(uint), 1, system.storage[system.ndisk-1]);
    printf("< init_disk_raid5 done for %d disks.\n", system.ndisk);
    dump_system(&system);
    return system;
}

int launcher(int ndisk, char* dir) {
    int init = false;
    int choix;
    int on = false;
    file_t temp_file;
    char* filename = malloc(50 * sizeof(char));
    char *newname = malloc(50 * sizeof(char));
    virtual_disk_t system;
    while(true) {
        printf("\nSystem Init: %d ON: %d\nChoix possibles:\n\t(1) Initialise\n\t(2) Write\n\t(3) Read\n\t(4) System preview", init, on);
        printf("\n\t(5) Check disks\n\t(6) Turn system OFF\n\t(7) Turn system ON\n\t(8) Defrag disks\n\t(9) Delete");
        printf("\n\t(10) Store to host\n\t(11) Edit with sublime-text \n\t(12) Wipe & Close\n\n");
        printf("Votre choix: ");

        fflush(stdin);
        scanf("%d", &choix);
        printf("\n");
        switch(choix) {
            case 1: 
                system = init_disk_raid5(ndisk, dir);
                init = true;
                on = true;
                break;
            case 2: 
                printf("filename: ");
                scanf("%s", filename);
                printf("\n");
                write_file(&system, filename);
                break;
            case 3: 
                printf("filename: ");
                scanf("%s", filename);
                printf("\n");
                read_file(&system, &temp_file, filename);
                dump_file(&temp_file);
                break;
            case 4:
                dump_system(&system);
                break;
            case 5:
                printf("Unusuable.\n");
                break;
            case 6:
                switch_raid5_off(&system);
                on = false;
                break;
            case 7:
                switch_raid5_on(&system, dir);
                on = true;
                break;
            case 8:
                defrag(&system);
                break;
            case 9: 
                printf("filename: ");
                scanf("%s", filename);
                printf("\n");
                delete_file(&system, filename);
                break;
            case 10: 
                printf("filename: ");
                scanf("%s", filename);
                printf("\n");
                printf("new name: ");
                scanf("%s", newname);
                printf("\n");
                store_to_host(&system, filename, newname);
                break;
            case 11:
                printf("filename: ");
                scanf("%s", filename);
                printf("\n");
                edit_file(&system, filename);
            case 12:
                free(filename);
                return delete_system(&system);
            default: 
                choix = -1;
                break;
        }
    }
}
