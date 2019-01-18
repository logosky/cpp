#include"human.h"

using namespace World;

int Human::eat()
{
	cout<<"Human eat..."<<endl;
	return 0;
}

int Human::work()
{
	cout<<"Human work..."<<endl;
	return 0;
}

void Human::show()
{
	cout<<"Human show - age:"<<m_age<<endl;
	cout<<"\t name:"<<m_name<<endl;
	cout<<"\t country:"<<m_country<<endl;
	cout<<endl;
}

string Human::get_name()
{
	return m_name;
}


