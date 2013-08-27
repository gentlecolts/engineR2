#include "vobjReadOff.h"
#include "polygon.h"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cmath>
#include <omp.h>

//includes and trim come from http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
using namespace std;

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

pos_t getNextOffLine(char* data,pos_t strpos,pos_t size,string* line){
	#define chartest ((data[strpos]>='0' && data[strpos]<='9') || data[strpos]=='.' || data[strpos]=='-')

	stringstream linestream;
	bool b;

	while(!chartest){
		strpos+=1;
	}
	//line="";

	while(strpos<size && data[strpos]!='\r' && data[strpos]!='\n'){
		//if((int)nums.find(data[strpos])>=0){
		b=chartest;
		//c=data[strpos];
		linestream<<char((b*data[strpos])^((!b)*' '));
		strpos+=1;
	}

	*line=linestream.str();
	if((*line)[line->size()-1]!=' '){
		*line+=' ';
	}

	return strpos;
}

const string nums("0123456789-.");
void readFromOFF(char* data,pos_t size,pos_t strpos,vnode* head){
	/*
	OFF numVertices numFaces numEdges
	x y z
	x y z
	... numVertices like above
	NVertices v1 v2 v3 ... vN
	MVertices v1 v2 v3 ... vM
	... numFaces like above
	*/
	#define maxdepth 10
	double minxyz[3],maxxyz[3],avgxyz[3];

	string line;

	bool b;
	long numvert,numtri;//long long

	strpos=getNextOffLine(data,strpos,size,&line);

	int start=0;
	int pos=line.find(' ');
	numvert=atoi(line.substr(start,pos-start).c_str());
	vert* verts=new vert[numvert];
	printf("num verts:%i\n",numvert);

	start=pos+1;
	pos=line.find(' ',start);
	numtri=atoi(line.substr(start,pos-start).c_str());
	printf("num face:%i\n",numtri);

	uint32_t rgba[4];
	int colnum;

	//num edges can be ignored

	//load all of the verticies
	minxyz[0]=INFINITY;
	minxyz[1]=INFINITY;
	minxyz[2]=INFINITY;
	maxxyz[0]=-INFINITY;
	maxxyz[1]=-INFINITY;
	maxxyz[2]=-INFINITY;
	for(long i=0;i<numvert;i++){
		strpos=getNextOffLine(data,strpos,size,&line);

		start=0;
		pos=line.find(' ');
		verts[i].x=atof(line.substr(start,pos-start).c_str());

		start=pos+1;
		pos=line.find(' ',start);
		verts[i].y=atof(line.substr(start,pos-start).c_str());

		start=pos+1;
		pos=line.find(' ',start);
		verts[i].z=atof(line.substr(start,pos-start).c_str());

		colnum=0;
		rgba[3]=0xff;//make the alpha ff if alpha is unused
		while(pos+1<line.size() && line.find(' ',pos+1)>0 && colnum<4){
			start=pos+1;
			pos=line.find(' ',start);

			rgba[colnum]=atoi(line.substr(start,pos-start).c_str())&0xff;
			++colnum;
		}

		verts[i].hascolor=colnum>2;
		verts[i].color=(rgba[3]<<24)|(rgba[0]<<16)|(rgba[1]<<8)|rgba[2];

		//if(verts[i].hascolor){printf("vert:%i\tcolor: %x %x %x %x\n",i,rgba[3],rgba[0],rgba[1],rgba[2]);}

		minxyz[0]=min(minxyz[0],verts[i].x);
		minxyz[1]=min(minxyz[1],verts[i].y);
		minxyz[2]=min(minxyz[2],verts[i].z);

		maxxyz[0]=max(maxxyz[0],verts[i].x);
		maxxyz[1]=max(maxxyz[1],verts[i].y);
		maxxyz[2]=max(maxxyz[2],verts[i].z);
	}

	//load all of the faces
	poly tri;
	const double gridsize=max(max(
					maxxyz[0]-minxyz[0],
					maxxyz[1]-minxyz[1]),
					maxxyz[2]-minxyz[2])/2;
	avgxyz[0]=(minxyz[0]+maxxyz[0])/2;
	avgxyz[1]=(minxyz[1]+maxxyz[1])/2;
	avgxyz[2]=(minxyz[2]+maxxyz[2])/2;

	const double g=1/gridsize;
	#pragma omp parallel for
	for(long i=0;i<numvert;i++){
		verts[i].x=(verts[i].x-avgxyz[0])*g;
		verts[i].y=(verts[i].y-avgxyz[1])*g;
		verts[i].z=(verts[i].z-avgxyz[2])*g;
	}

	vec3d v;
	for(long i=0;i<numtri;i++){
		strpos=getNextOffLine(data,strpos,size,&line);

		start=0;
		pos=line.find(' ');
		tri.numvert=atoi(line.substr(start,pos-start).c_str());
		tri.verts=new vert*[tri.numvert];

		//if(tmp!=3){throw;}
		for(int j=0;j<tri.numvert;j++){
			start=pos+1;
			pos=line.find(' ',start);
			tri.verts[j]=&(verts[atoi(line.substr(start,pos-start).c_str())]);
		}

		colnum=0;
		rgba[3]=0xff;//make the alpha ff if alpha is unused
		while(pos+1<line.size() && line.find(' ',pos+1)>0 && colnum<4){
			start=pos+1;
			pos=line.find(' ',start);

			rgba[colnum]=atoi(line.substr(start,pos-start).c_str())&0xff;
			++colnum;
		}

		tri.hascolor=colnum>2;
		tri.color=(rgba[3]<<24)|(rgba[0]<<16)|(rgba[1]<<8)|rgba[2];


		//find the triangle's spot in the grid
		tri.placePoly(head,v,1,maxdepth,0.5);
		delete[] tri.verts;
	}

	delete[] verts;
}
