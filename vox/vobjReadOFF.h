#ifndef VOBJREADOFF_H_INCLUDED
#define VOBJREADOFF_H_INCLUDED

#include "vobj.h"
typedef std::ifstream::pos_type pos_t;

void readFromOFF(char* data,pos_t size,pos_t strpos,vnode* head);

#endif // VOBJREADOFF_H_INCLUDED
