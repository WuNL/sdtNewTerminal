#ifndef OBSERVER_H
#define OBSERVER_H
#include <iostream>
#include <stdio.h>
#include "../common/common_utils.h"
#include "../common/cmd_options.h"
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
    virtual void updateOptions(CmdOptions cmd)
    {

    }
protected:

private:
};

#endif // OBSERVER_H
