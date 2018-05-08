#ifndef MSGSERVER_H
#define MSGSERVER_H
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include "unistd.h"
#include "subject.h"
#include "network.h"
#include "encode.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/asio/steady_timer.hpp>

using namespace std;
typedef boost::shared_ptr<boost::asio::ip::tcp::socket> sock_ptr;
#define RECVBUFFSIZE 1024

class msgServer : public subject
{
public:
    msgServer(CmdOptions& options);
    virtual ~msgServer();
    void addSubscribe(string topic,observer* ob);

    int start();
    int join();
    int quit();

    pthread_t getTid();
    bool isAlive();
    void run();


protected:

private:
    void notifyTopic(string topic,string msg)
    {
        map<string,vector<observer*> >::iterator it;

        it = topicMap.find(topic);
        //MAP中没有此主题
        if(it==topicMap.end())
        {
            printf("not finded!\n");
        }
        //已经有此主题，那么将该订阅者加入列表
        else
        {
            for(int i=0; i<it->second.size(); ++i)
            {
                it->second[i]->update(string(msg));
            }
        }
    };
    void notifyTopicOfOptions(string topic);
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

    void on_read(char * ptr, const boost::system::error_code & err, std::size_t read_bytes,sock_ptr sock);

private:
    CmdOptions opt;
};

#endif // MSGSERVER_H
