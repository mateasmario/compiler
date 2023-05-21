#ifndef _VMDEFINITIONS_H_
#define _VMDEFINITIONS_H_

#define STACK_SIZE (32*1024)
char stack[STACK_SIZE];
char* SP; // Stack Pointer
char* stackAfter; // first byte after stack; used for stack limit tests

#define GLOBAL_SIZE (32*1024)
char globals[GLOBAL_SIZE];
int nGlobals;

#endif