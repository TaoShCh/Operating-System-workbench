#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_STR 500
#define MAX_FILE 500
char name[MAX_STR];
typedef struct BMP_FILE{
    char BMP_name[MAX_STR];
    uint32_t BMP_offset;
    uint32_t BMP_size;
}BMP_dir;
BMP_dir bmp[MAX_FILE];
int bmp_num = 0;
extern char **environ;

void get_name(char *buf){
    memset(name,0,sizeof(name));
    int len;
    for(len = 0;len < 8;len++){
         if (buf[len] == ' ') break;
         name[len] = buf[len];
    }
    name[len++] = '.';
    memcpy(name + len, buf + 8, 3);
}

BMP_dir *find_matching_dir(int offset){
    //printf("%d %d\n", bmp_num,offset);
    for(int i = 0;i < bmp_num; i++){
        if(bmp[i].BMP_offset ==  offset){
            return &bmp[i];
        }
    }
    //printf("find error!\n");
    return NULL;
}

//FAT32 has no root directory region and directory entries are not continuous!!
//cluster start with index 2
int main(int argc, char *argv[]) {
    char filename[MAX_STR];
    strcpy(filename,argv[1]);
    int fd = open(filename,O_RDONLY); assert(fd != -1);
    uint8_t *fs = mmap(NULL, 1<<27, PROT_READ,MAP_SHARED, fd, 0);
    assert(fs != MAP_FAILED);
    uint16_t sector_size = *(uint16_t *)(&fs[0xb]);
    uint32_t cluster_size = *(uint8_t *)(&fs[0xd]) * sector_size;
    uint32_t reserved_size = *(uint16_t *)(&fs[0xe]) * sector_size;//in bytes
    uint8_t num_of_FAT = *(uint8_t *)(&fs[0x10]);
    uint32_t FAT_size = *(uint32_t *)(&fs[0x24]) * sector_size;//in bytes
    uint32_t fs_total_size = *(uint32_t *)(&fs[0x20]) * sector_size;//in bytes
    uint32_t first_cluster_offset = reserved_size + num_of_FAT * FAT_size;

    //printf("%x\n",(int)dir_offset);

    //go through the filesystem and find all entries, get a map from file offset to filename
    for(int i = first_cluster_offset; i < fs_total_size; i += 32){
        if(fs[i + 8] == 'B' && fs[i + 9] == 'M' && fs[i + 10] == 'P'){//find the BMP dirctory
            if(fs[i] == 0xe5) continue;//0xe5 on the first byte implies the file was deleted
            get_name((char *)&fs[i]);
            //printf("%s\n",name);
            uint32_t cluster_index = ((uint32_t)*(uint16_t *)(&fs[i + 0x14]) << 16) | *(uint16_t *)(&fs[i + 0x1a]);
            uint32_t file_offset = first_cluster_offset + (cluster_index - 2) * cluster_size;
            uint32_t file_size = *(uint32_t *)(&fs[i + 0x1c]);//in bytes
            strcpy(bmp[bmp_num].BMP_name, name);
            bmp[bmp_num].BMP_offset = file_offset;
            bmp[bmp_num].BMP_size = file_size;
            bmp_num++;
        }
    }
    //scan all the cluster and find BMP_header
    //int count=0;
    for(int i = first_cluster_offset; i < fs_total_size; i += cluster_size){
        if(fs[i] == 'B' && fs[i + 1] == 'M'){//find the BMP header
            char this_bmpname[MAX_STR];
            BMP_dir *this_bmpdir = find_matching_dir(i);
            if(this_bmpdir == NULL) continue;// directory entry missing
            strcpy(this_bmpname, this_bmpdir->BMP_name);
            uint32_t actual_file_size = *(uint32_t *)(&fs[i + 2]);
            assert(actual_file_size == this_bmpdir -> BMP_size);
            //printf("%d %d\n", data_offset, actual_file_size % cluster_size);


            int sha1sum_input_pipe[2];
            int sha1sum_output_pipe[2];
            assert(pipe(sha1sum_input_pipe) == 0);
            assert(pipe(sha1sum_output_pipe) == 0);
            pid_t pid = fork();//pipe[0] is used to read
            if(pid == 0){
                //printf("child\n");
                assert(close(sha1sum_input_pipe[1]) == 0);
                dup2(sha1sum_input_pipe[0], 0);
                dup2(sha1sum_output_pipe[1], 1);
                assert(close(sha1sum_output_pipe[0]) == 0);
                char *myarg[10] = {"sha1sum"};
                execve("/usr/bin/sha1sum", myarg, environ);
                assert(0);
            }
            else{
                assert(close(sha1sum_input_pipe[0]) == 0);
                assert(close(sha1sum_output_pipe[1]) == 0);
                write(sha1sum_input_pipe[1], (void *)&fs[i], actual_file_size);
                assert(close(sha1sum_input_pipe[1]) == 0);
                wait(NULL);
                char sha1sum_output[MAX_STR] = "\0";
                read(sha1sum_output_pipe[0], sha1sum_output, 40);
                strcat(sha1sum_output, "  ");
                strcat(sha1sum_output, this_bmpname);
                printf("%s\n", sha1sum_output);
            }
        }
    }


    munmap((void *)fs,1 << 27);
    return 0;
}
