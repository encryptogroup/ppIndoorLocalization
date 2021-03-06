/*
 *   MIRACL compiler/hardware definitions - mirdef.h
 */
#if defined(ANDROID)
#define MR_GENERIC_MT
#define MIRACL 32
#define MR_LITTLE_ENDIAN
/* or possibly
#define MR_BIG_ENDIAN
*/

// generic multithreading

#define MR_IBITS 32
#define MR_LBITS 32
#define mr_utype int
#define mr_dltype long long
#define mr_unsign32 unsigned int
#define mr_unsign64 unsigned long long
#define MAXBASE ((mr_small)1<<(MIRACL-1))
#define MR_BITSINCHAR 8

#define MR_NOASM
#else
#define MR_GENERIC_MT
#define MR_LITTLE_ENDIAN
#define MIRACL 64
#define mr_utype  long long
#define mr_unsign32 unsigned int
#define mr_unsign64 unsigned long long
#define MR_IBITS 32
#define MR_LBITS 32
#define MAXBASE ((mr_small)1<<(MIRACL-1))
#define MR_BITSINCHAR 8
#endif
