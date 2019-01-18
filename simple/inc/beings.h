#ifndef _BEINGS_H_
#define _BEINGS_H_


#include<iostream>
using namespace std;

namespace World
{
	class Beings
	{
	public:
		Beings(){};
		Beings(int age,const string &name):m_age(age), m_name(name)
		{
			cout<<"--->Constructor Beings..."<<endl;
		}
		virtual ~Beings()
		{
			cout<<"<---Destructor Beings..."<<endl;
		}

		virtual int eat() = 0;
		virtual void show() = 0;

protected:
		int m_age;
		string m_name;
	};

}

#endif

