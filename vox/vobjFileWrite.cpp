#include "vobj.h"
#include <cstdio>
#include <iostream>
using namespace std;

void vobj::writeToFile(string filename){
	queue<char> q;
	q.push('v');//voxel
	q.push('8');//octree
	q.push('l');//leaf nodes have color
	writeout(&q,head);

	queue<char>::size_type size=q.size();
	char data[size];

	///TODO: having both the queue and the array is not memory efficient, especially as the file grows large, consider writing to the file here
	for(queue<char>::size_type i=0;i<size;i++){
		data[i]=q.front();
		q.pop();
	}

	ofstream file(filename.c_str(),ios::out|ios::binary);
	file.write(data,size);
	file.close();
}
void vobj::writeout(queue<char>* q,vnode* node){
	q->push(node->shape);
	if(node->shape==0 || !(node->next)){
		q->push(((node->color)>>16)&0xff);
		q->push(((node->color)>>8)&0xff);
		q->push((node->color)&0xff);
	}else{
		for(int i=0;i<8;i++){
			if(((node->shape)>>i)&0x01){
				writeout(q,&(node->next[i]));
			}
		}
	}
}