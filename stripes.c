#include "raid_defines.h"

/**
 * @brief Initalise a stripe (parity included).
 * 
 * @param blocks : block array to set into the stripe
 * @param ndata : number of blocks to set
 * @return stripe_t : like stripe = [ [blocks] , parity_block ]
 */
void init_stripe_t(virtual_disk_t *system, stripe_t *s, block_t *blocks, int ndata) {
    printf("\n> init_stripe_t \n");
    s->nblocks = ndata+1; // Adding the parity block to the number of blocks
    s->stripe = (block_t *) malloc(system->ndisk * sizeof(struct block_s));
    printf("block's data writed :");
    for (int j = 0; j < ndata; j++) {
        for(int k=0; k<BLOCK_SIZE; k++)
            s->stripe[j].data[k] = blocks[j].data[k];
        printf("%s ", s->stripe[j].data);
    }
    printf("\n");
    s->stripe[ndata] = compute_parity(s);
    printf("< init_stripe_t done.\n\n\n");
}

/**
 * @brief Compute the number of stripe necessary to set @nblock blocks.
 * 
 * @param system 
 * @param nblock : number of blocks to store (parity included)
 * @return unsigned : ERROR / OK
 */
unsigned compute_nstripe(virtual_disk_t *system, unsigned nblock) {
    if(nblock%(system->ndisk) == 0) 
        return( nblock / (system->ndisk) );
    return( (nblock / system->ndisk) + 1); // +1 used to avoid bad convert like float 0.5 to int 0
}

/**
 * @brief compute the parity block of stripe_t @s
 * 
 * @param s : pre-initialised as s->data[1:nblocks-2] 
 * s->data[nblocks-1] gonna be the parity bloc => Initially not allowed
 * return parity : the parity block
 */
block_t compute_parity(stripe_t *s) {
    printf(">> compute_parity \n");
    if(s->nblocks == 0) exit(ERREUR);
    block_t parity = init_block_t(s->stripe[0].data);
    for(int b=1; b<s->nblocks-1; b++) {
        for(int it=0; it<BLOCK_SIZE; it++) 
            parity.data[it] = parity.data[it] ^ s->stripe[b].data[it]; // XOR is ^
    }
    printf("<< compute_parity done: %s \n", parity.data);
    return parity;
}

/**
 * @brief 
 * 
 * @param s 
 * @return int 
 */
int parity_index(stripe_t s) {
    static int parity_index = 4;
    parity_index--;
    if(parity_index >= s.nblocks)
        parity_index = s.nblocks-1;
    if(parity_index < 0)
        parity_index = s.nblocks - 1;
    return parity_index;
}

/**
 * @brief Write on the @raid the initialised stripe @s
 * 
 * @param s : stripe to store
 * @param raid : system where disks are stored
 * @param pos : location where stripe @s needs to store their blocks 
 *              (prevent overwriting previously stored blocks on disks)
 * @return unsigned : ERROR / OK
 */
unsigned write_stripe(stripe_t *s, virtual_disk_t *raid, int pos) {
    int check_write;
    int disk_id = 0;
    int block_id = 0;
    int pos_parity = parity_index(*s);
    while(disk_id < raid->ndisk) {
        if(disk_id == pos_parity) {
            for(char _ = 0; _<BLOCK_SIZE; _++) {
                if(sizeof(uchar) != fwrite(&s->stripe[s->nblocks-1].data[_], pos, sizeof(uchar), raid->storage[disk_id]))
                    return ERREUR;
                pos+=sizeof(uchar);
            }
        }
        else
            check_write = write_block(raid, &s->stripe[block_id++], disk_id, pos);
        disk_id++;
        if(check_write != OK)
            return ERREUR;
        pos += (sizeof(uchar)*BLOCK_SIZE);
    }
    return OK;
}

void dump_stripe(stripe_t s, FILE* output) {
    fprintf(output, "Ascii: ");
    for(char _=0; _<s.nblocks; _++) {
        for(char t=0; t<BLOCK_SIZE; t++)
            printf("%c", s.stripe[_].data[t]);
        printf(" | ");
    }
        
    fprintf(output, "\n");
}

