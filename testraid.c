#include "raid_defines.h"

int main(int agrc, char* argv[]) {
    virtual_disk_t raid5 = init_disk_raid5(atoi(argv[2]), argv[1]);
    printf("cr : %d\n", switch_raid5_off(&raid5));
    printf("cr : %d\n", switch_raid5_on(&raid5, argv[1]));
    printf("compute_nblock(48) : %d\n", compute_nblock(48));
}