#include "raid_defines.h"

inode_t init_inode_t(char *filename, int nblocks, int first_byte, int size) {
    inode_t inode;
    strcpy(inode.filename, filename);
    inode.first_byte = first_byte;
    inode.size = size;
    inode.nblock = compute_nblock(size);
    return inode;
}

inode_t *read_inodes_table(virtual_disk_t *system) {
    return system->inodes;
}

inode_t read_inode(virtual_disk_t *system, int index) {
    return system->inodes[index];
}

char delete_inode(virtual_disk_t *system, int index) {
    printf("> delete_inode (nb_file: %d) of file: %s\n", system->number_of_files, system->inodes[index].filename);
    if(index >= system->number_of_files)
        return ERREUR;
    for(int _=index; _<system->number_of_files-1; _++) {
        strcpy(system->inodes[_].filename, system->inodes[_+1].filename);
        system->inodes[_].first_byte = system->inodes[_+1].first_byte;
        system->inodes[_].nblock = system->inodes[_+1].nblock;
        system->inodes[_].size = system->inodes[_ + 1].size;
    }
    system->number_of_files-=1;
    update_firstFreeeByte(system);
    printf("< delete_inode, new nb_file: %d\n", system->number_of_files);
    return OK;
}

char write_inode_table(virtual_disk_t *system, inode_t from) {
    if(system->number_of_files >= INODE_TABLE_SIZE || system->number_of_files < 0)
        return ERREUR;
    system->inodes[system->number_of_files] = from;
    system->number_of_files+=1;
    update_firstFreeeByte(system);
    return OK;
}

int get_unused_inode(virtual_disk_t *system) {
    if(system->number_of_files >= INODE_TABLE_SIZE || system->number_of_files < 0)
        return ERREUR;
    return system->number_of_files;
}