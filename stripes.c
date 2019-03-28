#include "raid_defines.h"

unsigned compute_nstripe(virtual_disk_t *system, unsigned nblock)
{
    if(nblock%system->ndisk == 0) return nblock / system->ndisk;
    return nblock / system->ndisk + 1;
}

/**
 * @brief compute the parity block of stripe_t @param s
 * 
 * @param s : pre-initialised as s->data[1:nblocks-2] 
 * s->data[nblocks-1] gonna be the parity bloc => Initially not allowed
 * 
 * return parity : the parity block
 */
block_t compute_parity(stripe_t *s) {
    if(s->nblocks == 0) exit(ERREUR);
    block_t parity = init_block_t(s->stripe[0].data);
    for(int b=1; b<s->nblocks-1; b++) {
        for(int it=0; it<BLOCK_SIZE; it++)
            parity.data[it] = parity.data[it] ^ s->stripe[b].data[it]; // XOR is ^
    }
    return parity;
}

int parity_index(stripe_t s) {
    static int parity_index = 4;
    parity_index--;
    if(parity_index >= s.nblocks)
        parity_index = s.nblocks-1;
    if(parity_index < 0)
        parity_index = s.nblocks - 1;
    return parity_index;
}

unsigned write_stripe(stripe_t *s, virtual_disk_t *raid, int pos) {
    int check_write;
    int disk_id = 0;
    int block_id = 0;
    block_t parity = compute_parity(s);
    int pos_parity = parity_index(*s);
    while(disk_id < raid->ndisk) {
        if(disk_id == pos_parity)
            check_write = fwrite(&parity, pos, sizeof(parity), raid->storage[disk_id]);
        else
            check_write = fwrite(&s->stripe[block_id++], pos, s->nblocks * sizeof(uchar), raid->storage[disk_id]);
        disk_id++;
        if(!(check_write == sizeof(parity)) || check_write == s->nblocks*sizeof(uchar))
            return ERREUR;
    }
    return OK;
}

