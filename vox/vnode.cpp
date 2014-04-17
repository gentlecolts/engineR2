#include "vnode.h"

uint32_t vnode::defcol=0xffffff;

#if linkparent || 1
vnode::vnode(uint8_t shp,uint32_t col):nodes(0),color(col),shape(shp){}

nodelist::nodelist(vnode* node):selfpntr(node){
	for(int i=0;i<8;i++){
		next[i].index=i;
	}
}

vnode::vnode(const vnode& node){
	shape=node.shape;

	if(node.nodes){//copied node's next pointer is not null
		if(!nodes){//this node's next pointer is uninitialized
			nodes=new nodelist(this);
		}
		for(int i=0;i<8;i++){
			nodes->next[i]=node.nodes->next[i];
		}
	}
}
void vnode::operator =(const vnode& node){
	shape=node.shape;

	if(node.nodes){//copied node's next pointer is not null
		if(!nodes){//this node's next pointer is uninitialized
			nodes=new nodelist(this);
		}
		for(int i=0;i<8;i++){
			nodes->next[i]=node.nodes->next[i];
		}
	}
}

vnode::~vnode(){
	die();
}

void vnode::die(){
	if(nodes){
		nodes->next[0].die();
		nodes->next[1].die();
		nodes->next[2].die();
		nodes->next[3].die();
		nodes->next[4].die();
		nodes->next[5].die();
		nodes->next[6].die();
		nodes->next[7].die();

		//delete[] nodes->next;
		delete nodes;
		nodes=NULL;
	}
}

void vnode::initChildren(uint8_t node_shape){
	if(!nodes){
		nodes=new nodelist(this);
	}
	//shape=(shape&0x700)|node_shape;
	shape=node_shape;
}

vnode* vnode::getParent(){
	return *(vnode**)((int)(this) - sizeof(vnode)*(index)-sizeof(vnode*));
	//return &(this[-index-1])-sizeof(vnode*);
}

/**TODO: consider making more intelegent delete functions
ex: set shape to 0 and have a cleanup process/thread/function which actually removes the
*/
void vnode::deleteChildren(){
	die();
	shape=0x00;
}
void vnode::deleteChild(int i){
	if(nodes!=NULL && i>=0 && i<8){
		shape&=~(0x1<<i);
		if(shape==0x00){
			die();
		}else{
			nodes->next[i].die();
		}
	}
}

uint32_t vnode::calcColors(){
	if(nodes && shape!=0x00){
		uint32_t r=0,g=0,b=0;
		//color comes from the field

		for(int i=0;i<8;i++){
			color=((shape>>i)&0x01)*nodes->next[i].calcColors();
			r+=(color>>16)&0xff;
			g+=(color>>8)&0xff;
			b+=color&0xff;
		}
		uint32_t n=(0x55&shape)+(0x55&(shape>>1));
		n=(0x33&n)+(0x33&(n>>2));
		n=(0x0f&n)+(0x0f&(n>>4));
		n+=!n;//make sure to avoid division by zero
		r/=n;
		g/=n;
		b/=n;

		color=(r<<16)|(g<<8)|b;
	}
	return color;
	//return 0xff0000;
}
void vnode::cleanTree(){
	if(shape==0x00){
		die();
	}else if(nodes->next!=NULL){
		nodes->next[0].cleanTree();
		nodes->next[1].cleanTree();
		nodes->next[2].cleanTree();
		nodes->next[3].cleanTree();
		nodes->next[4].cleanTree();
		nodes->next[5].cleanTree();
		nodes->next[6].cleanTree();
		nodes->next[7].cleanTree();
	}
}
#else
vnode::vnode(uint8_t shp,uint32_t col):next(0),color(col),shape(shp){}

vnode::vnode(const vnode& node){
	shape=node.shape;

	if(node.next){//copied node's next pointer is not null
		if(!next){//this node's next pointer is uninitialized
			next=new vnode[8];
		}
		for(int i=0;i<8;i++){
			next[i]=node.next[i];
		}
	}
}
void vnode::operator =(const vnode& node){
	shape=node.shape;

	if(node.next){//copied node's next pointer is not null
		if(!next){//this node's next pointer is uninitialized
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
	if(!next){
		next=new vnode[8];
	}
	shape=node_shape;
}

/**TODO: consider making more intelegent delete functions
ex: set shape to 0 and have a cleanup process/thread/function which actually removes the
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
	if(next && shape!=0x00){
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
	//return 0xff0000;
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
#endif
