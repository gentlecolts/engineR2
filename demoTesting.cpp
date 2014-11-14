#include "demos.h"
#include <iostream>
#include <cmath>
using namespace std;

void demoTesting(SDL_Surface* screen,camera& cam){
	{
		cout<<sizeof(double)<<endl;
		cout<<sizeof(long long)<<endl;
		cout<<sizeof(void*)<<endl;
	}

	{
		long long l=0x3ff0000000000000l;
		double d=*(double*)&l;
		long* p=(long*)&d;
		cout<<d<<endl;
		printf("%p %p",*p,*(p+4));
	}

	{
		cout<<pow2(-13)<<endl;cout<<pow2(-12)<<endl;cout<<pow2(-11)<<endl;
		cout<<pow2(-10)<<endl;cout<<pow2(-9)<<endl;cout<<pow2(-8)<<endl;
		cout<<pow2(-7)<<endl;cout<<pow2(-6)<<endl;cout<<pow2(-5)<<endl;
		cout<<pow2(-4)<<endl;cout<<pow2(-3)<<endl;cout<<pow2(-2)<<endl;
		cout<<pow2(-1)<<endl;cout<<pow2(0)<<endl;cout<<pow2(1)<<endl;
		cout<<pow2(2)<<endl;cout<<pow2(3)<<endl;cout<<pow2(4)<<endl;
		cout<<pow2(5)<<endl;cout<<pow2(6)<<endl;cout<<pow2(7)<<endl;
		cout<<pow2(8)<<endl;cout<<pow2(9)<<endl;cout<<pow2(10)<<endl;
		cout<<pow2(11)<<endl;cout<<pow2(12)<<endl;cout<<pow2(13)<<endl;
	}

	{
		double
			a=1,
			b=INFINITY,
			c=(a<b)*a+(!(a<b))*b,
			d=(a>b)*a+(!(a>b))*b;
		cout<<max(a,b)<<endl;
		cout<<min(a,b)<<endl;
		cout<<d<<endl;
		cout<<c<<endl;
		cout<<1.0/c<<endl;
		cout<<(c>1)<<endl;
		cout<<(c<1)<<endl;
		cout<<(c==1)<<endl;
	}

	{
		uint32_t subtest=0;
		--subtest;
		cout<<"unsigned test: "<<subtest<<endl;
		cout<<"unsigned test2: "<<(subtest+1==0)<<endl;
		cout<<"unsigned test3: "<<(subtest==-1)<<endl;

		cout<<"bitshift tesst: "<<(1>>-1)<<endl;

		char tmpstr[65];
		long long tmpllong=0x7fffffffffffffffll;
		//long long tmpllong=(~0ull)>>1;
		cout<<"~0ll: "<<tmpllong<<endl;
		//cout<<"~0ll: "<<itoa(tmpllong,tmpstr,10)<<endl;
	}
}
