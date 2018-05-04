#ifndef ENCODE_H
#define ENCODE_H
#include <iostream>
#include <vector>
#include <errno.h>
#include <stdlib.h>
#include "observer.h"

#define CAPTURE_WIDTH 1920
#define CAPTURE_HEIGHT 1080
#define CAPTURE_FRAMERATE 30

class encode
{
public:
    encode(CmdOptions& options);
    virtual ~encode();
    int init(CmdOptions& options);
    mfxStatus encodeBuffer(unsigned char* buffer,bool savefile);
    mfxStatus encodeBufferLeft(unsigned char* buffer,bool savefile);
    mfxBitstream mfxBS;
protected:

private:
    mfxStatus sts;
    mfxIMPL impl ;
    mfxVersion ver ;
    MFXVideoSession session;
    MFXVideoENCODE* mfxENC;
    mfxVideoParam mfxEncParams;
    mfxVideoParam VPPParams;

    mfxFrameAllocRequest EncRequest;
    mfxFrameAllocRequest VPPRequest[2];     // [0] - in, [1] - out
    mfxFrameAllocResponse mfxResponseVPPIn;
    mfxFrameAllocResponse mfxResponseVPPOutEnc;
    mfxVideoParam par;


    mfxU16 nEncSurfNum ;

    mfxU16 width ;
    mfxU16 height ;
    mfxU8 bitsPerPixel ;
    mfxU32 surfaceSize ;
    mfxU8* surfaceBuffers ;
    mfxFrameSurface1** pEncSurfaces;

    FILE* fSink;

    bool useVPP;
};

#endif // ENCODE_H
