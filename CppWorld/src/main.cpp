#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include"comm.h"
#include"teacher.h"
#include"student.h"
#include"dog.h"

using namespace World;

void help()
{
	const char* msg =
	"usage:\n" \
	"		--help,-h\n"
	"		--type=<1,2,3>, -t <1,2,3>\n";
	
	fprintf(stdout, "%s", msg);
}

int main(int argc,char *argv[])
{
	const char optstring[64] = "ht:";
	int result;
	int type = 0;
	if(argc < 2)
	{
		help();
		return 0;
	}
	while ((result = getopt(argc, argv, optstring)) != -1)
    {
		switch (result) 
        {
			case 'h':
                help();
                return 0;
			case 't':
                type = atoi(optarg);
                break;
			default:
                help();
                return -1;
		}
	}
	cout<<"arg type:"<<type<<endl;
	
	Human *human[3];
	Teacher *ta = new Teacher(22, "Tom", "USA", "English");
	cout<<"create a Teacher..."<<endl;
	Teacher *tb = new Teacher(24, "Jack", "England", "Math");
	cout<<"create a Teacher..."<<endl;
	Teacher *tc = new Teacher(20, "Rose", "Japan", "Science");
	cout<<"create a Teacher..."<<endl;
	cout<<endl;
	
	human[0] = ta;
	human[1] = tb;
	human[2] = tc;

#ifdef TEST
	cout<<"You define TEST..."<<endl;
#endif

	cout<<"size:"<<sizeof(human)/sizeof(Human *)<<endl;
	
	for(int i = 0;i < sizeof(human)/sizeof(Human *);i++)
	{
		human[i]->show();
		human[i]->work();
	}

	Student *sa = new Student(12, "angel", "USA", 3);
	cout<<"create a Student..."<<endl;
	Student *sb = new Student(13, "andy", "German", 4);
	cout<<"create a Student..."<<endl;
	cout<<endl;
	
	ta->mark(sa,93);
	tb->mark(sb,96);

	//function template
	obj_do(sa);
	obj_do(sb);

	Dog da(3, "Dudu", "Japan");
	Dog db(2, "Tutu", "Germany");
	Dog dc = da + db;
	dc.show();

	//class template
	Stack<int> stack;
	stack.push(12);
	try
	{
		int a = stack.top();
		cout<<"stack top:"<<a<<endl;
		stack.pop();
		a = stack.top();
		cout<<"stack top:"<<a<<endl;
		stack.pop();
	}
	catch(const out_of_range &e)
	{
		cout<<"Error:"<<e.what()<<endl;
	}
	
	delete ta;
	cout<<"delete a Teacher..."<<endl;
	delete tb;
	cout<<"delete a Teacher..."<<endl;
	delete tc;
	cout<<"delete a Teacher..."<<endl;

	delete sa;
	cout<<"delete a Student..."<<endl;
	delete sb;
	cout<<"delete a Student..."<<endl;
	
	return 0;
}

