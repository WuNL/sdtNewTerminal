#include "encode.h"
namespace encodens
{
void quit(const char * msg)
{
    fprintf(stderr, "[%s] %d: %s\n", msg, errno, strerror(errno));
    exit(EXIT_FAILURE);
}
};

encode::encode(CmdOptions& options):sts(MFX_ERR_NONE),
    impl(options.values.impl),
    ver(
{
    {
        0, 1
    }
}),
fSink(NULL)
{
    //ctor


}

encode::~encode()
{
    //dtor
    printf("~encode\n");
    mfxENC->Close();
    delete mfxENC;
    // session closed automatically on destruction

    for (int i = 0; i < nEncSurfNum; i++)
        delete pEncSurfaces[i];
    MSDK_SAFE_DELETE_ARRAY(pEncSurfaces);
    MSDK_SAFE_DELETE_ARRAY(mfxBS.Data);

    MSDK_SAFE_DELETE_ARRAY(surfaceBuffers);
    if(fSink)
        fclose(fSink);
}

int encode::init(CmdOptions& options)
{
    sts = Initialize(impl, ver, &session, NULL);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Create Media SDK encoder
    mfxENC = new MFXVideoENCODE(session);

    memset(&mfxEncParams, 0, sizeof(mfxEncParams));

    mfxEncParams.mfx.CodecId = options.values.CodecId;

    if(mfxEncParams.mfx.CodecId == MFX_CODEC_AVC)
    {
        MSDK_FOPEN(fSink, "test.264", "wb");
    }

    else if(mfxEncParams.mfx.CodecId == MFX_CODEC_HEVC)
    {
        MSDK_FOPEN(fSink, "test.265", "wb");
    }

    if(mfxEncParams.mfx.CodecId == MFX_CODEC_AVC)
    {
        mfxEncParams.mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
        mfxEncParams.mfx.TargetKbps = options.values.Bitrate;
        mfxEncParams.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
        mfxEncParams.mfx.FrameInfo.FrameRateExtN = options.values.FrameRateN;
        mfxEncParams.mfx.FrameInfo.FrameRateExtD = options.values.FrameRateD;
        mfxEncParams.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
        mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
        mfxEncParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
        mfxEncParams.mfx.FrameInfo.CropX = 0;
        mfxEncParams.mfx.FrameInfo.CropY = 0;
        mfxEncParams.mfx.FrameInfo.CropW = options.values.Width;
        mfxEncParams.mfx.FrameInfo.CropH = options.values.Height;
        // Width must be a multiple of 16
        // Height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
        mfxEncParams.mfx.FrameInfo.Width = MSDK_ALIGN16(options.values.Width);
        mfxEncParams.mfx.FrameInfo.Height =
            (MFX_PICSTRUCT_PROGRESSIVE == mfxEncParams.mfx.FrameInfo.PicStruct) ?
            MSDK_ALIGN16(options.values.Height) :
            MSDK_ALIGN32(options.values.Height);

        mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;
    }
    else if(mfxEncParams.mfx.CodecId == MFX_CODEC_HEVC)
    {
        mfxEncParams.mfx.CodecId = MFX_CODEC_HEVC;
        mfxEncParams.mfx.TargetUsage = MFX_TARGETUSAGE_BALANCED;
        mfxEncParams.mfx.TargetKbps = options.values.Bitrate;
        mfxEncParams.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
        mfxEncParams.mfx.FrameInfo.FrameRateExtN = options.values.FrameRateN;
        mfxEncParams.mfx.FrameInfo.FrameRateExtD = options.values.FrameRateD;
        mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
        mfxEncParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
        mfxEncParams.mfx.FrameInfo.CropX = 0;
        mfxEncParams.mfx.FrameInfo.CropY = 0;
        mfxEncParams.mfx.FrameInfo.CropW = options.values.Width;
        mfxEncParams.mfx.FrameInfo.CropH = options.values.Height;
        // Width must be a multiple of 16
        // Height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
        mfxEncParams.mfx.FrameInfo.Width = MSDK_ALIGN16(options.values.Width);
        mfxEncParams.mfx.FrameInfo.Height =
            (MFX_PICSTRUCT_PROGRESSIVE == mfxEncParams.mfx.FrameInfo.PicStruct) ?
            MSDK_ALIGN16(options.values.Height) :
            MSDK_ALIGN32(options.values.Height);

        mfxEncParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY;

        //4. Load the HEVC plugin
        mfxPluginUID codecUID;
        bool success = true;
        codecUID = msdkGetPluginUID(MFX_IMPL_HARDWARE, MSDK_VENCODE, mfxEncParams.mfx.CodecId);
        if (AreGuidsEqual(codecUID, MSDK_PLUGINGUID_NULL))
        {
            printf("Get Plugin UID for HEVC is failed.\n");
            success = false;
        }

        printf("Loading HEVC plugin: %s\n", ConvertGuidToString(codecUID));

        //On the success of get the UID, load the plugin
        if (success)
        {
            sts = MFXVideoUSER_Load(session, &codecUID, ver.Major);
            if (sts < MFX_ERR_NONE)
            {
                printf("Loading HEVC plugin failed\n");
                success = false;
            }
        }
    }

    // Validate video encode parameters (optional)
    // - In this example the validation result is written to same structure
    // - MFX_WRN_INCOMPATIBLE_VIDEO_PARAM is returned if some of the video parameters are not supported,
    //   instead the encoder will select suitable parameters closest matching the requested configuration
    sts = mfxENC->Query(&mfxEncParams, &mfxEncParams);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    //4. Query number of required surfaces for encoder

    memset(&EncRequest, 0, sizeof(EncRequest));
    sts = mfxENC->QueryIOSurf(&mfxEncParams, &EncRequest);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    nEncSurfNum = EncRequest.NumFrameSuggested;

    if(mfxEncParams.mfx.CodecId == MFX_CODEC_AVC)
    {
        // Allocate surfaces for encoder
        // - Width and height of buffer must be aligned, a multiple of 32
        // - Frame surface array keeps pointers all surface planes and general frame info
        width = (mfxU16) MSDK_ALIGN32(EncRequest.Info.Width);
        height = (mfxU16) MSDK_ALIGN32(EncRequest.Info.Height);
        bitsPerPixel = 16;        // NV12 format is a 12 bits per pixel format
        surfaceSize = width * height * bitsPerPixel / 8 * 1.5;
        surfaceBuffers = (mfxU8*) new mfxU8[surfaceSize * nEncSurfNum];

        // Allocate surface headers (mfxFrameSurface1) for encoder
        pEncSurfaces = new mfxFrameSurface1 *[nEncSurfNum];
        MSDK_CHECK_POINTER(pEncSurfaces, MFX_ERR_MEMORY_ALLOC);
        for (int i = 0; i < nEncSurfNum; i++)
        {
            pEncSurfaces[i] = new mfxFrameSurface1;
            memset(pEncSurfaces[i], 0, sizeof(mfxFrameSurface1));
            memcpy(&(pEncSurfaces[i]->Info), &(mfxEncParams.mfx.FrameInfo), sizeof(mfxFrameInfo));
            pEncSurfaces[i]->Data.Y = &surfaceBuffers[surfaceSize * i];
            pEncSurfaces[i]->Data.U = pEncSurfaces[i]->Data.Y + width * height;
            pEncSurfaces[i]->Data.V = pEncSurfaces[i]->Data.U + 1;
            pEncSurfaces[i]->Data.Pitch = width;
            if (!1)
            {
                ClearYUVSurfaceSysMem(pEncSurfaces[i], width, height);
            }
        }
    }
    else if(mfxEncParams.mfx.CodecId == MFX_CODEC_HEVC)
    {
        //5. Allocate surfaces for encoder
        // - Width and height of buffer must be aligned, a multiple of 32
        // - Frame surface array keeps pointers all surface planes and general frame info
        // - For HEVC 10 bit, the bytes per pixel is doubled, so the width is multiplied by 2.
        width = (mfxU16) MSDK_ALIGN32(EncRequest.Info.Width * 2);
        height = (mfxU16) MSDK_ALIGN32(EncRequest.Info.Height);
        bitsPerPixel = 12;        // P010 format is a 24 bits per pixel format but we have double the width
        surfaceSize = width * height * bitsPerPixel / 8;
        surfaceBuffers = (mfxU8*) new mfxU8[surfaceSize * nEncSurfNum];

        // Allocate surface headers (mfxFrameSurface1) for encoder, noticed the width has been doubled for P010.
        pEncSurfaces = new mfxFrameSurface1 *[nEncSurfNum];
        MSDK_CHECK_POINTER(pEncSurfaces, MFX_ERR_MEMORY_ALLOC);
        for (int i = 0; i < nEncSurfNum; i++)
        {
            pEncSurfaces[i] = new mfxFrameSurface1;
            memset(pEncSurfaces[i], 0, sizeof(mfxFrameSurface1));
            memcpy(&(pEncSurfaces[i]->Info), &(mfxEncParams.mfx.FrameInfo), sizeof(mfxFrameInfo));
            pEncSurfaces[i]->Data.Y = &surfaceBuffers[surfaceSize * i];
            pEncSurfaces[i]->Data.U = pEncSurfaces[i]->Data.Y + width * height;
            pEncSurfaces[i]->Data.V = pEncSurfaces[i]->Data.U + 1;
            pEncSurfaces[i]->Data.Pitch = width;
            if (!1)
            {
                ClearYUVSurfaceSysMem(pEncSurfaces[i], width, height);
            }
        }
    }

    //5. Initialize the Media SDK encoder
    sts = mfxENC->Init(&mfxEncParams);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Retrieve video parameters selected by encoder.
    // - BufferSizeInKB parameter is required to set bit stream buffer size

    memset(&par, 0, sizeof(par));
    sts = mfxENC->GetVideoParam(&par);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    //6. Prepare Media SDK bit stream buffer

    memset(&mfxBS, 0, sizeof(mfxBS));
    mfxBS.MaxLength = par.mfx.BufferSizeInKB * 1000;
    mfxBS.Data = new mfxU8[mfxBS.MaxLength];
    MSDK_CHECK_POINTER(mfxBS.Data, MFX_ERR_MEMORY_ALLOC);
}

mfxStatus encode::encodeBuffer(unsigned char* buffer,bool savefile)
{
    if(!buffer)
        return MFX_ERR_NULL_PTR;
    int nEncSurfIdx = 0;
    mfxU32 nFrame = 0;
    mfxSyncPoint syncp;
    nEncSurfIdx = GetFreeSurfaceIndex(pEncSurfaces, nEncSurfNum);   // Find free frame surface
    MSDK_CHECK_ERROR(MFX_ERR_NOT_FOUND, nEncSurfIdx, MFX_ERR_MEMORY_ALLOC);

    sts = LoadRawFrameFromV4l2(pEncSurfaces[nEncSurfIdx], buffer);

    MSDK_RETURN_ON_ERROR(sts);

    for (;;)
    {
        // Encode a frame asychronously (returns immediately)
        sts = mfxENC->EncodeFrameAsync(NULL, pEncSurfaces[nEncSurfIdx], &mfxBS, &syncp);
        //printf("********************\n");
        if (MFX_ERR_NONE < sts && !syncp)       // Repeat the call if warning and no output
        {
            if (MFX_WRN_DEVICE_BUSY == sts)
                MSDK_SLEEP(1);  // Wait if device is busy, then repeat the same call
        }
        else if (MFX_ERR_NONE < sts && syncp)
        {
            sts = MFX_ERR_NONE;     // Ignore warnings if output is available
            break;
        }
        else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
        {
            // Allocate more bitstream buffer memory here if needed...
            break;
        }
        else
            break;
    }


    if (MFX_ERR_NONE == sts)
    {
        sts = session.SyncOperation(syncp, 60000);      // Synchronize. Wait until encoded frame is ready
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        ++nFrame;
        if (savefile)
        {
            //原版，将编码厚的264存文件
            int tmp = mfxBS.DataLength;
            sts = WriteBitStreamFrame(&mfxBS, fSink);
            MSDK_RETURN_ON_ERROR(sts);
            mfxBS.DataLength = tmp;
            fflush(stdout);
        }
//        else
//            mfxBS.DataLength = 0;
    }
}

mfxStatus encode::encodeBufferLeft(unsigned char* buffer,bool savefile)
{
    if(!buffer)
        return MFX_ERR_NULL_PTR;
    int nEncSurfIdx = 0;
    mfxU32 nFrame = 0;
    mfxSyncPoint syncp;
    while (MFX_ERR_NONE <= sts)
    {
        for (;;)
        {
            // Encode a frame asychronously (returns immediately)
            sts = mfxENC->EncodeFrameAsync(NULL, NULL, &mfxBS, &syncp);

            if (MFX_ERR_NONE < sts && !syncp)       // Repeat the call if warning and no output
            {
                if (MFX_WRN_DEVICE_BUSY == sts)
                    MSDK_SLEEP(1);  // Wait if device is busy, then repeat the same call
            }
            else if (MFX_ERR_NONE < sts && syncp)
            {
                sts = MFX_ERR_NONE;     // Ignore warnings if output is available
                break;
            }
            else
                break;
        }

        if (MFX_ERR_NONE == sts)
        {
            sts = session.SyncOperation(syncp, 60000);      // Synchronize. Wait until encoded frame is ready
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

            ++nFrame;
            if (savefile)
            {
                sts = WriteBitStreamFrame(&mfxBS, fSink);
                MSDK_BREAK_ON_ERROR(sts);
            }
            else
                mfxBS.DataLength = 0;
        }
    }

}
