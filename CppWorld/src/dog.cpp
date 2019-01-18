#include"dog.h"

using namespace World;

int Dog::eat()
{
	cout<<"Dog eat..."<<endl;
	return 0;
}

void Dog::show()
{
	cout<<"Dog show - age:"<<m_age<<endl;
	cout<<"\t name:"<<m_name<<endl;
	cout<<"\t type:"<<m_type<<endl;
	cout<<endl;
}

Dog Dog::operator+(const Dog& dog)
{
	Dog obj;
	obj.m_age = this->m_age + dog.m_age;
	obj.m_name = this->m_name + dog.m_name;
	obj.m_type = this->m_type + dog.m_type;

	return obj;
}



