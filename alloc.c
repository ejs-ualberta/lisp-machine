#include "config.h"

typedef struct used_mem_datastructure{
  word mem_sz;
  word mem[];
} umds;

typedef struct free_mem_datastructure{
  word prev;
  word next;
  word mem_sz;
} fmds;

typedef struct heap_datastructure{
  word heap_end;
  word prev;
  word next;
  word fmds_ptr;
} hds;

const word umds_sz = sizeof(umds)/sizeof(word);
const word fmds_sz = sizeof(fmds)/sizeof(word);
const word hds_sz = sizeof(hds)/sizeof(word);
const word min_alloc_sz = fmds_sz;


word * init_heap(word * heap_start, word heap_sz){
  if ((word)heap_start % sizeof(word)){return 0;}
  if (heap_sz < min_alloc_sz + hds_sz){return 0;}

  hds * heap = (hds*)heap_start;
  heap->heap_end = (word)(heap_start + heap_sz);
  heap->prev = 0;
  heap->next = 0;
  heap->fmds_ptr = (word)(heap_start + hds_sz);

  fmds * mem = (fmds*)(heap_start + hds_sz);
  mem->prev = 0;
  mem->next = 0;
  mem->mem_sz = heap_sz - hds_sz;

  return heap_start;
}


word * alloc(word * heap, word mem_sz){
  mem_sz += umds_sz;
  if (mem_sz < min_alloc_sz){mem_sz = min_alloc_sz;}

  hds * h_info = (hds*)heap;
  fmds * mem_ptr = (fmds*)h_info->fmds_ptr;
  while (mem_ptr && mem_ptr->mem_sz < mem_sz){
    mem_ptr = (fmds*)(mem_ptr->next);
  }
  if (!mem_ptr){return 0;}

  /* word offset = 0; */
  if (mem_ptr->mem_sz < mem_sz + min_alloc_sz){
    mem_sz = mem_ptr->mem_sz;

    fmds * prev = (fmds*)mem_ptr->prev;
    if (prev){
      prev->next = mem_ptr->next;
    }else{
      h_info->fmds_ptr = mem_ptr->next;
    }
    
    fmds * next = (fmds*)mem_ptr->next;
    if (next){
      next->prev = mem_ptr->prev;
    }
  }else{
    /* offset = mem_ptr->mem_sz - mem_sz; */
    /* mem_ptr->mem_sz = offset; */
    word os = mem_ptr->mem_sz - mem_sz;
    fmds * new_ptr = (fmds*)((word*)mem_ptr + mem_sz);
    *new_ptr = *mem_ptr;
    new_ptr->mem_sz = os;
    fmds * pv = (fmds*)new_ptr->prev;
    if (pv){
      pv->next = (word)new_ptr;
    }else{h_info->fmds_ptr = (word)new_ptr;}
  }

  umds * mem = (umds*)(mem_ptr);  /* + offset); */
  mem->mem_sz = mem_sz;
  return (word*)&(mem->mem);
}


void free(word * heap, word * addr){
  hds * h_info = (hds*)heap;
  if (addr < (word*)h_info + hds_sz || addr >= (word*)h_info + h_info->heap_end){
    return;
  }
  fmds * mem_ptr = (fmds*)h_info->fmds_ptr;
  fmds * freed_mem = (fmds*)(addr - umds_sz);
  umds * mem_obj = (umds*)(addr - umds_sz);
  word fm_sz = mem_obj->mem_sz;

  if (!mem_ptr){
    h_info->fmds_ptr = (word)freed_mem;
    freed_mem->prev = 0;
    freed_mem->next = 0;
    freed_mem->mem_sz = fm_sz;
    return;
  }

  fmds * next = mem_ptr;
  mem_ptr = 0;
  while (next && next < freed_mem){
    mem_ptr = next;
    next = (fmds*)mem_ptr->next;
  }

  freed_mem->next = 0;
  freed_mem->prev = 0;
  freed_mem->mem_sz = fm_sz;
  if (next){
    if ((word*)freed_mem + fm_sz == (word*)next){
      fmds * next_next = (fmds*)next->next;
      freed_mem->mem_sz += next->mem_sz;
      freed_mem->next = (word)next_next;
      if (next_next){
	next_next->prev = (word)freed_mem;
      }
    }else{
      freed_mem->next = (word)next;
      next->prev = (word)freed_mem;
    }
  }

  if (mem_ptr){
    if ((word*)mem_ptr + mem_ptr->mem_sz == (word*)freed_mem){
      fmds * next_next = (fmds*)freed_mem->next;
      mem_ptr->mem_sz += freed_mem->mem_sz;
      mem_ptr->next = (word)next_next;
      if (next_next){
	next_next->prev = (word)mem_ptr;
      }
    }else{
      freed_mem->prev = (word)mem_ptr;
      mem_ptr->next = (word)freed_mem;
    }
  }else{
    h_info->fmds_ptr = (word)freed_mem;
  }
}


//TODO: Make this better.
word * realloc(word * heap, word * addr, word mem_sz){
  word * new_addr = alloc(heap, mem_sz);
  if (!new_addr){return 0;}
  umds * old_obj = (umds*)(addr - umds_sz);
  for (word i = 0; i < old_obj->mem_sz; ++i){
    new_addr[i] = addr[i];
  }
  free(heap, addr);
  return new_addr;
}


word get_mem_sz(word * addr){
  umds * used_mem = (umds*)(addr - umds_sz);
  return used_mem->mem_sz - umds_sz;
}
