#ifndef __R5_DEFINES__
#define __R5_DEFINES__

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#define OK 0
#define ERREUR -1
#define BLOCK_SIZE 4                                  // octets
#define FILENAME_MAX_SIZE 32                          // caractères
#define MAX_FILE_SIZE (50 * 1024)                     // uchar
#define INODE_TABLE_SIZE 10                           // taille fixe = nb max fichiers
#define MAX_MSG 1024                                  // uchar
#define SUPER_BLOCK_SIZE 4                            // nb blocs avec parité
#define INODES_START SUPER_BLOCK_SIZE *BLOCK_SIZE + 1 // en octets
#define INODE_SIZE 11                                 // en blocks avec parité
#define FIRST_BYTE 4                                  // 4 octets

typedef unsigned int uint;   // même taille que int
typedef unsigned char uchar; // 8 bits = octet
enum raid {ZERO, UN, CINQ, ZERO_UN, UN_ZERO, CINQUANTE, CENT};

/* Type of a block of data */
typedef struct block_s {
  uchar data[BLOCK_SIZE]; // une case par octet
} block_t;

/* Type of the pseudo-inode structure */
typedef struct inode_s {
  char filename[FILENAME_MAX_SIZE]; // dont '\0'
  uint size;                        // du fichier en octets
  uint nblock;                      // nblock du fichier = (size+BLOCK_SIZE-1)/BLOCK_SIZE ?
  uint first_byte;                  // start block number on the virtual disk
} inode_t;

/* Type of the inode table */

typedef inode_t inode_table_t[INODE_TABLE_SIZE]; // la taille est fixe

typedef struct super_block_s {
  enum raid raid_type;
  uint nb_blocks_used;  //
  uint first_free_byte; // premier octet libre
} super_block_t;

/* Type of the virtual disk system */
typedef struct virtual_disk_s {
  int number_of_files;
  super_block_t super_block;
  inode_table_t inodes; // tableau
  int ndisk;
  enum raid raidmode; // type de RAID
  FILE **storage;     //tab[NUMBER_OF_DISKS];
} virtual_disk_t;

//=======================================================
typedef struct stripe_s {
  // inclut le bloc de parité
  int nblocks;     // egal à NB_DISK : un bloc par disque
  block_t *stripe; // stripe[NB_DISK] les data
} stripe_t;

typedef struct file_s {
  uint size;                 // Size of file in bytes
  uchar data[MAX_FILE_SIZE]; // only text files
} file_t;

///////////////////////////////////
/*         FUNCTIONS PART        */
///////////////////////////////////

int launcher(int ndisk, char *dir);

super_block_t init_super_block_t(enum raid raidtype);

block_t init_block_t(uchar data[]);

virtual_disk_t init_disk_raid5(int ndisk, char *directoryname);

int switch_raid5_off(virtual_disk_t *system);

int switch_raid5_on(virtual_disk_t *system, char* dirname);

/***
 * 
 * 
 * 
 * 
*/

uint get_file_size(FILE *fp);

char *getBlockData(block_t block);

unsigned compute_nblock(unsigned total_octal);

char write_block(virtual_disk_t *system, block_t *block, int diskid, uint block_pos);

char read_block(virtual_disk_t *system, block_t *block, int diskid, uint block_pos);

block_t block_repair(virtual_disk_t *system, stripe_t s, unsigned numfailed);

void afficher_block(block_t block);

void init_stripe_t(virtual_disk_t *system, stripe_t *s, block_t *blocks, int ndata);

unsigned compute_nstripe(virtual_disk_t *system, unsigned nblock);

block_t compute_parity(virtual_disk_t *system, stripe_t *s);

int parity_index(virtual_disk_t *system, uint block_pos);

char write_stripe(stripe_t *s, virtual_disk_t *raid, uint block_pos);

char read_stripe(virtual_disk_t *system, stripe_t* readed, uint block_pos);

void dump_stripe(stripe_t s, int ndisk, FILE *output);

char write_chunk(virtual_disk_t *system, file_t *text, uint pos);

char wipe_chunk(virtual_disk_t *system, inode_t *deleted);

char read_chunk(virtual_disk_t *system, file_t *text, inode_t *to_read);

bool check_data_correct(virtual_disk_t *system, stripe_t s_corrupt);

inode_t init_inode_t(char *filename, int nblocks, int first_byte, int size);

inode_t *read_inodes_table(virtual_disk_t *system);

char delete_inode(virtual_disk_t *system, int index);

inode_t read_inode(virtual_disk_t *system, int index);

char write_inode_table(virtual_disk_t *system, inode_t from);

int get_unused_inode(virtual_disk_t *system);

void update_firstFreeeByte(virtual_disk_t *system);

char write_file(virtual_disk_t *system, char *filename);

char delete_file(virtual_disk_t *system, char *filename);

char store_to_host(virtual_disk_t *system, char *filename, char* newname);

void dump_file(file_t *fp);

char read_file(virtual_disk_t *system, file_t *file, char *filename);

void dump_system(virtual_disk_t *system);

char defrag(virtual_disk_t *system);

void edit_file(virtual_disk_t *system, char *filename);

#endif // __R5_DEFINES__
