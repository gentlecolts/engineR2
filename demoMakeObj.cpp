#include "demos.h"
using namespace std;

/*
 shape
bit	zyx
0	+++
1	++-
2	+-+
3	+--
4	-++
5	-+-
6	--+
7	---
*/

void demoMakeVobj(SDL_Surface* screen,camera& cam){
	vobj o2(1,1,1,0,0,0);
	vnode *node0,*node1,*node2,*node3;

	node0=o2.head;
	node0->initChildren(0xff);

	node1=&(node0->nodes->next[0]);
	node1->initChildren(0xff);

	node2=&(node1->nodes->next[0]);
	node2->initChildren(0xfe);
	node2->nodes->next[0].color=0x00ff00;
	node2->nodes->next[1].color=0x00ff00;
	node2->nodes->next[2].color=0x00ff00;
	node2->nodes->next[3].color=0x00ff00;
	node2->nodes->next[4].color=0x00ff00;
	node2->nodes->next[5].color=0x00ff00;
	node2->nodes->next[6].color=0x00ff00;
	node2->nodes->next[7].color=0x00ff00;

	node2=&(node1->nodes->next[2]);
	node2->initChildren(0xfb);
	node2->nodes->next[0].color=0xff0000;
	node2->nodes->next[1].color=0x0000ff;
	node2->nodes->next[3].color=0xffff00;
	node2->nodes->next[4].color=0x00ff00;
	node2->nodes->next[5].color=0x00ffff;
	node2->nodes->next[6].color=0xff00ff;
	node2->nodes->next[7].color=0xffffff;

	node1=&(node0->nodes->next[1]);
	node1->initChildren(0xff);

	node2=&(node1->nodes->next[1]);
	node2->initChildren(0xfd);
	node2->nodes->next[0].color=0x00ff00;
	node2->nodes->next[1].color=0x00ff00;
	node2->nodes->next[2].color=0x00ff00;
	node2->nodes->next[3].color=0x00ff00;
	node2->nodes->next[4].color=0x00ff00;
	node2->nodes->next[5].color=0x00ff00;
	node2->nodes->next[6].color=0x00ff00;
	node2->nodes->next[7].color=0x00ff00;

	node2=&(node1->nodes->next[3]);
	node2->initChildren(0xf7);
	node2->nodes->next[0].color=0x007f00;
	node2->nodes->next[1].color=0x007f00;
	node2->nodes->next[2].color=0x007f00;
	node2->nodes->next[3].color=0x007f00;
	node2->nodes->next[4].color=0x007f00;
	node2->nodes->next[5].color=0x007f00;
	node2->nodes->next[6].color=0x007f00;
	node2->nodes->next[7].color=0x007f00;

	node1=&(node0->nodes->next[4]);
	node1->initChildren(0xff);

	node2=&(node1->nodes->next[4]);
	node2->initChildren(0xef);
	node2->nodes->next[0].color=0x00ff00;
	node2->nodes->next[1].color=0x00ff00;
	node2->nodes->next[2].color=0x00ff00;
	node2->nodes->next[3].color=0x00ff00;
	node2->nodes->next[4].color=0x00ff00;
	node2->nodes->next[5].color=0x00ff00;
	node2->nodes->next[6].color=0x00ff00;
	node2->nodes->next[7].color=0x00ff00;

	node2=&(node1->nodes->next[6]);
	node2->initChildren(0xbf);
	node2->nodes->next[0].color=0x007f00;
	node2->nodes->next[1].color=0x007f00;
	node2->nodes->next[2].color=0x007f00;
	node2->nodes->next[3].color=0x007f00;
	node2->nodes->next[4].color=0x007f00;
	node2->nodes->next[5].color=0x007f00;
	node2->nodes->next[6].color=0x007f00;
	node2->nodes->next[7].color=0x007f00;

	node1=&(node0->nodes->next[5]);
	node1->initChildren(0xff);

	node2=&(node1->nodes->next[5]);
	node2->initChildren(0xdf);
	node2->nodes->next[0].color=0x00ff00;
	node2->nodes->next[1].color=0x00ff00;
	node2->nodes->next[2].color=0x00ff00;
	node2->nodes->next[3].color=0x00ff00;
	node2->nodes->next[4].color=0x00ff00;
	node2->nodes->next[5].color=0x00ff00;
	node2->nodes->next[6].color=0x00ff00;
	node2->nodes->next[7].color=0x00ff00;

	node2=&(node1->nodes->next[7]);
	node2->initChildren(0x7f);
	node2->nodes->next[0].color=0x007f00;
	node2->nodes->next[1].color=0x007f00;
	node2->nodes->next[2].color=0x007f00;
	node2->nodes->next[3].color=0x007f00;
	node2->nodes->next[4].color=0x007f00;
	node2->nodes->next[5].color=0x007f00;
	node2->nodes->next[6].color=0x007f00;
	node2->nodes->next[7].color=0x007f00;

	node1=&(node0->nodes->next[2]);
	node1->initChildren(0xfa);

	node2=&(node1->nodes->next[1]);
	node2->initChildren(0xa0);
	node2=&(node1->nodes->next[3]);
	node2->initChildren(0xa0);
	node2=&(node1->nodes->next[4]);
	node2->initChildren(0xa0);
	node2=&(node1->nodes->next[6]);
	node2->initChildren(0xa0);

	node1=&(node0->nodes->next[3]);
	node1->initChildren(0xf5);

	node2=&(node1->nodes->next[0]);
	node2->initChildren(0x50);
	node2=&(node1->nodes->next[2]);
	node2->initChildren(0x50);
	node2=&(node1->nodes->next[5]);
	node2->initChildren(0x50);
	node2=&(node1->nodes->next[7]);
	node2->initChildren(0x50);

	node1=&(node0->nodes->next[6]);
	node1->initChildren(0xaf);

	node2=&(node1->nodes->next[0]);
	node2->initChildren(0x0a);
	node2=&(node1->nodes->next[2]);
	node2->initChildren(0x0a);
	node2=&(node1->nodes->next[5]);
	node2->initChildren(0x0a);
	node2=&(node1->nodes->next[7]);
	node2->initChildren(0x0a);

	node1=&(node0->nodes->next[7]);
	node1->initChildren(0x5f);

	node2=&(node1->nodes->next[1]);
	node2->initChildren(0x05);
	node2=&(node1->nodes->next[3]);
	node2->initChildren(0x05);
	node2=&(node1->nodes->next[4]);
	node2->initChildren(0x05);
	node2=&(node1->nodes->next[6]);
	node2->initChildren(0x05);

	o2.head->calcColors();

	printf("before write\n");
	//o2.writeToFile("data/obj.v8l");
	printf("after write\n");

	/*
	o2.zvec.x=0.2;
	o2.yvec.z=0.2;
	o2.xvec.y=0.2;
	//*/

	loopDrawObj(screen,&o2,cam);
}
