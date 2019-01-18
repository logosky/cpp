#ifndef _TEACHER_H_
#define _TEACHER_H_

#include"human.h"
#include"student.h"
using namespace std;

namespace World
{
	class Student;
	class Teacher:public Human
	{
		public:
			Teacher(int age = 0, const string &name = "", const string &country = "", const string &subject = ""):Human(age,name,country)
			{
				cout<<"--->Constructor Teacher..."<<endl;
				m_subject = subject;
			}
			
			~Teacher()
			{
				cout<<"<---Destructor Teacher..."<<endl;
			}
			virtual int eat();
			virtual int work();
			virtual void show();
			void mark(Student *stu, int score);
			
		private:
			string m_subject;
	};
}

#endif
