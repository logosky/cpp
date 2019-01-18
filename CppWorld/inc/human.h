#ifndef _HUMAN_H_
#define _HUMAN_H_

#include"beings.h"
namespace World
{
	class Human:public Beings
	{
		public:
			Human(){};
			Human(int age, string name, string country):Beings(age,name), m_country("None")
			{
				cout<<"--->Constructor Human..."<<endl;
				this->m_country = country;
			}
			~Human()
			{
				cout<<"<---Destructor Human..."<<endl;
			}
			virtual int eat();
			virtual int work();
			virtual void show();
			string get_name();
			
		protected:
			string m_country;
	};
}

#endif

