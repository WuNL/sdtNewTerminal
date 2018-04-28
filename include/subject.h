#ifndef SUBJECT_H
#define SUBJECT_H
#include <iostream>
#include <map>
#include <vector>
#include "observer.h"

using namespace std;

class subject
{
public:
    subject() {}
    virtual ~subject() {}
    virtual void addSubscribe(string topic,observer* ob) = 0;
    virtual void notifyTopic(string topic) = 0;
protected:

    map<string,vector<observer*> >  topicMap;
private:

};

#endif // SUBJECT_H
