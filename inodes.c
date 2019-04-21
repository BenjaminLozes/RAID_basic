#include "raid_defines.h"

inode_t *read_inodes_table(virtual_disk_t *system) {
    return system->inodes;
}