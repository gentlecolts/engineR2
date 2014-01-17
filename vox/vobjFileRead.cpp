#include "vobj.h"
#include <cstdio>
#include "vobjReadV8l.h"
#include "vobjReadOFF.h"

#include <cmath>
#include <ctime>

#include <omp.h>
using namespace std;

typedef ifstream::pos_type pos_t;

void vobj::readFromFile(string filename){
	//TODO: decide whether to check for null or make head protected
	//TODO: i might not need the next two lines if i make a call to cleanTree
	head->die();

	//printf("head dead\n");

	ifstream file(filename.c_str(),ios::in|ios::binary|ios::ate);
	if(file.is_open()){
		pos_t size=file.tellg();

		//printf("size:%u %u\n",(unsigned int)(size),(unsigned int)(file.tellg()));

		char* data=new char[size];
		file.seekg(0,ios::beg);
		file.read(data,size);
		file.close();
		//printf("before readin\n");
		if(data[0]=='v' && data[1]=='8' && data[2]=='l' ){
			readFromV8l(data,size,3,head);
		}else if(
			(data[0]=='O' && data[1]=='F' && data[2]=='F')
			||
			(data[0]=='C' && data[1]=='O' && data[2]=='F' || data[3]=='F')
		){
			printf("loading from off\n");
			int tid;
			maxdepth=9;
			//#pragma omp single nowait
			//#pragma omp sections nowait
			{
				readFromOFF(data,size,3+(data[0]=='C'),head,maxdepth);
			}
			//#pragma omp barrier
		}
		//printf("after readin\n");
		delete[] data;
	}else{
		//printf("file not opened\n");
		throw;
	}
}
