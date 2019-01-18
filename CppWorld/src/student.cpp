#include<iostream>
#include"student.h"
using namespace World;

//static member variable should be defined 
int Student::m_num = 0;

int Student::eat()
{
	cout<<"Student eat..."<<endl;
	return 0;
}

int Student::work()
{
	cout<<"Student work:"<<m_level<<endl;
	return 0;
}

void Student::show()
{
	cout<<"Student show - total:"<<Student::m_num<<endl;
	cout<<"\t age:"<<m_age<<endl;
	cout<<"\t name:"<<m_name<<endl;
	cout<<"\t country:"<<m_country<<endl;
	cout<<"\t level:"<<m_level<<endl;
	cout<<"\t score:"<<m_score<<endl;
	cout<<endl;
}


