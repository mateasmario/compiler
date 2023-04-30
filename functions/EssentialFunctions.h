#ifndef _ESSENTIALFUNCTIONS_H_
#define _ESSENTIALFUNCTIONS_H_

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

#endif