#include "raid_defines.h"

#define BLOCK_DATA_SIZE BLOCK_SIZE*sizeof(uchar)

super_block_t init_super_block_t(enum raid raidtype) {
    super_block_t super_block;
    super_block.nb_blocks_used = 0;
    super_block.raid_type = raidtype;
    return super_block;
}

block_t init_block_t(uchar data[]) {
    printf("> init_block_t: %s\n", data);
    block_t block;
    uchar * temp = malloc(sizeof(uchar) * (BLOCK_SIZE+1));
    for(char _=0; _<BLOCK_SIZE; _++)
        temp[_] = data[_];
    temp[BLOCK_SIZE] = '\0';
    printf(">> Temp in init: %s\n", temp);
    for(char _=0; _<BLOCK_SIZE; _++)
        block.data[_] = (uchar) temp[_];
    free(temp);
    printf("< init_block_t data = %s \n\n", block.data);
    return block;
}

char *getBlockData(block_t block) {
    char* output = malloc((BLOCK_SIZE+1)*sizeof(uchar));
    for(char _=0; _<BLOCK_SIZE; _++)
        output[_] = block.data[_];
    output[BLOCK_SIZE] = '\0';
    return output;
}

unsigned compute_nblock(unsigned total_octal) {
    if(total_octal%BLOCK_SIZE==0) 
        return total_octal / BLOCK_SIZE;
    return total_octal / BLOCK_SIZE + 1;
}

char write_block(virtual_disk_t *system, block_t *block, int diskid, int *pos) {
    printf("> Write_block: %s \n", block->data);
    char cr_fseek = fseek(system->storage[diskid], *pos, SEEK_SET);
    if(cr_fseek != 0)
        return ERREUR;
    printf("Preparing a %lu array.\n", BLOCK_DATA_SIZE);
    fwrite(block, BLOCK_DATA_SIZE, 1, system->storage[diskid]);
    pos+=BLOCK_DATA_SIZE;
    return OK;
}

char read_block(virtual_disk_t *system, block_t *block, int diskid, int pos) {
    char cr_fseek = fseek(system->storage[diskid], pos, SEEK_SET);
    if (cr_fseek != 0)
        return ERREUR;
    char cr_fread = fread(block, BLOCK_DATA_SIZE, 1, system->storage[diskid]);
    if (sizeof(struct block_s) != cr_fread)
        return ERREUR;
    return OK;
}

block_t block_repair(stripe_t s, unsigned numfailed) {
    printf("> block_repair on block %d: %s\n", numfailed, getBlockData(s.stripe[numfailed]));
    assert(numfailed < s.nblocks);
    block_t repaired = init_block_t(s.stripe[s.nblocks-1].data);
    for(char k=0; k<s.nblocks-1; k++) {
        if(k != numfailed) {
            printf("--> Compare to k=%d : %s \n", k, getBlockData(s.stripe[k]));
            for(int c=0; c<BLOCK_SIZE; c++) {
                printf("%c ^ %c =  %c || ", (char)repaired.data[c], (char)s.stripe[k].data[c], (char)s.stripe[k].data[c] ^ repaired.data[c]);
                repaired.data[c] = s.stripe[k].data[c] ^ repaired.data[c];
            }
            printf("\n temp: %s \n", repaired.data);
        }
    }
    printf("\n< block_repair done: %s\n", repaired.data);
    return repaired;
}

void afficher_block(block_t block) {
    printf("<> Print Hexa: ");
    for(int _=0; _<BLOCK_SIZE; _++) 
        printf("%02X ", block.data[_]);
    printf("\n<> Print Asci: %s\n", block.data);
}
