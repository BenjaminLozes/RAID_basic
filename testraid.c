#include "raid_defines.h"

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Enter ./PROGRAMM NBOFDISKS DISK_DIRECTORY \n");
        return 1;
    }
    /*virtual_disk_t raid5 = init_disk_raid5(atoi(argv[1]), argv[2]);
    write_file(&raid5, "testfile.txt");
    dump_system(&raid5);
    write_file(&raid5, "testfile2.txt");
    dump_system(&raid5);
    write_file(&raid5, "testfile3.txt");
    dump_system(&raid5);
    //file_t file = read_chunk(&raid5, &raid5.inodes[0]);
    //file = read_chunk(&raid5, &raid5.inodes[1]);
    printf("\n\n---------------------------------------\n\n");
    file_t file;
    read_file(&raid5, &file, "testfile.txt");
    dump_file(&file);
    read_file(&raid5, &file, "testfile2.txt");
    dump_file(&file);
    read_file(&raid5, &file, "testfile3.txt");
    dump_file(&file);
    defrag(&raid5);
    read_file(&raid5, &file, "testfile.txt");
    dump_file(&file);
    read_file(&raid5, &file, "testfile2.txt");
    dump_file(&file);
    read_file(&raid5, &file, "testfile3.txt");
    dump_file(&file);

    delete_file(&raid5, "testfile.txt");
    dump_system(&raid5);
    read_file(&raid5, &file, "testfile2.txt");
    dump_file(&file);
    defrag(&raid5);*/

    return launcher(atoi(argv[1]), argv[2]);
}