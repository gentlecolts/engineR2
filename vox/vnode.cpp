#include "vnode.h"

uint32_t vnode::defcol=0xffffff;

vnode::vnode(uint8_t shp,uint32_t col):next(NULL),color(col),shape(shp){}

vnode::vnode(const vnode& node){
	shape=node.shape;
	if(node.next!=NULL){
		if(next==NULL){
			next=new vnode[8];
		}
		for(int i=0;i<8;i++){
			next[i]=node.next[i];
		}
	}
}
void vnode::operator =(const vnode& node){
	shape=node.shape;
	if(node.next!=NULL){
		if(next==NULL){
			next=new vnode[8];
		}
		for(int i=0;i<8;i++){
			next[i]=node.next[i];
		}
	}
}

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
		next=NULL;
	}
}

void vnode::initChildren(uint8_t node_shape){
	if(next==NULL){
		next=new vnode[8];
	}
	shape=node_shape;
}

/**TODO: consider making more intelegent delete functions
ex: set the shape flag to 0 and have a cleanup process/thread/function which actually removes the
*/
void vnode::deleteChildren(){
	die();
	shape=0x00;
}
void vnode::deleteChild(int i){
	if(next!=NULL && i>=0 && i<8){
		shape&=~(0x1<<i);
		if(shape==0x00){
			die();
		}else{
			next[i].die();
		}
	}
}

uint32_t vnode::calcColors(){
	if(next!=NULL && shape!=0x00){
		uint32_t r=0,g=0,b=0;
		//color comes from the field

		for(int i=0;i<8;i++){
			color=((shape>>i)&0x01)*next[i].calcColors();
			r+=(color>>16)&0xff;
			g+=(color>>8)&0xff;
			b+=color&0xff;
		}
		uint32_t n=(0x55&shape)+(0x55&(shape>>1));
		n=(0x33&n)+(0x33&(n>>2));
		n=(0x0f&n)+(0x0f&(n>>4));
		r/=n;
		g/=n;
		b/=n;

		color=(r<<16)|(g<<8)|b;
	}
	return color;
}
void vnode::cleanTree(){
	if(shape==0x00){
		die();
	}else if(next!=NULL){
		next[0].cleanTree();
		next[1].cleanTree();
		next[2].cleanTree();
		next[3].cleanTree();
		next[4].cleanTree();
		next[5].cleanTree();
		next[6].cleanTree();
		next[7].cleanTree();
	}
}
