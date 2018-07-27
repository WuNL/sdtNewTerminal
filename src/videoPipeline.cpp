#include "videoPipeline.h"

videoPipeline::videoPipeline(CmdOptions& options):captureDevice(NULL),
    encoder(NULL),
    pkt(NULL),
    nethandle(NULL),
    msgSer(NULL),
    isRun(true),
    isReleased(false),
    fifo(NULL)
{
    //ctor
    opt = options;
}

videoPipeline::~videoPipeline()
{
    //dtor
}

void videoPipeline::start()
{
    init(opt);
    pthread_create(&thread,NULL,run,(void *)this);
}

void videoPipeline::cancel()
{
    isRun = false;
    //pthread_cancel(thread);
}

void videoPipeline::join()
{
    void *status;
    int rc = pthread_join(thread, &status);
    if (rc)
    {
        printf("ERROR; return code from pthread_join() is %d\n", rc);
        exit(-1);
    }
    printf("Main: completed join with thread %ld having a status of %ld\n",thread,(long)status);
}

void* videoPipeline::run(void *threadarg)
{
    videoPipeline* ptr = (videoPipeline*)threadarg;
    while(ptr->GetisRun())
    {
        ptr->videoInPipeline();
    }
    printf("videoPipeline ready to release!\n");
    ptr->realRelease();
    return NULL;
}

void videoPipeline::init(CmdOptions& options)
{
    opt = options;

    captureDevice=NULL;
    if(options.values.CaptureDevice==MFX_CAPTURE_DEVICE_V4L2)
    {
        captureDevice = new capture;
        //ms.addSubscribe("capture",(observer*)capturedevice);
    }
    else
    {
        printf("error: This capture device not supported yet\n");
        return;
    }

    encoder = NULL;
    encoder = new encode(options);
    sj->addSubscribe(string("encode"),(observer*)encoder);
    encoder->init(options);

    if(options.values.CodecId==MFX_CODEC_AVC)
    {
        h264Packet* h264pkt = NULL;
        h264pkt = new h264Packet(1400,10);
        pkt = (packet*) h264pkt;
    }
    else if(options.values.CodecId==MFX_CODEC_HEVC)
    {
        h265Packet* h265pkt = NULL;
        h265pkt = new h265Packet(1400,10);
        pkt = (packet*) h265pkt;
    }

    nethandle = NULL;
    struct net_param netp;

    netp.type = UDP;
    netp.serip = options.values.Ip;
    netp.serport = options.values.Port;

    nethandle = net_open(netp);
    if (!nethandle)
    {
        printf("--- Open network failed\n");
        return;
    }
    isReleased = false;

    fifo = shmfifo_init(1234, sizeof(vb), 30);

}

void videoPipeline::realRelease()
{
    if(captureDevice)
        delete captureDevice;
    if(encoder)
        delete encoder;
    if(pkt)
        delete pkt;
    if(nethandle)
        net_close(nethandle);
    isReleased = true;
}

void videoPipeline::videoInPipeline()
{
    captureDevice->camera_start();
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    /* skip 5 frames for booting a cam */
    for (int i = 0; i < 5; i++)
    {
        captureDevice->camera_frame(timeout);
    }
    //接受capture得到的YUV，格式为YUV420
    unsigned char* yuvbuffer=NULL;
    uint32_t height,width;
    mfxStatus sts = MFX_ERR_NONE;
    sts = MFX_ERR_NONE;
    int cnt = 0;
    while((MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts) && GetisRun())
    {
        cnt++;
        captureDevice->camera_frame_and_decode(timeout,yuvbuffer,width,height);

        sts = encoder->encodeBuffer(yuvbuffer,false);

        //skip a frame; used for 60 fps to 30 fps
        if(MFX_ERR_MORE_DATA==sts)
            continue;

        if(encoder->mfxBS.DataLength==0)
            continue;

        //WuNL
        //todo share buffer

        memset(&vb_, 0, sizeof(vb));
        vb_.size = encoder->mfxBS.DataLength;

        memcpy(vb_.buffer,encoder->mfxBS.Data + encoder->mfxBS.DataOffset,(size_t)encoder->mfxBS.DataLength);

        shmfifo_put(fifo, &vb_);

        //WuNL
        //网络发包版本

//        pkt->pack_put(encoder->mfxBS.Data + encoder->mfxBS.DataOffset, encoder->mfxBS.DataLength);
//
//        int pac_len;
//        while (pkt->pack_get(&pac_buf, &pac_len) == 1)
//        {
//            int ret = net_send(nethandle, pac_buf, pac_len);
//            //printf("send pack data, size: %d\n", pac_len);
//            if (ret != pac_len)
//            {
//                printf("send pack data failed, size: %d, err: %s\n", pac_len,
//                       strerror(errno));
//            }
//        }

        encoder->mfxBS.DataLength = 0;


        sts = MFX_ERR_NONE;
        MSDK_BREAK_ON_ERROR(sts);



        //printf("Frame number: %d\r", nFrame);
        fflush(stdout);

        memset(yuvbuffer, 1, sizeof(unsigned char) * width * height * 3 / 2);//清0
    }
}
