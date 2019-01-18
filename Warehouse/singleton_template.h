/*
 * singleton.h
 *
 * use example: class xxx : public Singleton<xxx> {
 *                  public:
 *                      friend class Singleton<LogHelp> ;
 *              };
 *              xxx::Ins()->yy()
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_
#include <exception>
#include <new>
#include <stddef.h>

#define assert_if_eq(a, b) assert(a != b)
#define assert_if_ne(a, b) assert(a == b)
#define assert_if_lt(a, b) assert(a >= b)
#define assert_if_le(a, b) assert(a > b)
#define assert_if_gt(a, b) assert(a <= b)
#define assert_if_ge(a, b) assert(a < b)

/// prevent copy
class NonCopyableHelp {
public:
    NonCopyableHelp() {
    }

private:
    NonCopyableHelp(const NonCopyableHelp &) {
    }
    const NonCopyableHelp & operator =(const NonCopyableHelp &) {
        return *this;
    }
};
//singleton template
template<class TYPE>
class Singleton {
public:
    static TYPE* Ins();
    static void Destroy();
protected:
    Singleton() {
    }
    static TYPE *gins;
};

template<class TYPE>
TYPE * Singleton<TYPE>::gins = NULL;

template<class TYPE>
TYPE* Singleton<TYPE>::Ins() {
    if (!gins)
        gins = new (std::nothrow) TYPE;
    return gins;
}

template<class TYPE>
void Singleton<TYPE>::Destroy() {
    if (gins) {
        delete gins;
        gins = NULL;
    }
}
#if 0
//usage
class MonitorHelp: public Singleton<MonitorHelp> {
public:
    friend class Singleton<MonitorHelp> ; //不用再定义构造函数和析构函数
    static void attr_pack_size(int iPackSize);

private:
    MonitorHelp();
    ~MonitorHelp();
};
#endif
#endif /* SINGLETON_H_ */

