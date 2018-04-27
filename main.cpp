#include <iostream>
#include "capture.h"
#include "encode.h"
#include "packet.h"
#include "network.h"
#include "msgServer.h"
using namespace std;

static void usage(CmdOptionsCtx* ctx)
{
    printf(
        "Encodes INPUT and optionally writes OUTPUT. If INPUT is not specified\n"
        "simulates input with empty frames filled with the color.\n"
        "\n"
        "Usage: %s [options] [INPUT] [OUTPUT]\n", ctx->program);
}

int main(int argc, char** argv)
{
    msgServer ms("127.0.0.1",10086);
    ms.start();


    mfxStatus sts = MFX_ERR_NONE;
    bool bEnableInput;  // if true, removes all YUV file reading (which is replaced by pre-initialized surface data). Workload runs for 1000 frames.
    bool bEnableOutput; // if true, removes all output bitsteam file writing and printing the progress
    CmdOptions options;

    // =====================================================================
    // Intel Media SDK encode pipeline setup
    // - In this example we are encoding an AVC (H.264) stream
    // - For simplistic memory management, system memory surfaces are used to store the raw frames
    //   (Note that when using HW acceleration video surfaces are prefered, for better performance)
    //

    //1. Read options from the command line (if any is given)
    memset(&options, 0, sizeof(CmdOptions));
    options.ctx.options = OPTIONS_ENCODE;
    options.ctx.usage = usage;
    // Set default values:
    options.values.impl = MFX_IMPL_AUTO_ANY;

    // here we parse options
    ParseOptions(argc, argv, &options);
    if (!options.values.CodecId)
    {
        printf("error: codec type not set (mandatory)\n");
        return -1;
    }
    if (!options.values.Width || !options.values.Height)
    {
        printf("error: input video geometry not set (mandatory)\n");
        return -1;
    }
    if (!options.values.Bitrate)
    {
        printf("error: bitrate not set (mandatory)\n");
        return -1;
    }
    if (!options.values.FrameRateN || !options.values.FrameRateD)
    {
        printf("error: framerate not set (mandatory)\n");
        return -1;
    }
    if (!options.values.CaptureDevice)
    {
        printf("error: CaptureDevice not set (mandatory)\n");
        return -1;
    }

    bEnableInput = (options.values.SourceName[0] != '\0');
    bEnableOutput = (options.values.SinkName[0] != '\0');

    encode encoder(options);
    encoder.init(options);


    capture* capturedevice=NULL;
    if(options.values.CaptureDevice==MFX_CAPTURE_DEVICE_V4L2)
    {
        capturedevice = new capture;
        ms.addSubscribe("capture",(observer*)capturedevice);
    }
    else
    {
        printf("error: This capture device not supported yet\n");
        return -1;
    }

    packet* pkt=NULL;
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


    struct net_param netp;
    struct net_handle *nethandle = NULL;
    netp.type = UDP;
    netp.serip = "192.168.1.100";
    netp.serport = 8088;

    nethandle = net_open(netp);
    if (!nethandle)
    {
        printf("--- Open network failed\n");
        return -1;
    }
    void *cap_buf, *cvt_buf, *hd_buf, *enc_buf, *pac_buf;

//    -----------------------------------------------------------


    capturedevice->camera_start();
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    /* skip 5 frames for booting a cam */
    for (int i = 0; i < 5; i++)
    {
        capturedevice->camera_frame(timeout);
    }
    //接受capture得到的YUV，格式为YUV420
    unsigned char* yuvbuffer=NULL;
    uint32_t height,width;

    sts = MFX_ERR_NONE;
    int cnt = 0;
    while((MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts))
    {
        cnt++;
        capturedevice->camera_frame_and_decode(timeout,yuvbuffer,width,height);

        sts = encoder.encodeBuffer(yuvbuffer,true);

        if(encoder.mfxBS.DataLength==0)
            continue;
        pkt->pack_put(encoder.mfxBS.Data + encoder.mfxBS.DataOffset, encoder.mfxBS.DataLength);

        int pac_len;
        while (pkt->pack_get(&pac_buf, &pac_len) == 1)
        {
//            int ret = net_send(nethandle, pac_buf, pac_len);
//            if (ret != pac_len)
//            {
//                printf("send pack data failed, size: %d, err: %s\n", pac_len,
//                       strerror(errno));
//            }
        }
        encoder.mfxBS.DataLength = 0;


        sts = MFX_ERR_NONE;
        MSDK_BREAK_ON_ERROR(sts);



        //printf("Frame number: %d\r", nFrame);
        fflush(stdout);

        memset(yuvbuffer, 1, sizeof(unsigned char) * width * height * 3 / 2);//清0
    }
    encoder.encodeBufferLeft(yuvbuffer,true);
    memset(yuvbuffer, 1, sizeof(unsigned char) * width * height * 3 / 2);//清0

    cout<<width<<"-"<<height<<endl;



    Release();
    if(capturedevice)
        delete capturedevice;
    if(pkt)
        delete pkt;
    return 0;
}
