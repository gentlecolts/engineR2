#include "vnode.h"

uint32_t vnode::defcol=0xffffff;

vnode::vnode(uint32_t col):next(NULL),color(col){}
vnode::~vnode(){
	die();
}

void vnode::die(){
	if(next!=NULL){
		next[0].die();
		next[1].die();
		next[2].die();
		next[3].die();
		next[4].die();
		next[5].die();
		next[6].die();
		next[7].die();

		delete[] next;
	}
}

void vnode::initChildren(uint8_t node_shape){
	if(next==NULL){
		next=new vnode[8];
		shape=node_shape;
	}
}

void vnode::deleteChildren(){
}
void vnode::deleteChild(int i){
}
