#ifndef DEBUG_H
#define DEBUG_H

#define ASSERT(EXPECTED, CONDITION) \
	if (CONDITION != EXPECTED) \
	{ \
		print((const char*)__LINE__); \
		panic("__LINE__ HAS ISSUES!"); \
	} 

#define NASSERT(NEXPECTED, CONDITION) \
	if (CONDITION == NEXPECTED) \
	{ \
		print(__LINE__); \
		panic("ISSUES!"); \
	}

#endif
