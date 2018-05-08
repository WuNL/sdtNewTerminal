#include "encode.h"
#define MSDK_ZERO_MEMORY(VAR)                    {memset(&VAR, 0, sizeof(VAR));}
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
fSink(NULL),
useVPP(false),
insertIDR(false)
{
    //ctor
    surfaceBuffers = NULL;

}

encode::~encode()
{
    //dtor
    printf("~encode\n");
    mfxENC->Close();
    delete mfxENC;
    // session closed automatically on destruction

    MSDK_SAFE_DELETE_ARRAY(mfxBS.Data);
    if(!surfaceBuffers)
        MSDK_SAFE_DELETE_ARRAY(surfaceBuffers);
    if(fSink)
        fclose(fSink);
}

int encode::init(CmdOptions& options)
{
    if(options.values.Bitrate!=CAPTURE_FRAMERATE || options.values.Width!=CAPTURE_WIDTH || options.values.Height!=CAPTURE_HEIGHT)
        useVPP = true;


    sts = Initialize(impl, ver, &session, NULL);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);


    memset(&mfxEncParams, 0, sizeof(mfxEncParams));

    mfxEncParams.mfx.CodecId = options.values.CodecId;

    mfxEncParams.mfx.GopOptFlag = MFX_GOP_STRICT;
    mfxEncParams.mfx.GopPicSize = (mfxU16)1200;
    mfxEncParams.mfx.IdrInterval = (mfxU16)1;
    mfxEncParams.mfx.GopRefDist = (mfxU16)1;

    //取消每帧附带的sei。实际发现取消后容易花屏
//    std::vector<mfxExtBuffer*> m_InitExtParams_ENC;
//    mfxExtCodingOption* pCodingOption = new mfxExtCodingOption;
//    MSDK_ZERO_MEMORY(*pCodingOption);
//    pCodingOption->Header.BufferId = MFX_EXTBUFF_CODING_OPTION;
//    pCodingOption->Header.BufferSz = sizeof(mfxExtCodingOption);
//    pCodingOption->RefPicMarkRep = MFX_CODINGOPTION_OFF;
//    pCodingOption->NalHrdConformance = MFX_CODINGOPTION_OFF;
//
//    mfxExtCodingOption2* pCodingOption2 = new mfxExtCodingOption2;
//    MSDK_ZERO_MEMORY(*pCodingOption2);
//    pCodingOption2->Header.BufferId = MFX_EXTBUFF_CODING_OPTION2;
//    pCodingOption2->Header.BufferSz = sizeof(mfxExtCodingOption2);
//    pCodingOption2->RepeatPPS = MFX_CODINGOPTION_OFF;
//
//    m_InitExtParams_ENC.push_back(reinterpret_cast<mfxExtBuffer *>(pCodingOption));
//    m_InitExtParams_ENC.push_back(reinterpret_cast<mfxExtBuffer *>(pCodingOption2));
//
//    mfxEncParams.ExtParam    = m_InitExtParams_ENC.data();
//    mfxEncParams.NumExtParam = (mfxU16)m_InitExtParams_ENC.size();

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

    // Initialize VPP parameters
    memset(&VPPParams, 0, sizeof(VPPParams));
    // Input data V4L2
    VPPParams.vpp.In.FourCC = MFX_FOURCC_NV12;
    VPPParams.vpp.In.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    VPPParams.vpp.In.CropX = 0;
    VPPParams.vpp.In.CropY = 0;
    VPPParams.vpp.In.CropW = 1920;
    VPPParams.vpp.In.CropH = 1080;
    VPPParams.vpp.In.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    VPPParams.vpp.In.FrameRateExtN = 30;
    VPPParams.vpp.In.FrameRateExtD = 1;
    // width must be a multiple of 16
    // height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
    VPPParams.vpp.In.Width = MSDK_ALIGN16(1920);
    VPPParams.vpp.In.Height =
        (MFX_PICSTRUCT_PROGRESSIVE == VPPParams.vpp.In.PicStruct) ?
        MSDK_ALIGN16(1080) :
        MSDK_ALIGN32(1080);
    // Output data
    VPPParams.vpp.Out.FourCC = MFX_FOURCC_NV12;
    VPPParams.vpp.Out.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    VPPParams.vpp.Out.CropX = 0;
    VPPParams.vpp.Out.CropY = 0;
    VPPParams.vpp.Out.CropW = options.values.Width;
    VPPParams.vpp.Out.CropH = options.values.Height;
    VPPParams.vpp.Out.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    VPPParams.vpp.Out.FrameRateExtN = options.values.FrameRateN;
    VPPParams.vpp.Out.FrameRateExtD = options.values.FrameRateD;
    // width must be a multiple of 16
    // height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
    VPPParams.vpp.Out.Width = MSDK_ALIGN16(VPPParams.vpp.Out.CropW);
    VPPParams.vpp.Out.Height =
        (MFX_PICSTRUCT_PROGRESSIVE == VPPParams.vpp.Out.PicStruct) ?
        MSDK_ALIGN16(VPPParams.vpp.Out.CropH) :
        MSDK_ALIGN32(VPPParams.vpp.Out.CropH);

    VPPParams.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;


    //5. Create Media SDK encoder
    mfxENC = new MFXVideoENCODE(session);
    // Create Media SDK VPP component
    mfxVPP = new MFXVideoVPP(session);


    // Query number of required surfaces for encoder
    memset(&EncRequest, 0, sizeof(EncRequest));
    sts = mfxENC->QueryIOSurf(&mfxEncParams, &EncRequest);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Query number of required surfaces for VPP
    memset(&VPPRequest, 0, sizeof(mfxFrameAllocRequest) * 2);
    sts = mfxVPP->QueryIOSurf(&VPPParams, VPPRequest);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    EncRequest.Type |= MFX_MEMTYPE_FROM_VPPOUT;     // surfaces are shared between VPP output and encode input

    // Determine the required number of surfaces for VPP input and for VPP output (encoder input)
    nSurfNumVPPIn = VPPRequest[0].NumFrameSuggested;
    nSurfNumVPPOutEnc = EncRequest.NumFrameSuggested + VPPRequest[1].NumFrameSuggested;

    EncRequest.NumFrameSuggested = nSurfNumVPPOutEnc;



    //4. Allocate surfaces for VPP: In
    // - Width and height of buffer must be aligned, a multiple of 32
    // - Frame surface array keeps pointers all surface planes and general frame info
    width = (mfxU16) MSDK_ALIGN32(MAX_WIDTH);
    height = (mfxU16) MSDK_ALIGN32(MAX_HEIGHT);
    mfxU8 bitsPerPixel = 12;        // NV12 format is a 12 bits per pixel format
    mfxU32 surfaceSize = width * height * bitsPerPixel / 8;
    mfxU8* surfaceBuffersIn = (mfxU8*) new mfxU8[surfaceSize * nSurfNumVPPIn];

    pVPPSurfacesIn = new mfxFrameSurface1 *[nSurfNumVPPIn];
    MSDK_CHECK_POINTER(pVPPSurfacesIn, MFX_ERR_MEMORY_ALLOC);
    for (int i = 0; i < nSurfNumVPPIn; i++)
    {
        pVPPSurfacesIn[i] = new mfxFrameSurface1;
        memset(pVPPSurfacesIn[i], 0, sizeof(mfxFrameSurface1));
        memcpy(&(pVPPSurfacesIn[i]->Info), &(VPPParams.vpp.In), sizeof(mfxFrameInfo));
        pVPPSurfacesIn[i]->Data.Y = &surfaceBuffersIn[surfaceSize * i];
        pVPPSurfacesIn[i]->Data.U = pVPPSurfacesIn[i]->Data.Y + width * height;
        pVPPSurfacesIn[i]->Data.V = pVPPSurfacesIn[i]->Data.U + 1;
        pVPPSurfacesIn[i]->Data.Pitch = width;
        if (!1)
        {
            ClearYUVSurfaceSysMem(pVPPSurfacesIn[i], width, height);
        }
    }

    // Allocate surfaces for VPP: Out
    width = (mfxU16) MSDK_ALIGN32(MAX_WIDTH);
    height = (mfxU16) MSDK_ALIGN32(MAX_HEIGHT);
    surfaceSize = width * height * bitsPerPixel / 8;
    mfxU8* surfaceBuffersOut = (mfxU8*) new mfxU8[surfaceSize * nSurfNumVPPOutEnc];

    pVPPSurfacesOut = new mfxFrameSurface1 *[nSurfNumVPPOutEnc];
    MSDK_CHECK_POINTER(pVPPSurfacesOut, MFX_ERR_MEMORY_ALLOC);
    for (int i = 0; i < nSurfNumVPPOutEnc; i++)
    {
        pVPPSurfacesOut[i] = new mfxFrameSurface1;
        memset(pVPPSurfacesOut[i], 0, sizeof(mfxFrameSurface1));
        memcpy(&(pVPPSurfacesOut[i]->Info), &(VPPParams.vpp.Out), sizeof(mfxFrameInfo));
        pVPPSurfacesOut[i]->Data.Y = &surfaceBuffersOut[surfaceSize * i];
        pVPPSurfacesOut[i]->Data.U = pVPPSurfacesOut[i]->Data.Y + width * height;
        pVPPSurfacesOut[i]->Data.V = pVPPSurfacesOut[i]->Data.U + 1;
        pVPPSurfacesOut[i]->Data.Pitch = width;
    }

    // Initialize extended buffer for frame processing
    // - Denoise           VPP denoise filter
    // - mfxExtVPPDoUse:   Define the processing algorithm to be used
    // - mfxExtVPPDenoise: Denoise configuration
    // - mfxExtBuffer:     Add extended buffers to VPP parameter configuration
    mfxExtVPPDoUse extDoUse;
    memset(&extDoUse, 0, sizeof(extDoUse));
    mfxU32 tabDoUseAlg[1];
    extDoUse.Header.BufferId = MFX_EXTBUFF_VPP_DOUSE;
    extDoUse.Header.BufferSz = sizeof(mfxExtVPPDoUse);
    extDoUse.NumAlg = 1;
    extDoUse.AlgList = tabDoUseAlg;
    tabDoUseAlg[0] = MFX_EXTBUFF_VPP_DENOISE;

    mfxExtVPPDenoise denoiseConfig;
    memset(&denoiseConfig, 0, sizeof(denoiseConfig));
    denoiseConfig.Header.BufferId = MFX_EXTBUFF_VPP_DENOISE;
    denoiseConfig.Header.BufferSz = sizeof(mfxExtVPPDenoise);
    denoiseConfig.DenoiseFactor = 100;        // can be 1-100

    mfxExtBuffer* ExtBuffer[2];
    ExtBuffer[0] = (mfxExtBuffer*) &extDoUse;
    ExtBuffer[1] = (mfxExtBuffer*) &denoiseConfig;
    VPPParams.NumExtParam = 2;
    VPPParams.ExtParam = (mfxExtBuffer**) &ExtBuffer[0];



    //8 Initialize the Media SDK encoder
    sts = mfxENC->Init(&mfxEncParams);
    MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
    //5. Initialize Media SDK VPP
    sts = mfxVPP->Init(&VPPParams);
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
    int nSurfIdxIn = 0, nSurfIdxOut = 0;
    static mfxU32 nFrame = 0;
    mfxSyncPoint syncp;
//    nEncSurfIdx = GetFreeSurfaceIndex(pEncSurfaces, nEncSurfNum);   // Find free frame surface
//    MSDK_CHECK_ERROR(MFX_ERR_NOT_FOUND, nEncSurfIdx, MFX_ERR_MEMORY_ALLOC);
//
//    sts = LoadRawFrameFromV4l2(pEncSurfaces[nEncSurfIdx], buffer);
//
//    MSDK_RETURN_ON_ERROR(sts);



    nSurfIdxIn = GetFreeSurfaceIndex(pVPPSurfacesIn, nSurfNumVPPIn);        // Find free input frame surface
    pVPPSurfacesIn[nSurfIdxIn]->Data.TimeStamp = nFrame*90000/VPPParams.vpp.Out.FrameRateExtN;

    sts = LoadRawFrameFromV4l2(pVPPSurfacesIn[nSurfIdxIn], buffer);
    MSDK_RETURN_ON_ERROR(sts);

    nSurfIdxOut = GetFreeSurfaceIndex(pVPPSurfacesOut, nSurfNumVPPOutEnc);     // Find free output frame surface
    MSDK_CHECK_ERROR(MFX_ERR_NOT_FOUND, nSurfIdxOut, MFX_ERR_MEMORY_ALLOC);

    for (;;)
    {
        // Process a frame asychronously (returns immediately)
        sts = mfxVPP->RunFrameVPPAsync(pVPPSurfacesIn[nSurfIdxIn], pVPPSurfacesOut[nSurfIdxOut], NULL, &syncp);

        //skip a frame

        if(MFX_ERR_MORE_DATA==sts)
        {
            nSurfIdxIn = GetFreeSurfaceIndex(pVPPSurfacesIn,nSurfNumVPPIn);
            MSDK_CHECK_ERROR(MFX_ERR_NOT_FOUND, nSurfIdxIn,MFX_ERR_MEMORY_ALLOC);
            pVPPSurfacesIn[nSurfIdxIn]->Data.TimeStamp=nFrame*90000/VPPParams.vpp.Out.FrameRateExtN;
            return sts;
        }

        //add (often duplicate) a frame
        if(MFX_ERR_MORE_SURFACE==sts)
        {
            //todo
        }


        if (MFX_WRN_DEVICE_BUSY == sts)
        {
            MSDK_SLEEP(1);  // Wait if device is busy, then repeat the same call
        }
        else
            break;

    }

    for (;;)
    {
        // Encode a frame asychronously (returns immediately)
        if(insertIDR)
        {
            mfxEncodeCtrl ctrl;
            memset(&ctrl,0,sizeof(ctrl));
            ctrl.FrameType=MFX_FRAMETYPE_IDR;
            sts = mfxENC->EncodeFrameAsync(&ctrl, pVPPSurfacesOut[nSurfIdxOut], &mfxBS, &syncp);
            printf("IDR frame insert success\n");
            insertIDR = false;
        }
        else
        {
            sts = mfxENC->EncodeFrameAsync(NULL, pVPPSurfacesOut[nSurfIdxOut], &mfxBS, &syncp);
        }

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
