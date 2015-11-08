
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MetaData.h>

#include <gui/GLConsumer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

//#include <SoftwareRenderer.h>

#include "SoftwareRenderer.h"

using namespace android;

typedef struct tagBitmapInfo{
    unsigned int width;
    unsigned int height;
    unsigned int bitcount;
    unsigned char * buf;
}BitmapInfo;

typedef unsigned short WORD;
typedef unsigned int DWORD;

//文件信息头
typedef struct tagBITMAPFILEHEADER{
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
}BITMAPFILEHEADER;

//位图信息头
typedef struct tagBITMAPINFOHEADER{
    DWORD   biSize;
    DWORD   biWidth;
    DWORD   biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    DWORD   biXPelsPerMeter;
    DWORD   biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
}BITMAPINFOHEADER;


BitmapInfo * readbmp(const char * filename)
{
    FILE * fp = fopen(filename, "rb");
    if(!fp)
    {
        return 0;
    }

    char szFlag[2];
    fread(szFlag, 1, 2, fp);
    if(szFlag[0] != 'B' || szFlag[1] != 'M')
    {
        fclose(fp);
        return 0;
    }

    BITMAPFILEHEADER bmpFileHeader;
    fread(&bmpFileHeader, 1, sizeof(BITMAPFILEHEADER), fp);

    BITMAPINFOHEADER bmpInfoHeader;
    fread(&bmpInfoHeader, 1, sizeof(BITMAPINFOHEADER), fp);

    BitmapInfo * pBmp = new BitmapInfo;
    pBmp->width = bmpInfoHeader.biWidth;
    pBmp->height = bmpInfoHeader.biHeight;
    pBmp->bitcount = bmpInfoHeader.biBitCount;
printf("w = %d, h = %d, bit = %d\n", pBmp->width, pBmp->height, pBmp->bitcount);
    int len = pBmp->width * pBmp->height * pBmp->bitcount / 8;
    pBmp->buf = new unsigned char[len];

    //fread(pBmp->buf, 1, len, fp);
    int pos = 0;
    int bytspp = bmpInfoHeader.biBitCount / 8;
    for (int i = pBmp->height - 1; i >= 0; --i)
    {
        for (int j = 0; j < pBmp->width; ++j)
        {
            pos = i * pBmp->width * bytspp + j * bytspp;
            fread(pBmp->buf + pos, 1, bytspp, fp);
        }
    }

    fclose(fp);

    return pBmp;
}

int main(int argc, char **argv) {
    SoftwareRenderer * mSoftRenderer = NULL;
    sp<ANativeWindow> gSurface;
    sp<SurfaceComposerClient> composerClient;
    sp<SurfaceControl> control;

    composerClient = new SurfaceComposerClient;
    CHECK_EQ(composerClient->initCheck(), (status_t)OK);

    control = composerClient->createSurface(
            String8("A Surface"),
            1280,
            720,
            PIXEL_FORMAT_RGBX_8888,
            0);

    CHECK(control != NULL);
    CHECK(control->isValid());

    SurfaceComposerClient::openGlobalTransaction();
    CHECK_EQ(control->setLayer(INT_MAX), (status_t)OK);
    CHECK_EQ(control->show(), (status_t)OK);
    control->setAlpha(1);
    SurfaceComposerClient::closeGlobalTransaction();

    gSurface = control->getSurface();
    CHECK(gSurface != NULL);

    CHECK_EQ((status_t)OK,
             native_window_api_connect(
                 gSurface.get(), NATIVE_WINDOW_API_MEDIA));

    //////////////////////////////

    sp<MetaData> meta = new MetaData;
    meta->setInt32(kKeyWidth, 1920);
    meta->setInt32(kKeyHeight, 1080);
    meta->setInt32(kKeyColorFormat, OMX_COLOR_Format24bitRGB888);//OMX_TI_COLOR_FormatYUV420PackedSemiPlanar);

    //true 顔色显示正常，false不正常，详情查阅SoftwareRenderer.cpp
    mSoftRenderer = new SoftwareRenderer(gSurface, meta, true);

    BitmapInfo * pBitmap = readbmp("/mnt/sdcard/test.bmp");
    
    int i = 5;
    while(--i >= 0)
    {
        mSoftRenderer->render(pBitmap->buf, pBitmap->width * pBitmap->height * pBitmap->bitcount / 8, NULL);
        printf("i = %d\n", i);
        usleep(1000 * 1000);
    }
   

    CHECK_EQ((status_t)OK,
                 native_window_api_disconnect(
                     gSurface.get(), NATIVE_WINDOW_API_MEDIA));

    gSurface.clear();

    composerClient->dispose();

    delete mSoftRenderer;
    mSoftRenderer = NULL;

    delete []pBitmap->buf;
    delete pBitmap;
    //fclose(fp);

    return 0;
}

