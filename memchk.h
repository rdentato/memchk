#ifndef MEMCHK_H
#define MEMCHK_H

#ifdef NDEBUG
#ifdef MEMCHECK
#undef MEMCHECK
#endif
#endif

#ifdef MEMCHK
  void *MC_calloc(size_t count, size_t size, char *file, int32_t line);
  void *MC_malloc(size_t size, char *file, int32_t line);
  void *MC_realloc(void *ptr, size_t size, char *file, int32_t line);
  void  MC_free(void *ptr, char *file, int32_t line);

  #define malloc(sz)   MC_malloc(sz,__FILE__, __LINE__)
  #define calloc(n,sz) MC_calloc(n,sz,__FILE__, __LINE__)
  #define realloc(p,sz) MC_realloc(p,sz,__FILE__, __LINE__)
  #define free(p)      MC_free(p,__FILE__, __LINE__)

  void mem_chk(void *ptr, char *file, int32_t line);
  #define memchk(p) mem_chk(p,__FILE__,__LINE__);
#else
  #define memchk(p) (void)0
#endif

#endif