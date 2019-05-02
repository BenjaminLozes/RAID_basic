#include "raid_defines.h"

/**
 * @brief Initalise a stripe (parity included).
 * 
 * @param blocks : block array to set into the stripe
 * @param ndata : number of blocks to set
 * @return stripe_t : like stripe = [ [blocks] , parity_block ]
 */
void init_stripe_t(virtual_disk_t *system, stripe_t *s, block_t *blocks, int ndata) {
    //printf("\n> init_stripe_t \n");
    s->nblocks = ndata; // ADDING PARITY BLOCK
    if(s->nblocks > system->ndisk) // SI PLUS DE BLOCS QUE DE DISQUES
        s->nblocks = system->ndisk-1;
    s->stripe = (block_t *) malloc(system->ndisk * sizeof(struct block_s));
    if(ndata == 0) // CAS D'UNE INITIALISATION D'UNE STRIPE VIDE
        return;
    //printf("block's data writed :");
    for (int j = 0; j < s->nblocks; j++) { // TOUT BLOCS PARITY EXCLUS
        for(int k=0; k<BLOCK_SIZE; k++)
            s->stripe[j].data[k] = blocks[j].data[k];
        //printf("%s ", s->stripe[j].data);
    }
    //printf("\n");
    s->stripe[system->ndisk-1] = compute_parity(system, s);
    /*printf("<-> init_stripe_t done pour %d blocks: ", s->nblocks);
    for(int _=0; _<s->nblocks; _++)
        printf("%s ", getBlockData(s->stripe[_]));
    printf("%s \n", getBlockData(s->stripe[system->ndisk - 1]));*/
}

/**
 * @brief Compute the number of stripe necessary to set @nblock blocks.
 * 
 * @param system 
 * @param nblock : number of blocks to store (parity included)
 * @return unsigned : ERROR / OK
 */
unsigned compute_nstripe(virtual_disk_t *system, unsigned nblock) {
    printf("> compute nstripe for %d blocks: %d\n", nblock, (nblock/(system->ndisk-1)));
    if(nblock%(system->ndisk-1) != 0) 
        return (nblock / (system->ndisk-1)) + 1;
    return nblock / (system->ndisk-1); // +1 used to avoid bad convert like float 0.5 to int 0
}


block_t compute_parity(virtual_disk_t *system, stripe_t *s) {
    //printf(">> compute_parity for %d blocks.\n", s->nblocks);
    if(s->nblocks == 0) exit(ERREUR);
    block_t parity = init_block_t(s->stripe[0].data);
    for(int b=1; b<s->nblocks; b++) {
        if(b!=system->ndisk) {
            for(int it=0; it<BLOCK_SIZE; it++) {
                parity.data[it] = parity.data[it] ^ s->stripe[b].data[it]; // XOR is ^
            }
        }
    }
    //printf("<<-->> compute_parity done: '%s' \n", parity.data);
    return parity;
}

int parity_index(virtual_disk_t *system, uint block_pos) {
    return ((block_pos + system->ndisk - 1) % system->ndisk);
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
char write_stripe(stripe_t *s, virtual_disk_t *raid, uint block_pos) {
    int block_id = 0;
    int nb_block_writed = 0;
    int pos_parity = parity_index(raid, block_pos);
    for(int disk=0; disk < raid->ndisk; disk++) {
        if(disk == pos_parity) {
            if (OK != write_block(raid, &s->stripe[raid->ndisk - 1], disk, block_pos))
                return(ERREUR);
        }
        else {
            if (OK != write_block(raid, &s->stripe[block_id++], disk, block_pos))
                return(ERREUR);
            nb_block_writed++;
        }
    }
    //printf("<-> writed all to pos : %d\n", block_pos);
    return OK;
}

char read_stripe(virtual_disk_t *system, stripe_t *readed, uint block_pos) {
    readed->nblocks = 0;
    char cr_fread = 0;
    block_t temp = init_block_t(NULL);
    int parityIndex = parity_index(system, block_pos);
    bool all_data_read = false;
    for(int disk=0; disk < system->ndisk; disk++) {
        temp=init_block_t(NULL);
        if(disk == parityIndex) {
            cr_fread = read_block(system, &readed->stripe[system->ndisk-1], disk, block_pos) ^ cr_fread;
        }
        else if(!all_data_read) {
            cr_fread = read_block(system, &temp, disk, block_pos) ^ cr_fread;
            // ******************
            // TRANSFORMER EN FONCTION A PART ENTIERE
            bool blockIsNul = true;
            int each=0;
            while (each < BLOCK_SIZE && blockIsNul) {
                blockIsNul = (temp.data[each] == 0);
                each++;
            }
            // ******************
            if(blockIsNul) {
                all_data_read = true;
            }
            if(!blockIsNul) {
                cr_fread = read_block(system, &readed->stripe[readed->nblocks], disk, block_pos) ^ cr_fread;
                //afficher_block(readed->stripe[readed->nblocks]);
                readed->nblocks+=1;
            }
            //printf("\n");
        }
    }
    /*printf("<-> read_stripe nb blocs lus: %d --> ", readed->nblocks);
    dump_stripe(*readed, system->ndisk, stdout);
    printf("\n");*/
    return OK;
}

void dump_stripe(stripe_t s, int nbdisk, FILE *output){
    fprintf(output, "Ascii: ");
    for (int _ = 0; _ < s.nblocks; _++){
        for (int t = 0; t < BLOCK_SIZE; t++)
            printf("%c", s.stripe[_].data[t]);
        printf(" | ");
    }
    for (int _ = s.nblocks; _ < nbdisk - 1; _++)
        printf("NULL | ");
    for (int t = 0; t < BLOCK_SIZE; t++)
        printf("%c", s.stripe[nbdisk - 1].data[t]);
    fprintf(output, "\n");
}

char write_chunk(virtual_disk_t *system, file_t *text, uint pos) {
    printf("!!!------> write_chunck:\n");
    unsigned nblocks = compute_nblock(text->size);
    unsigned nstripes = compute_nstripe(system, nblocks);
    block_t* tmp_blocks = malloc((system->ndisk-1) * sizeof(struct block_s));
    stripe_t tmp_stripe;
    int bytes_writed = 0;
    //printf("==> %u blocks and %u stripes au total \n", nblocks, nstripes);
    for(int s=0; s<(int) nstripes; s++) {
        printf("%d%% écrit !\n", (int) 100*s/(nstripes-1));
        init_stripe_t(system, &tmp_stripe, tmp_blocks, 0);
        int b=0;
        while(b<system->ndisk-1 && bytes_writed < (int) text->size) {
            tmp_blocks[b] = init_block_t((uchar *) ((text->data)+bytes_writed));
            bytes_writed+=BLOCK_SIZE;
            b++;
        }
        init_stripe_t(system, &tmp_stripe, tmp_blocks, b);
        //dump_stripe(tmp_stripe, system->ndisk, stdout);
        write_stripe(&tmp_stripe, system, pos);
        pos+=1;
    }
    free(tmp_blocks);
    free(tmp_stripe.stripe);
    return OK;
}

char wipe_chunk(virtual_disk_t *system, inode_t *deleted) {
    file_t blank_file;
    blank_file.size = deleted->size;
    memset(blank_file.data, '\0', blank_file.size);
    if (write_chunk(system, &blank_file, deleted->first_byte) != OK)
        return ERREUR;
    return OK;
}

char read_chunk(virtual_disk_t *system, file_t *text, inode_t *to_read) {
    printf("!!!-----> READ CHUNK\n");
    text->size = to_read->size;
    memset(text->data, '\0', MAX_FILE_SIZE);
    stripe_t s;
    unsigned byte_read = 0;
    int pos = to_read->first_byte;
    while (byte_read < to_read->size){
        init_stripe_t(system, &s, NULL, 0);
        int verif = read_stripe(system, &s, pos);
        if(verif != OK) {
            printf("Read OK ? %d \n%d byte readed / %d \n", OK == verif, byte_read, to_read->size);
            return ERREUR;
        }
        pos+=1;
        for(int b=0; b<s.nblocks; b++) 
            for(int _=0; _<BLOCK_SIZE; _++) 
                text->data[byte_read++] = s.stripe[b].data[_];
        //dump_stripe(s, system->ndisk, stdout);
    }
    free(s.stripe);
    printf("< readed chunk\n");
    return OK;
}

bool check_data_correct(virtual_disk_t *system, stripe_t s_corrupt) {
    printf("> check defects in stripe:\n");
    //dump_stripe(s_corrupt, system->ndisk, stdout);
    block_t parity = s_corrupt.stripe[system->ndisk - 1];
    block_t xor_calc = init_block_t(s_corrupt.stripe[0].data);
    for (int t = 1; t < s_corrupt.nblocks; t++)
        for(int _=0; _<BLOCK_SIZE; _++)
            xor_calc.data[_] ^= s_corrupt.stripe[t].data[_];
    bool same = true;
    printf("parity founded: %s\n", getBlockData(xor_calc));
    for(int _=0; _<s_corrupt.nblocks; _++)
        same = same & (parity.data[_] == xor_calc.data[_]);
    printf("< stripe correct: %d.\n", same);
    return same;
}   
