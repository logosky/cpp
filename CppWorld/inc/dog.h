#ifndef _DOG_H_
#define _DOG_H_


#include"beings.h"
namespace World
{
	class Dog:public Beings
	{
		public:
			Dog(int age = 0, const string &name ="", const string &type =""):Beings(age,name)
			{
				cout<<"--->Constructor Dog..."<<endl;
				m_type = type;
			}
			~Dog()
			{
				cout<<"<---Destructor Dog..."<<endl;
			}
			virtual int eat();
			virtual void show();
			Dog operator+(const Dog& dog);
			
		private:
			string m_type;
	};
}


#endif



