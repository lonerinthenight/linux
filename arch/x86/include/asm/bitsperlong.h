#ifndef __ASM_X86_BITSPERLONG_H
#define __ASM_X86_BITSPERLONG_H

#ifdef __x86_64__
# define __BITS_PER_LONG 64		/* x86_64 系统字长(bits_per_long) = 64*/
#else
# define __BITS_PER_LONG 32		/*X86     系统字长(bits_per_long) = 32*/
#endif

#include <asm-generic/bitsperlong.h>

#endif /* __ASM_X86_BITSPERLONG_H */

