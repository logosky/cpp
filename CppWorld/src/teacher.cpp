#include<iostream>
#include"teacher.h"
using namespace World;

int Teacher::eat()
{
	cout<<"Teatcher eat..."<<endl;
	return 0;
}

int Teacher::work()
{
	cout<<"Teatcher work:"<<m_subject<<endl;
	return 0;
}

void Teacher::mark(Student *stu, int score)
{
	stu->m_score = score;
}

void Teacher::show()
{
	cout<<"Teacher show - age:"<<m_age<<endl;
	cout<<"\t name:"<<m_name<<endl;
	cout<<"\t country:"<<m_country<<endl;
	cout<<"\t subject:"<<m_subject<<endl;
	cout<<endl;
}

