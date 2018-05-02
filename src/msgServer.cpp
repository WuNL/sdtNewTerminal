#include "msgServer.h"

msgServer::msgServer(CmdOptions& options):serverep(boost::asio::ip::tcp::v4(),10086),acceptor(ios,serverep)
{
    //ctor
    opt = options;
    startrecv();
}

msgServer::~msgServer()
{
    //dtor
    cout << "~ThreadBase tid="<< m_tid << endl;
    if (m_isAlive)
    {
        cout << "Kill the thread tid= "<< m_tid << endl;
        pthread_kill(m_tid, 0);
    }
    if(recv_buff)
        free(recv_buff);
}

void msgServer::addSubscribe(string topic,observer* ob)
{
    map<string,vector<observer*> >::iterator it;
    it = topicMap.find(topic);
    //MAP中没有此主题
    if(it==topicMap.end())
    {
        vector<observer*> tmpVec;
        tmpVec.push_back(ob);
        topicMap.insert(pair<string,vector<observer*> >(topic,tmpVec));
    }
    //已经有此主题，那么将该订阅者加入列表
    else
    {
        it->second.push_back(ob);
    }

};
int msgServer::start()
{
    cout << "Start a new thread" << endl;
    if (pthread_create(&m_tid, NULL, start_func, (void*)this) != 0)
    {
        cout << "Start a new thread failed!" << endl;
        return -1;
    }

    cout << "Start a new thread success! tid="<< m_tid << endl;
    m_isAlive = true;
    return 0;
}
int msgServer::join()
{
    int ret = -1;
    cout << "Join the thread tid=" << m_tid <<endl;
    ret = pthread_join(m_tid, NULL);

    if (ret != 0)
        cout << "Join the thread fail tid=" << m_tid <<endl;
    else
        cout << "Join the thread success tid=" << m_tid <<endl;

    return ret;
}
int msgServer::quit()
{
    cout << "Quit the thread tid=" << m_tid <<endl;
    m_isAlive = false;
    return 0;
}

pthread_t msgServer::getTid()
{
    return m_tid;
}
bool msgServer::isAlive()
{
    return m_isAlive;
}
void msgServer::run()
{
    ios.run();
}

void msgServer::accept_handler(const boost::system::error_code &ec, sock_ptr sock)
{
    if(ec)
        return;
    //输出客户端连接信息
    std::cout <<"remote ip:"<<sock->remote_endpoint().address()<<std::endl;
    std::cout <<"remote port:"<<sock->remote_endpoint().port() << std::endl;

    char * buff= new char[RECVBUFFSIZE];
    memset(buff,0x00,RECVBUFFSIZE*sizeof(char));
    sock->async_receive(boost::asio::buffer(buff, RECVBUFFSIZE), boost::bind(&msgServer::on_read,this,buff,_1,_2,sock));


    //再次启动异步接受连接
    startrecv();
}

void msgServer::write_handler(const boost::system::error_code& err)
{
    std::cout<<"send msg complete!"<<std::endl;
}

void msgServer::on_read(char * ptr, const boost::system::error_code & err, std::size_t read_bytes,sock_ptr sock)
{
    printf("recving %d bytes:%s\n",read_bytes,ptr);

    if(strcmp(ptr,"1")==0)
        int i = 0;
    if(strcmp(ptr,"2")==0)
        int i = 0;

    notifyTopicOfOptions(string("general"));
    delete[] ptr;

    //异步向客户端发送数据，发送完成时调用write_handler
    sock->async_write_some(boost::asio::buffer("I heard you!"),
                           bind(&msgServer::write_handler,this,boost::asio::placeholders::error));
}

void msgServer::startrecv()
{
    sock_ptr sock(new boost::asio::ip::tcp::socket(ios));
    //当有连接进入时回调accept_handler函数
    acceptor.async_accept(*sock,
                          boost::bind(&msgServer::accept_handler,this,boost::asio::placeholders::error,sock));
}

void msgServer::notifyTopicOfOptions(string topic)
{
    map<string,vector<observer*> >::iterator it;


    for(it=topicMap.begin(); it!=topicMap.end(); ++it)
    {
        std::cout<<"key: "<<it->first <<std::endl;
    }

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
            it->second[i]->updateOptions(opt);
        }
    }
}
