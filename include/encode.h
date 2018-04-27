#ifndef ENCODE_H
#define ENCODE_H
#include "../common/common_utils.h"
#include "../common/cmd_options.h"
#include <errno.h>
#include <stdlib.h>

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
    mfxFrameAllocRequest EncRequest;
    mfxVideoParam par;


    mfxU16 nEncSurfNum ;

    mfxU16 width ;
    mfxU16 height ;
    mfxU8 bitsPerPixel ;
    mfxU32 surfaceSize ;
    mfxU8* surfaceBuffers ;
    mfxFrameSurface1** pEncSurfaces;

    FILE* fSink;
};

#endif // ENCODE_H
