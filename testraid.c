#include "raid_defines.h"

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Enter ./PROGRAMM NBOFDISKS DISK_DIRECTORY \n");
        return 1;
    }
    virtual_disk_t raid5 = init_disk_raid5(atoi(argv[2]), argv[1]);
    printf("cr off: %d\n", switch_raid5_off(&raid5));
    printf("cr on : %d\n", switch_raid5_on(&raid5, argv[1]));

    uchar *mainData = (uchar *)"chocolatedon";
    unsigned data_length = compute_nblock(strlen((char *)mainData));
    printf("compute_nblock(%lu): %u\n", strlen((char *)mainData), data_length);

    block_t *block= malloc(data_length * sizeof(struct block_s));
    for (unsigned n = 0; n < data_length; n++) {
        block[n] = init_block_t(mainData);
        mainData += 4;
    } 
    printf("!! data en out init: %s\n", mainData);

    stripe_t stripe;
    init_stripe_t(&raid5, &stripe, block, data_length);
    stripe.stripe[1].data[2] = 'F';

    printf("On casse le block : %s\nPuis on verifie stripe entiere: ", stripe.stripe[1].data);
    dump_stripe(stripe, stdout);
    block_t test = block_repair(stripe, 1);
    printf("\n\n\n");
    stripe.stripe[1] = test;
    dump_stripe(stripe, stdout);
    printf("parity_indexcensé décroître: ");
    for(int k=0; k<10; k++)
        printf("%d, ", parity_index(stripe));
    printf("\n\n");
}