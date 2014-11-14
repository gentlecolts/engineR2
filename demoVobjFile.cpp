#include "demos.h"
#include <iostream>
using namespace std;

void demoVobjFromFile(SDL_Surface* screen,camera& cam){
	vobj o(1,1,1,0,0,0);
	//o.readFromFile("data/obj.v8l");
	SDL_WM_SetCaption("loading file",NULL);
	//o.readFromFile("data/neptune_4Mtriangles_manifold/803_neptune_4Mtriangles_manifold.off");
	//o.readFromFile("data/WTFZOMFG/794_lagomaggiore.off");
	//o.readFromFile("data/Chinese_dragon/783_Chinese_dragon.off");
	//o.readFromFile("data/chair.off");
	//o.readFromFile("data/pleo/pleo.off");
	o.readFromFile("data/pleo.v8l");
	//o.readFromFile("data/abstr.off");
	//o.readFromFile("data/octa.off");
	//o.readFromFile("data/socket.off");
	//o.readFromFile("data/pleo.v8l");
	//o.readFromFile("data/zangoose.off");
	printf("after read\n");

	//o.writeToFile("data/zangoose.v8l");

	//o.xvec.xyz[1]=0.5;

	SDL_WM_SetCaption("writing",NULL);

	cout<<"blah"<<endl;

	#if linkparent && 0
	vnode* head=o.head,*node;
	printf("---node testing---\nhead: 0x%p\n",head);
	printf("headpntr: 0x%p\n",&(head->nodes->selfpntr));
	int diff;

	head=&(head->nodes->next[4]);
	for(int i=0;i<8;i++){
		node=&(head->nodes->next[i]);
		printf("node(%i) addr: 0x%p difference:0x%x",i,node,node-head);
		diff=sizeof(vnode)*(node->index)+sizeof(vnode*);
		//printf("\t calculated head (inline): 0x%p",*(vnode**)((int)node - diff));
		node=node->getParent();
		printf("\t calculated head: 0x%p\t off by: 0x%x\n",node,node-head);
	}
	#endif

	//sleep(100);
	//o.writeToFile("data/chinese_dragon.v8l");
	//o.writeToFile("data/pleo.v8l");

	loopDrawObj(screen,&o,cam);
}
