#include <iostream>
using namespace std;
class  locker
{
    public:
    inline locker( )  //���캯��
    {
        cout<<"locker constructor..."<<endl;
        pthread_mutex_init(&mutex,NULL);
    }

    inline  ~locker( ) //��������
    {
        cout<<"locker destructor..."<<endl;
        pthread_mutex_destroy(&mutex);
    }

    public:
    inline void lock()
    {
        cout<<"locker lock..."<<endl;
        pthread_mutex_lock(&mutex);
    }

    inline void unlock()
    {
        cout<<"locker unlock..."<<endl;
        pthread_mutex_unlock(&mutex);
    }

    private:
    pthread_mutex_t   mutex;
};

class Singleton{
public:
    static Singleton* getInstance();

private:
    Singleton(){};
    //�Ѹ��ƹ��캯����=������Ҳ��Ϊ˽��,��ֹ������
    Singleton(const Singleton&);
    bool operator==(const Singleton&);

    static Singleton* instance;
    static locker lock;
};

Singleton* Singleton::getInstance()
{
    if (instance == NULL)
    {
        lock.lock();
        if (instance == NULL)
        {
            cout<<"first create Singleton..."<<endl;
            instance = new Singleton();
        }
        lock.unlock();
    }
    else
    {
        cout<<"already has Singleton..."<<endl;
    }
    
    return instance;
}

bool Singleton::operator==(const Singleton& item){
    if(this->instance == item.instance)
        return true;
    return false;
}
//����һ��Ҫ���ж��壬���е�ֻ������
Singleton* Singleton::instance = NULL;
locker Singleton::lock;
int main(){
    Singleton* singleton1 = Singleton::getInstance();
    Singleton* singleton2 = Singleton::getInstance();

    if (singleton1 == singleton2)
        cout<<"singleton1 = singleton2"<<endl;

    return 0;
}