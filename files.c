#include "raid_defines.h"

uint get_file_size(FILE* fp) {
    int temp = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    uint size = (uint) ftell(fp);
    fseek(fp, temp, SEEK_SET);
    return size;
}

file_t init_file_t(char* filename) {
    FILE* fp = fopen(filename, "r");
    file_t file;
    file.size = get_file_size(fp);
    memset(file.data, '\0', MAX_FILE_SIZE);
    fseek(fp, 0L, SEEK_SET);
    fread(file.data, sizeof(uchar), file.size, fp);
    fclose(fp);
    printf("<-> init %d lenth file: %s\ntext: %s\n", file.size, filename, file.data);
    return file;
}

void dump_file(file_t *fp) {
    if(fp->size == 0)
        printf("\nFICHIER VIDE, RIEN A AFFICHER.\n\n");
    else
        printf("\n\t\tFILE OF %d BYTES :\n//////////////////////////////////////////////////\n\n%s\n\n//////////////////////////////////////////////////\n", fp->size, fp->data);
}

char read_file(virtual_disk_t *system, file_t *file, char *filename) {
    int index = -1;
    for(int i=0; i<system->number_of_files; i++) {
        //printf("strcmp(%s, %s) = %d\n", system->inodes[i].filename, filename, strcmp(system->inodes[i].filename, filename));
        if(0 == strcmp(system->inodes[i].filename, filename))
            index = i;
    }
    if(index == -1) {
        printf("ERREUR: Fichier demandé inexistant !\n");
        return ERREUR;
    }
    printf("read_chunk avec index = %d\n", index);
    if(OK != read_chunk(system, file, &system->inodes[index]))
        return ERREUR;
    return OK;
}

char delete_file(virtual_disk_t *system, char* filename) {
    printf("> Delete file: %s\n", filename);
    int index = -1;
    for(int _=0; _<system->number_of_files; _++)
        if(strcmp(system->inodes[_].filename, filename) == 0)
            index = _;
    if(index == -1) {
        printf("ERREUR: Fichier inexistant en inodes.\n");
        return ERREUR;
    }
    if (wipe_chunk(system, &system->inodes[index]) != OK)
        return ERREUR;
    delete_inode(system, index);
    return OK;
}

char write_file(virtual_disk_t *system, char* filename) {
    printf("\n\n> write file %s\n", filename);
    int first_byte;
    int index = -1;
    file_t file = init_file_t(filename);
    file_t blank;
    if(file.size == 0) {
        printf("ERREUR: Fichier vide.\n");
        return ERREUR;
    }
    int nblocks = compute_nblock(file.size);
    for (int i = 0; i < system->number_of_files; i++)
        if(strcmp(system->inodes[i].filename, filename) == 0)
            index = i;
    if(index != -1) {
        printf("Already in. \n");
        if (OK != wipe_chunk(system, &system->inodes[index]))
            return ERREUR;
        printf("stocked size: %d  &&  new size: %d\n", system->inodes[index].size, file.size);
        if (system->inodes[index].size <= file.size) {
            // SI LA TAILLE EST IDENTIQUE OU INFÉRIEURE À AVANT, ON RÉÉCRIT PAR DESSUS.
            first_byte = system->inodes[index].first_byte;
            system->inodes[index] = init_inode_t(filename, nblocks, first_byte, file.size);
            update_firstFreeeByte(system);
            write_chunk(system, &file, first_byte);
            printf("< write_file done en réécriture.\n");
            read_file(system, &blank, filename);
            return OK;
        }
        else if(index != system->number_of_files-1) { 
            // SI LA TAILLE DU NOUVEAU EST PLUS GRANDE ET QUE CE N'ÉTAIT PAS LA DERNIERE INODE.
            if(delete_inode(system, index) != OK)
                return ERREUR;
        }
    }
    first_byte = system->super_block.first_free_byte;
    printf("inode[%d] = init(nblock: %d, ffbyte: %d, size: %d) \n"
            ,system->number_of_files, nblocks, first_byte, file.size);
    write_inode_table(system, init_inode_t(filename, nblocks, first_byte, file.size));
    write_chunk(system, &file, first_byte);
    read_file(system, &blank, filename);
    printf("< write file done.\n\n\n");
    return OK;
}

char store_to_host(virtual_disk_t *system, char* filename, char* newname) {
    printf("> Store file: %s\n", filename);
    int index = -1;
    for (int _ = 0; _ < system->number_of_files; _++)
        if (strcmp(system->inodes[_].filename, filename) == 0)
            index = _;
    if (index == -1) {
        printf("ERREUR: Fichier inexistant en inodes.\n");
        return ERREUR;
    }
    file_t readed;
    printf("read_chunk avec index = %d\n", index);
    if (OK != read_chunk(system, &readed, &system->inodes[index]))
        return ERREUR;
    //read_file(system, &readed, filename);
    FILE *new = fopen(newname, "ab+");
    fseek(new, 0L, SEEK_SET);
    fwrite(readed.data, sizeof(uchar), readed.size, new);
    fclose(new);
    return OK;
}

char defrag(virtual_disk_t *system) {
    printf("> DEFRAGMENTATION\n");
    dump_system(system);
    uint nextPos = FIRST_BYTE;
    if(system->number_of_files < 1)
        return ERREUR;
    for(int f=0; f<system->number_of_files; f++) {
        if(f > 0)
            nextPos = system->inodes[f-1].first_byte + compute_nstripe(system, system->inodes[f-1].nblock);
        if(nextPos < system->inodes[f].first_byte) {
            printf("--> rewrite %s to pos %d\n", system->inodes[f].filename, nextPos);
            file_t temp;
            read_chunk(system, &temp, &system->inodes[f]);
            wipe_chunk(system, &system->inodes[f]);
            write_chunk(system, &temp, nextPos);
            system->inodes[f].first_byte = nextPos;
        }
    }
    update_firstFreeeByte(system);
    dump_system(system);
    printf("< DEFRaGMENTED\n");
    return OK;
}