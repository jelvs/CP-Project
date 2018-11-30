#ifndef __UNIT_H
#define __UNIT_H

typedef void (*TESTFUNCTION)(void *, size_t, size_t);

extern TESTFUNCTION testAllFunction[];

extern char *testAllNames[];

extern TESTFUNCTION testParallelFunction[];

extern char *testParallelNames[];

extern int nTestAllFunction;

extern int nTestParallelFunction;

#endif