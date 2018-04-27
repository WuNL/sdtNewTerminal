#ifndef ASYNCHRONOUS_SOCKET_H
#define ASYNCHRONOUS_SOCKET_H

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/asio/steady_timer.hpp>

using namespace boost::asio;

class asynchronous_socket
{
public:
    asynchronous_socket(io_service &io,ip::tcp::endpoint &ep):ios(io),acceptor(io,ep)
    {
    }
    virtual ~asynchronous_socket() {}
    void start()
    {
        sock_ptr sock(new ip::tcp::socket(ios));
        //当有连接进入时回调accept_handler函数
        acceptor.async_accept(*sock,
                              boost::bind(&asynchronous_socket::accept_handler,this,boost::asio::placeholders::error,sock));
    }
protected:

private:

    io_service &ios;
    ip::tcp::acceptor acceptor;

    typedef boost::shared_ptr<ip::tcp::socket> sock_ptr;
    void accept_handler(const boost::system::error_code &ec, sock_ptr sock)
    {
        if(ec)
            return;
        //输出客户端连接信息
        std::cout <<"remote ip:"<<sock->remote_endpoint().address()<<std::endl;
        std::cout <<"remote port:"<<sock->remote_endpoint().port() << std::endl;

        char * buff= new char[5120];
        memset(buff,0x00,5120*sizeof(char));
        sock->async_receive(boost::asio::buffer(buff, 5120), boost::bind(&asynchronous_socket::on_read,this,buff,_1,_2));

        //异步向客户端发送数据，发送完成时调用write_handler
        sock->async_write_some(boost::asio::buffer("I heard you!"),
                               bind(&asynchronous_socket::write_handler,this,boost::asio::placeholders::error));
        //再次启动异步接受连接
        start();
    }

    void write_handler(const boost::system::error_code& err)
    {
        std::cout<<"send msg complete!"<<std::endl;
    }

    void on_read(char * ptr, const boost::system::error_code & err, std::size_t read_bytes)
    {
        printf("recving %d\n",read_bytes);

    }
};

#endif // ASYNCHRONOUS_SOCKET_H
