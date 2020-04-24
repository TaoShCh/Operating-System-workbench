#include <common.h>
#include <klib.h>
#include <lock.h>
static uintptr_t pm_start, pm_end;

#define BL_FREE 1
#define KB 1024
typedef struct _Block{
    struct _Block *next;
    struct _Block *last;
    char free;
    size_t size;
}Block_head;
Block_head *list_head;
Block_head *list_tail;

static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
  list_head = NULL;
  list_tail = NULL;
}

LOCKDEF(alloc);

void *first_time_alloc(size_t size){
    Block_head *block = (Block_head *)pm_start;
    block->next = NULL;
    block->last = NULL;
    block->size = size;
    block->free = 0;
    list_head = block;
    list_tail = block;
    return (void*)pm_start;
}

void *alloc_new_block(size_t size){
    Block_head *block = (Block_head *)((uintptr_t)list_tail + sizeof(Block_head) + list_tail->size);
    block->last = list_tail;
    block->next = NULL;
    block->size = size;
    block->free = 0;
    list_tail->next = block;
    list_tail = block;
    if((uintptr_t)block + sizeof(Block_head) + size >= pm_end){
        printf("ERROR: insufficiet memory space\n!");
        return NULL;
    }
    return block;
}

void block_merge(Block_head *block){
    if(block->next == NULL || block == NULL) return;
    if(block->next->free == BL_FREE && block->free == BL_FREE){
        block->size += block->next->size + sizeof(Block_head);
        void *tmp = block->next;
        block->next = block->next->next;
        memset(tmp, 0, sizeof(Block_head));
        if(list_tail == tmp){
            list_tail = block;
        }
        else{
            block->next->next->last = block;
        }
    }
}

void block_divide(Block_head *block, size_t size){
    if(block->size - size >= 4 * 1024){
        Block_head* new_block = (Block_head *)((uintptr_t)block + sizeof(Block_head) + size);
        new_block->last = block;
        new_block->next = block->next;
        new_block->free = BL_FREE;
        new_block->size = block->size - size - sizeof(Block_head);
        block->next->last = new_block;
        block->next = new_block;
        block->size = size;
        if(list_tail == block){
            list_tail = new_block;
        }
    }
}

static void *kalloc(size_t size) {
    alloc_lock();
    int find_flag = 0;
    Block_head *alloc_block = list_head;
    if(alloc_block == NULL){
        alloc_block = first_time_alloc(size);
    }
    else{
        while(alloc_block != NULL){
            if(alloc_block->free == BL_FREE && alloc_block->size >= size){
                block_divide(alloc_block, size);
                alloc_block->free = 0;
                find_flag = 1;
                break;
            }
            alloc_block = alloc_block->next;
        }
        if(!find_flag){
            alloc_block = alloc_new_block(size);
        }
    }
    void *ans = (void *)((uintptr_t)alloc_block + sizeof(Block_head));
    alloc_unlock();
    return ans;
}

static void kfree(void *ptr) {
    alloc_lock();
    Block_head *block = (Block_head *)((uintptr_t)ptr - sizeof(Block_head));
    block->free = BL_FREE;
    memset(ptr, 0, block->size);
    block_merge(block);
    block_merge(block->last);
    alloc_unlock();
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
