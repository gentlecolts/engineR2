#include "vobjReadV8l.h"
#include <cstdio>
using namespace std;

pos_t readFromV8l(char* data,pos_t size,pos_t strpos,vnode* node){
	uint32_t r=0,g=0,b=0;

	//printf("data:%p\tsize:%u\tpos:%u\tnode:%p\n",data,(unsigned int)(size),(unsigned int)(strpos),node);
	//printf("data val:%x\n",data[strpos]);

	if(data[strpos]!=0x00){
		//printf("has children\n\n");
		uint8_t n=0;
		node->initChildren(data[strpos]);
		strpos+=1;

		if(strpos>=size){
			throw;
		}

		for(int i=0;i<8;i++){
			if(((node->shape)>>i)&0x01){
				#if linkparent
				strpos=readFromV8l(data,size,strpos,&(node->nodes->next[i]));
				r+=((node->nodes->next[i].color)>>16)&0xff;
				g+=((node->nodes->next[i].color)>>8)&0xff;
				b+=(node->nodes->next[i].color)&0xff;
				#else
				strpos=readFromV8l(data,size,strpos,&(node->next[i]));
				r+=((node->next[i].color)>>16)&0xff;
				g+=((node->next[i].color)>>8)&0xff;
				b+=(node->next[i].color)&0xff;
				#endif
				++n;
			}
		}

		r/=n;
		g/=n;
		b/=n;
	}else{
		//printf("does not have children\n");
		node->shape=0x00;
		strpos+=1;

		//i dont care if strpos==size at the end of this part, it actually should do that
		if(2+strpos>=size){
			throw;
		}

		r=data[strpos]&0xff;
		strpos+=1;
		g=data[strpos]&0xff;
		strpos+=1;
		b=data[strpos]&0xff;
		strpos+=1;

		//printf("r:%x\ng:%x\nb:%x\ncolor:%p\n\n",r,g,b,(r<<16)|(g<<8)|b);
	}

	node->color=(r<<16)|(g<<8)|b;
	return strpos;
}
