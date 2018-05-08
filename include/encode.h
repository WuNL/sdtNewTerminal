#ifndef ENCODE_H
#define ENCODE_H
#include <iostream>
#include <vector>
#include <errno.h>
#include <stdlib.h>
#include "observer.h"

#define CAPTURE_WIDTH 1920
#define CAPTURE_HEIGHT 1080
#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080
#define CAPTURE_FRAMERATE 30

class encode:public observer
{
public:
    encode(CmdOptions& options);
    virtual ~encode();
    int init(CmdOptions& options);
    mfxStatus encodeBuffer(unsigned char* buffer,bool savefile);
    mfxStatus encodeBufferLeft(unsigned char* buffer,bool savefile);
    mfxBitstream mfxBS;
    void update(string msg)
    {
        if(msg==string("insertIDR"))
        {
            printf("insertIDR msg is comming!\n");
            insertIDR = true;
        }
    }
protected:

private:
    mfxStatus sts;
    mfxIMPL impl ;
    mfxVersion ver ;
    MFXVideoSession session;
    MFXVideoENCODE* mfxENC;
    MFXVideoVPP* mfxVPP;
    mfxVideoParam mfxEncParams;
    mfxVideoParam VPPParams;

    mfxFrameAllocRequest EncRequest;
    mfxFrameAllocRequest VPPRequest[2];     // [0] - in, [1] - out
    mfxFrameAllocResponse mfxResponseVPPIn;
    mfxFrameAllocResponse mfxResponseVPPOutEnc;
    mfxVideoParam par;


    mfxU16 nEncSurfNum ;
    mfxU16 nSurfNumVPPIn;
    mfxU16 nSurfNumVPPOutEnc;

    mfxU16 width ;
    mfxU16 height ;
    mfxU8 bitsPerPixel ;
    mfxU32 surfaceSize ;
    mfxU8* surfaceBuffers ;
    mfxFrameSurface1** pEncSurfaces;
    mfxFrameSurface1** pVPPSurfacesIn;
    mfxFrameSurface1** pVPPSurfacesOut;

    FILE* fSink;

    bool useVPP;
    bool insertIDR;
};

#endif // ENCODE_H
