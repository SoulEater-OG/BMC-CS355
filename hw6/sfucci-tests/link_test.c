#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "structs.h"

int main(int argc, char**argv){
  if(argc !=2){
    printf("Usage: ./linktest filename\n");
    exit(EXIT_FAILURE);
  }

  FILE * defrag_file = fopen(argv[1],"r");

  void * data = malloc(512);
  fseek(defrag_file, 512, SEEK_SET);
  fread(data, 1, 512, defrag_file);
  rewind(defrag_file);

  struct superblock * sblock = (struct superblock *) data;
  int blocksize = sblock->size;
  int block_region_start = 1024 + (blocksize * sblock ->data_offset);

  int free_list_start = sblock->free_block;
  int total_blocks = sblock->swap_offset - sblock ->data_offset;

  printf("Disk should have %d blocks\n", total_blocks);
  int counted_free_blocks = 0;
  int expected_free_blocks = 0;
  if(free_list_start == -1){
    expected_free_blocks = 0;
  }else{
    expected_free_blocks = total_blocks - free_list_start;
  }

  if(free_list_start != -1){
    int current_free_block = free_list_start;
    for(int i = 0; i < expected_free_blocks -1; i++){
      fseek(defrag_file, block_region_start + (blocksize * current_free_block), SEEK_SET);
      int * addr= malloc(4);
      fread(addr, 1,4,defrag_file);
      printf("free block #%d points to next free block %d\n", current_free_block, *addr);
      assert(*addr == (current_free_block +1));
      current_free_block++;
      counted_free_blocks++;
    }
    //assert that data at current_free block's first 4 bytes is -1
    fseek(defrag_file, block_region_start + (blocksize * current_free_block), SEEK_SET);
    int* addr= malloc(4);
    fread(addr, 1,4,defrag_file);
    printf("free block #%d points to next free block %d\n", current_free_block, *addr);
    assert(*addr == -1);
    counted_free_blocks++;
  }else{
    assert(sblock ->free_block == -1);
  }
  assert(counted_free_blocks == expected_free_blocks);
  
  free(data);
  return 0;
}
