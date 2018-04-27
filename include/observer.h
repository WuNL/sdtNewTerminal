#ifndef OBSERVER_H
#define OBSERVER_H
#include <iostream>
#include <stdio.h>

class observer
{
public:
    observer() {}
    virtual ~observer() {}
    virtual void update(char* msg)
    {
        printf("update!\n");
    }

protected:

private:
};

#endif // OBSERVER_H
