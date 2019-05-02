#include "raid_defines.h"

#define BLOCK_DATA_SIZE BLOCK_SIZE*sizeof(uchar)

super_block_t init_super_block_t(enum raid raidtype) {
    super_block_t super_block;
    super_block.nb_blocks_used = 0;
    super_block.first_free_byte = sizeof(uint);
    super_block.raid_type = raidtype;
    return super_block;
}

block_t init_block_t(uchar data[]) {
    block_t block;
    uchar * temp = malloc(sizeof(uchar) * (BLOCK_SIZE+1));
    memset(temp, 0, BLOCK_SIZE);
    temp[BLOCK_SIZE] = '\0';
    for(int _=0; _<BLOCK_SIZE; _++)
        block.data[_] = (uchar) temp[_];
    if(data == NULL)
        return block;
    int datalen = strlen((char *)data);
    for (int _ = 0; _ < BLOCK_SIZE; _++)
        if (_ < datalen)
            block.data[_] = data[_];
    free(temp);
    //printf("<-> init_block_t data = %s \n", block.data);
    return block;
}

char* getBlockData(block_t block) {
    char* output = malloc((BLOCK_SIZE+1)*sizeof(uchar));
    for(int _=0; _<BLOCK_SIZE; _++)
        output[_] = block.data[_];
    output[BLOCK_SIZE] = '\0';
    return output;
}

unsigned compute_nblock(unsigned total_octal) {
    if(total_octal%BLOCK_SIZE==0) 
        return total_octal / BLOCK_SIZE;
    return total_octal / BLOCK_SIZE + 1;
}

char write_block(virtual_disk_t *system, block_t *block, int diskid, uint block_pos) {
    //printf("> write_block : '%s' at pos: %d\n", getBlockData(*block), block_pos);
    uint byte_pos = block_pos * BLOCK_SIZE;
    FILE *fp = system->storage[diskid];
    int cr_fseek = -99;
    if(ftell(fp) != byte_pos)
        cr_fseek = fseek(fp, byte_pos, SEEK_SET);
    int cr_fwrite = fwrite(block->data, sizeof(uchar), BLOCK_SIZE, fp);
    //printf("cr_fseek: %d & cr_fwrite: %d.\n< write_block.\n", cr_fseek, cr_fwrite);
    return OK;
}

char read_block(virtual_disk_t *system, block_t *block, int diskid, uint block_pos) {
    char cr_fseek = fseek(system->storage[diskid], block_pos*BLOCK_SIZE*sizeof(uchar), SEEK_SET);
    if (cr_fseek != 0)
        return ERREUR;
    char cr_fread = fread(block->data, sizeof(uchar), BLOCK_SIZE, system->storage[diskid]);
    if (!cr_fread)
        return ERREUR;
    return OK;
}

block_t block_repair(virtual_disk_t *system, stripe_t s, unsigned numfailed) {
    printf("> block_repair on block %d: %s\n", numfailed, getBlockData(s.stripe[numfailed]));
    assert((int) numfailed < s.nblocks);
    block_t repaired = init_block_t(s.stripe[system->ndisk-1].data);
    for(int k=0; k<s.nblocks; k++) {
        if(k != (int) numfailed) {
            //printf("--> Compare to k=%d : %s \n", k, getBlockData(s.stripe[k]));
            for(int c=0; c<BLOCK_SIZE; c++) {
                //printf("%c ^ %c =  %c || ", (char)repaired.data[c], (char)s.stripe[k].data[c], (char)s.stripe[k].data[c] ^ repaired.data[c]);
                repaired.data[c] = s.stripe[k].data[c] ^ repaired.data[c];
            }
            //printf("\n temp: %s \n", repaired.data);
        }
    }
    printf("< block_repair done: %s\n", repaired.data);
    return repaired;
}

void afficher_block(block_t block) {
    printf("<> Print Hexa: ");
    for(int _=0; _<BLOCK_SIZE; _++) 
        printf("%02X ", block.data[_]);
    printf("\n<> Print Asci: ");
    for(int _=0; _<BLOCK_SIZE; _++)
        printf("%c", block.data[_]);
    printf("\n");
}
