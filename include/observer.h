#ifndef OBSERVER_H
#define OBSERVER_H
#include <iostream>
#include <stdio.h>
using namespace std;
class observer
{
public:
    observer() {}
    virtual ~observer() {}
    virtual void update(string msg)
    {
        printf("update:%s\n",msg.c_str());
    }

protected:

private:
};

#endif // OBSERVER_H
