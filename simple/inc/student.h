#ifndef _STUDENT_H_
#define _STUDENT_H_

#include"human.h"
#include"teacher.h"

namespace World
{
	class Student:public Human
	{
		public:
			Student(int age = 0, const string &name = "", const string &country = "", int level = 0, int score = 0):Human(age,name,country)
			{
				cout<<"--->Constructor Student..."<<endl;
				m_level = level;
				m_score = score;
				m_num++;
			}
			~Student()
			{
				cout<<"<---Destructor Student..."<<endl;
				m_num--;
			}
			virtual int eat();
			virtual int work();
			virtual void show();
			friend class Teacher;
			
		private:
			static int m_num;	//static,student num
			int m_level;
			int m_score;
	};
}

#endif


