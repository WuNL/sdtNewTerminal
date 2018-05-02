#ifndef VIDEOPIPELINE_H
#define VIDEOPIPELINE_H
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "capture.h"
#include "encode.h"
#include "packet.h"
#include "network.h"
#include "msgServer.h"

class videoPipeline : public observer
{
public:
    videoPipeline(CmdOptions& options);
    virtual ~videoPipeline();

    bool GetisRun()
    {
        return isRun;
    }
    void SetisRun(bool val)
    {
        isRun = val;
    }
    void update(string msg)
    {
        printf("videoPipeline update:%s\n",msg.c_str());
        if(msg==string("stop"))
        {
            isRun=false;
        }
        else if(isReleased==true && msg==string("start"))
        {
            isRun=true;
            start();
        }
    }

    void updateOptions(CmdOptions options)
    {
        opt = options;
        isRun=false;
        join();
        isRun=true;
        start();
    }

    void start();
    //阻塞，等待这个线程工作完成
    void join();
    //对这个线程发出停止信号，线程在时机合适时停止运行
    void cancel();


    //负责创建各个模块的实例
    void init(CmdOptions& options);

private:

    static void* run(void *threadarg);

    static void release(void* threadarg)
    {
        videoPipeline* ptr = (videoPipeline*)threadarg;
        ptr->realRelease();
    }
    void realRelease();

    void videoInPipeline();



protected:

private:
    CmdOptions opt;

    bool isRun,isReleased;

    capture* captureDevice;
    encode* encoder;
    packet* pkt;
    struct net_handle *nethandle;

    msgServer* msgSer;

    pthread_t thread;
    void *cap_buf, *cvt_buf, *hd_buf, *enc_buf, *pac_buf;
};

#endif // VIDEOPIPELINE_H
