#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define BEGCHK 0xCA5ABA5E
#define ENDCHK 0x10CCADD1
#define CLRCHK 0xB5B0CC1A

//
//    +--------------+  <-- Actually allocated memory
//    |  0xCA5ABA5E  |   
//    +--------------+
//    |     size     |
//    +--------------+  <-- Returned pointer
//    |              |   
//   //              //
//    |              |
//    +--------------+
//    |  0x10CCADD1  |   <-- Marker to identify overflow
//    +--------------+
//

typedef struct memhdr_s {
    uint32_t head;
    size_t   size;
} *memhdr_t;

#define memhdr_size sizeof(struct memhdr_s)
#define fullsize(sz) ((sz) + memhdr_size + 4)

#define MC_msg(f,l,...) (fprintf(stderr,__VA_ARGS__), \
                         fprintf(stderr, " %s:%d\n",f,l))

void mem_chk(void *ptr, char *file, int32_t line)
{
  memhdr_t tmp;
  char *err = NULL;

  MC_msg(file,line,"MEMCHK: %p",ptr);

  if (ptr != NULL) {
    tmp = (memhdr_t)((char *)ptr - memhdr_size);
  
    if (tmp->head == CLRCHK) 
      err = "KO: RELEASED BLOCK";
    else if (tmp->head != BEGCHK)
      err = "KO: INVALID POINTER";
    else if (memcmp(ptr + (tmp->size), &((uint32_t){ENDCHK}), 4)) 
      err = "KO: BUFFER OVERRUN";
  }

  MC_msg(file,line," -- %s",err?err:"OK");
  if (err) abort();
}

static void *set_check(memhdr_t ptr, size_t size, size_t oldsize,char *file, int32_t line)
{
  void *retptr = NULL;
  if (ptr && (size > 0)) {
    ptr->head = BEGCHK;
    ptr->size = size;
    retptr = (char *)ptr + memhdr_size;
    memcpy(retptr + size, &((uint32_t){ENDCHK}), 4);
  }
  MC_msg(file,line," -> %p +%lu -%lu",retptr,size,oldsize);
  return retptr;
}

void *MC_malloc(size_t size, char *file, int32_t line)
{
  MC_msg(file, line, "malloc(%lu)", size);
  return set_check(malloc(fullsize(size)), size, 0,file, line);
}

void *MC_calloc(size_t count, size_t size, char *file, int32_t line)
{
  MC_msg(file, line, "calloc(%lu,%lu)", count, size);
  return set_check(calloc(1, fullsize(count * size)), size, 0,file, line);
}

void *MC_realloc(void *ptr, size_t size, char *file, int32_t line)
{
  memhdr_t tmp = ptr;
  size_t oldsize = 0;

  MC_msg(file, line, "realloc(%p,%lu)", ptr, size);
  if (ptr != NULL) {
    mem_chk(ptr,file,line);
    tmp = (memhdr_t)((char *)ptr - memhdr_size);
    oldsize = tmp->size;
  }
  tmp = realloc(tmp, size > 0? fullsize(size): 0);
  return set_check(tmp,size,oldsize,file, line);
}

void MC_free(void *ptr, char *file, int32_t line) {
  size_t oldsize = 0;
  MC_msg(file, line, "free(%p)", ptr);
  memhdr_t tmp = NULL;
  if (ptr != NULL) {
    mem_chk(ptr,file,line);
    tmp = (memhdr_t)((char *)ptr - memhdr_size);
    tmp->head = CLRCHK;
    oldsize = tmp->size;
    tmp->size = 0;
  }
  MC_msg(file,line," -> %p +0 -%lu",NULL,oldsize);
  free(tmp);
}
