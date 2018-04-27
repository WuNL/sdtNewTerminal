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
    virtual void addSubscribe(char* topic,observer* ob) = 0;
    virtual void notifyTopic(char* topic) = 0;
protected:

    map<char*,vector<observer*> >  topicMap;
private:

};

#endif // SUBJECT_H
