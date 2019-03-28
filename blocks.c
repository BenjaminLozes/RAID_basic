#include "raid_defines.h"

super_block_t init_super_block_t(enum raid raidtype) {
    super_block_t super_block;
    super_block.nb_blocks_used = 0;
    super_block.raid_type = raidtype;
    return super_block;
}

block_t init_block_t(uchar data[]) {
    block_t block;
    for (int _ = 0; _ < BLOCK_SIZE; _++)
        block.data[_] = data[_];
    return block;
}

unsigned compute_nblock(unsigned total_octal) {
    if(total_octal%BLOCK_SIZE==0)
        return total_octal / BLOCK_SIZE;
    return total_octal / BLOCK_SIZE + 1;
}

int write_block(virtual_disk_t *system, block_t block, int diskid, int pos) {
    int cr_fseek = fseek(system->storage[diskid], pos * sizeof(struct block_s), SEEK_SET);
    if(cr_fseek != 0)
        return ERREUR;
    int cr_fwrite = fwrite(&block , 1, sizeof(struct block_s), system->storage[diskid]);
    if(sizeof(struct block_s) != cr_fwrite)
        return ERREUR;
    return OK;
}

int read_block(virtual_disk_t *system, block_t *block, int diskid, int pos) {
    int cr_fseek = fseek(system->storage[diskid], pos * sizeof(struct block_s), SEEK_SET);
    if (cr_fseek != 0)
        return ERREUR;
    int cr_fread = fread(block, sizeof(struct block_s), 1, system->storage[diskid]);
    if (sizeof(struct block_s) != cr_fread)
        return ERREUR;
    return OK;
}

int block_repair() {
    return 0;
}

void afficher_block(block_t block) {
    for(int _=0; _<BLOCK_SIZE; _++) 
        printf("%02X ", block.data[_]);
    printf("\n");
}
