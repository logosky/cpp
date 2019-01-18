#include <iostream>
#include <stdlib.h>
using namespace std;

int testC(int argc,char** args)
{
	int ret = 0;
    string ip = "10.208.140.219";
    int port = 15150;

	cout<<"function testC...."<<endl;
	
    if(argc >= 2)
    {
        ip = args[0];
        port = atoi(args[1]);
    }
	else
	{
		cout << "wrong args..." << endl;
		return 0;
	}
	cout<<"args ip:"<<ip<<"\tport:"<<port<<endl;
	
	return ret;
}
