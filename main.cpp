#include <iostream>

#include "videoPipeline.h"
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



    msgServer msgSer(options);
    videoPipeline vp(options);
    vp.addEncodeAsOb(&msgSer);
    msgSer.addSubscribe("general",(observer*)&vp);


    vp.start();
    msgSer.start();

    msgSer.join();

    Release();
    return 0;
}
