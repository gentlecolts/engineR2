#include "vobj.h"
#include "../msc/functions.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

using namespace std;

#if 0
template<typename T>
T min(T a,T b){
	/*
	if(a<b){return a;}
	return b;
	/*/
	return (a<b)*a+(!(a<b))*b;
	//*/
}

template<typename T>
T max(T a,T b){
	/*
	if(a>b){return a;}
	return b;
	/*/
	return (a>b)*a+(!(a>b))*b;
	//*/
}
#endif

vobj::vobj(double wid,double hght,double dep,double x0,double y0,double z0):pos(x0,y0,z0),xvec(wid,0,0),yvec(0,hght,0),zvec(0,0,dep){
	head=new vnode();
}

//this should copy the tree of the inputted object
vobj::vobj(vobjref o){
}

vobj::~vobj(){
	delete head;
}

//this should behave like the copy constructor
vobjref vobj::operator =(vobjref o){
}

void vobj::updateVals(){
	w=(abs(xvec.x)+abs(yvec.x)+abs(zvec.x));
	h=(abs(xvec.y)+abs(yvec.y)+abs(zvec.y));
	d=(abs(xvec.z)+abs(yvec.z)+abs(zvec.z));
	varr={xvec,-xvec,yvec,-yvec,zvec,-zvec};
}

bool vobj::intersects(vecref vec,vecref origin,uint32_t* color,double* closeT) const{
	return chkIntersect(head,pos,vec,origin.x,origin.y,origin.z,color,1,closeT);
}
bool vobj::intersects(vecref vec,double x0,double y0,double z0,uint32_t* color,double* closeT) const{
	return chkIntersect(head,pos,vec,x0,y0,z0,color,1,closeT);
}

typedef long long llong;

bool vobj::chkIntersect(vnode* node,vec3d p,vecref v,double vx0,double vy0,double vz0,uint32_t* color,int scale,double* closeT) const{
	//if(scale>1){printf("%i\n",scale);}
	const double denom=pow2(-scale);//1.0/(llong(1)<<llong(scale));// 1/(2^scale)
	/*
	const double
		x1=( (1-2*(v.x>=0)) *w*denom-(vx0-p.x))/v.x,
		x2=( (2*(v.x>=0)-1) *w*denom-(vx0-p.x))/v.x,
		y1=( (1-2*(v.y>=0)) *h*denom-(vy0-p.y))/v.y,
		y2=( (2*(v.y>=0)-1) *h*denom-(vy0-p.y))/v.y,
		z1=( (1-2*(v.z>=0)) *d*denom-(vz0-p.z))/v.z,
		z2=( (2*(v.z>=0)-1) *d*denom-(vz0-p.z))/v.z;

	double
		tlow=max(max(x1,y1),z1),
		thigh=min(min(x2,y2),z2);
	/*/
	#define x1 ( (1-2*(v.x>=0)) *w*denom-(vx0-p.x))/v.x
	#define x2 ( (2*(v.x>=0)-1) *w*denom-(vx0-p.x))/v.x
	#define y1 ( (1-2*(v.y>=0)) *h*denom-(vy0-p.y))/v.y
	#define y2 ( (2*(v.y>=0)-1) *h*denom-(vy0-p.y))/v.y
	#define z1 ( (1-2*(v.z>=0)) *d*denom-(vz0-p.z))/v.z
	#define z2 ( (2*(v.z>=0)-1) *d*denom-(vz0-p.z))/v.z

	double
		tlow=max(max(x1,y1),z1),
		thigh=min(min(x2,y2),z2);

	#undef x1
	#undef x2
	#undef y1
	#undef y2
	#undef z1
	#undef z2
	//*/

	vec3d vec;

	//if(tlow<=thigh){printf("%f %f\t",tlow,thigh);}

	if(tlow>thigh || thigh<0){//the ray does not intersect the current node or the cube is completely behind the ray
		return false;
	}else if(node->next==NULL || node->shape==0x00){///TODO: or tlow>cutoff_distance //ray intersects and this is a leaf node
		*color=node->color;
		*closeT=max(tlow,0.0);
		return true;
	}

	//it intersects the current node, but the node is not a leaf node
	bool b=false;
	double tmp;
	uint32_t closeCol,tmpcol;

	tlow=INFINITY;

	++scale;

	/*
	can i use tlow to figure out which node to go to next?
	could i use the same dot product method as i use with the camera?
	do the signs of those values actually tell me which quadrant to look in?
	i dont think that works because tlow relies on the axis aligned cube, which is too different form
	it might still work though
	it also would work if i were to figure out ray-parallelepiped intersection
	*/

	#if 1

	//i think this one is faster, need more testing to determine if it truely is or not

	uint32_t n=0;
	uint32_t nodes=0x00000000;

	#define blarg(i) \
	nodes|=(i*(((node->shape)>>i)&0x1))<<n;\
	n+=(((node->shape)>>i)&0x1)*3;

	blarg(0);
	blarg(1);
	blarg(2);
	blarg(3);
	blarg(4);
	blarg(5);
	blarg(6);
	blarg(7);

	//printf("%p\t%p\n",node->shape,nodes);

	#undef blarg

	//count the number of visible children nodes
	/*
	01010101=0x55
	00110011=0x33
	00001111=0x0f
	*/
	//*
	n=((node->shape)&0x55)+(((node->shape)>>1)&0x55);
	n=(n&0x33)+((n>>2)&0x33);
	n=(n&0x0f)+((n>>4)&0x0f);
	//*/

	int i;

	if(n>0){
		do{
			i=nodes&0x7;
			//if(chkIntersect(&(node->next[i]),p+denom/2*((1-((i<<1)&0x2))*xvec+(1-(i&0x2))*yvec+(1-((i>>1)&0x2))*zvec),v,vx0,vy0,vz0,&tmpcol,scale,&tmp)){
			vec=varr[i&0x1];
			vec+=varr[2+((i>>1)&0x1)];
			vec+=varr[4+((i>>2)&0x1)];
			vec*=denom/2;
			vec+=p;
			if(chkIntersect(&(node->next[i]),vec,v,vx0,vy0,vz0,&tmpcol,scale,&tmp)){
				b=true;

				closeCol=(tmp<tlow)*tmpcol+(!(tmp<tlow))*closeCol;
				tlow=min(tlow,tmp);
			}
			nodes>>=3;
		}while(nodes!=0);
	}

	#else
	for(int i=0;i<8;i++){
		/*
		x=1-2*((i>>0)&0x1)=1-((i<<1)&0x2)
		y=1-2*((i>>1)&0x1)=1-(i&0x2)
		z=1-2*((i>>2)&0x1)=1-((i>>1)&0x2)
		*/
		//if the current child is supposed to exist
		if((node->shape>>i)&0x1){
			//if(chkIntersect(&(node->next[i]),p+denom/2*((1-((i<<1)&0x2))*xvec+(1-(i&0x2))*yvec+(1-((i>>1)&0x2))*zvec),v,vx0,vy0,vz0,&tmpcol,scale,&tmp)){
			vec=varr[i&0x1];
			vec+=varr[2+((i>>1)&0x1)];
			vec+=varr[4+((i>>2)&0x1)];
			vec*=denom/2;
			vec+=p;
			if(chkIntersect(&(node->next[i]),vec,v,vx0,vy0,vz0,&tmpcol,scale,&tmp)){
				b=true;

				closeCol=(tmp<tlow)*tmpcol+(!(tmp<tlow))*closeCol;
				tlow=min(tlow,tmp);
			}
		}
	}
	#endif

	*color=closeCol;
	*closeT=tlow;
	return b;
}

void vobj::readFromFile(string filename){
	//TODO: decide whether to check for null or make head protected
	//TODO: i might not need the next two lines if i make a call to cleanTree
	head->die();

	printf("head dead\n");

	ifstream file(filename.c_str(),ios::in|ios::binary|ios::ate);
	if(file.is_open()){
		ifstream::pos_type size=file.tellg();

		printf("size:%u %u\n",(unsigned int)(size),(unsigned int)(file.tellg()));

		char* data=new char[size];
		file.seekg(0,ios::beg);
		file.read(data,size);
		file.close();
		printf("before readin\n");
		readin(data,size,0,head);
		printf("after readin\n");
		delete[] data;
	}else{
		printf("file not opened\n");
		throw;
	}
}

ifstream::pos_type vobj::readin(char* data,ifstream::pos_type size,ifstream::pos_type strpos,vnode* node){
	uint32_t r=0,g=0,b=0;

	printf("data:%p\tsize:%u\tpos:%u\tnode:%p\n",data,(unsigned int)(size),(unsigned int)(strpos),node);
	printf("data val:%x\n",data[strpos]);

	if(data[strpos]!=0x00){
		printf("has children\n\n");
		uint8_t n=0;
		node->initChildren(data[strpos]);
		strpos+=1;

		if(strpos>=size){
			throw;
		}

		for(int i=0;i<8;i++){
			if(((node->shape)>>i)&0x01){
				strpos=readin(data,size,strpos,&(node->next[i]));
				r+=((node->next[i].color)>>16)&0xff;
				g+=((node->next[i].color)>>8)&0xff;
				b+=(node->next[i].color)&0xff;
				++n;
			}
		}

		r/=n;
		g/=n;
		b/=n;
	}else{
		printf("does not have children\n");
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

		printf("r:%x\ng:%x\nb:%x\ncolor:%p\n\n",r,g,b,(r<<16)|(g<<8)|b);
	}

	node->color=(r<<16)|(g<<8)|b;
	return strpos;
}

void vobj::writeToFile(string filename){
	queue<char> q;
	writeout(&q,head);

	queue<char>::size_type size=q.size();
	char data[size];

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
	if(node->shape==0 || node->next==NULL){
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
