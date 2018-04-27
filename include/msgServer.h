#ifndef MSGSERVER_H
#define MSGSERVER_H
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include "unistd.h"
#include "subject.h"
#include "network.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/asio/steady_timer.hpp>

typedef boost::shared_ptr<boost::asio::ip::tcp::socket> sock_ptr;
#define RECVBUFFSIZE 1024

class msgServer : public subject
{
public:
    msgServer(const char* ip,const int port);
    virtual ~msgServer();
    void addSubscribe(char* topic,observer* ob);

    int start();
    int join();
    int quit();

    pthread_t getTid();
    bool isAlive();
    void run();

protected:

private:
    void notifyTopic(char* topic)
    {
        map<char*,vector<observer*> >::iterator it;


        for(it=topicMap.begin();it!=topicMap.end();++it)
        {
            std::cout<<"key: "<<it->first <<std::endl;
        }

        char* s = "capture";
        std::cout<<topicMap.count(s)<<std::endl;

        for(it=topicMap.begin();it!=topicMap.end();++it)
        {
            std::cout<<"key: "<<it->first <<std::endl;
        }

        it = topicMap.find(s);
        //MAP中没有此主题
        if(it==topicMap.end())
        {
            printf("not finded!\n");
        }
        //已经有此主题，那么将该订阅者加入列表
        else
        {
            for(int i=0;i<it->second.size();++i)
            {
                it->second[i]->update("test!!!");
            }
        }
    };
    static void* start_func(void* arg)
    {
        msgServer *ptr = (msgServer*) arg;
        ptr->run();
        return NULL;
    }
    void startrecv();
    void* recv_buff;
    pthread_t m_tid;
    bool      m_isAlive;
    boost::asio::io_service ios;
    boost::asio::ip::tcp::endpoint serverep;

    boost::asio::ip::tcp::acceptor acceptor;


    void accept_handler(const boost::system::error_code &ec, sock_ptr sock);

    void write_handler(const boost::system::error_code& err);

    void on_read(char * ptr, const boost::system::error_code & err, std::size_t read_bytes);
};

#endif // MSGSERVER_H
