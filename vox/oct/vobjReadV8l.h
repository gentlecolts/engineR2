#ifndef VOBJREADV8L_H_INCLUDED
#define VOBJREADV8L_H_INCLUDED

#include "../vobj.h"
typedef std::ifstream::pos_type pos_t;

pos_t readFromV8l(char* data,pos_t size,pos_t strpos,vnode* node);

#endif // VOBJREADV8L_H_INCLUDED
