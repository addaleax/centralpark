#ifndef _W32COMPAT_H
#define _W32COMPAT_H

#include <assert.h>
#include <limits.h>

#ifdef _WIN32
#ifdef __GNUC__
#define ffs(x) ((x) ? __builtin_ctz((x)) + 1 : 0)
#else
static const unsigned char ffstbl[256] = {
	0, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
	5, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	6, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	5, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	7, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	5, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	6, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	5, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	8, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	5, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	6, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	5, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	7, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	5, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	6, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1,
	5, 1, 2, 1, 3, 1, 2, 1,	4, 1, 2, 1, 3, 1, 2, 1
};

static int my_ffs(int i_) {
	unsigned int i = i_;
	if (!i)
		return 0;
	
	int rv = 0;
	if (!(i & 0xffff)) {
		i >>= 16;
		rv += 16;
	}
	
	if (!(i & 0xff)) {
		i >>= 8;
		rv += 8;
	}
	
	assert(CHAR_BIT == 8);
	return rv + ffstbl[i & 0xff];
}
#define ffs my_ffs
#endif
#endif

#endif
