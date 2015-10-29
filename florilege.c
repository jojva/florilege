#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include <SDL.h>
#include <SDL_thread.h>

//#define DUMP_FRAMES

typedef struct Context
{
    // Generic stuff
    char* videoName;
    // FFmpeg
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    // SDL
    SDL_Surface *screen;
} Context;

#ifdef DUMP_FRAMES
void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
    FILE *pFile;
    char szFilename[32];
    int y;

    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if (pFile == NULL)
        return;
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);
    for (y = 0; y < height; y++)
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
    fclose(pFile);
}
#endif

void DecodePacket(Context *pContext, AVFrame *pFrame, AVFrame *pFrameRGB, AVPacket *pkt)
{
    AVCodecContext *pCodecCtx = pContext->pCodecCtx;
    int frameFinished;
    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, pkt);
#ifdef DUMP_FRAMES
    if (frameFinished)
    {
        static int i = 0;
        struct SwsContext *ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                                pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,
                                                SWS_FAST_BILINEAR, NULL, NULL, NULL);
        sws_scale(ctx, (const uint8_t * const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
                  pFrameRGB->data, pFrameRGB->linesize);
        SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);
        i++;
    }
#else
    if(frameFinished) {
        SDL_Overlay *bmp = NULL;
        bmp = SDL_CreateYUVOverlay(pCodecCtx->width, pCodecCtx->height, SDL_YV12_OVERLAY, pContext->screen);
        struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                                    pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                                    SWS_BILINEAR, NULL, NULL, NULL);
        SDL_LockYUVOverlay(bmp);
        AVPicture pict;
        pict.data[0] = bmp->pixels[0];
        pict.data[1] = bmp->pixels[2];
        pict.data[2] = bmp->pixels[1];
        pict.linesize[0] = bmp->pitches[0];
        pict.linesize[1] = bmp->pitches[2];
        pict.linesize[2] = bmp->pitches[1];
        // Convert the image into YUV format that SDL uses
        sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
                  pFrame->linesize, 0, pCodecCtx->height,
                  pict.data, pict.linesize);
        SDL_UnlockYUVOverlay(bmp);
        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = pCodecCtx->width;
        rect.h = pCodecCtx->height;
        SDL_DisplayYUVOverlay(bmp, &rect);
    }
#endif
}

int InitSDL(Context* pContext)
{
    SDL_Surface *screen;
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
    {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        return 1;
    }
    screen = SDL_SetVideoMode(pContext->pCodecCtx->width, pContext->pCodecCtx->height, 0, 0);
    if(!screen) {
        fprintf(stderr, "SDL: could not set video mode - exiting\n");
        return 1;
    }
    pContext->screen = screen;
    return 0;
}

int InitFFmpeg(Context *pContext)
{
    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pCodecCtx = NULL;
    int videoStream;
    int i;
    AVCodec *pCodec;
    av_log_set_level(AV_LOG_TRACE);
    avformat_network_init();
    av_register_all();
    if (avformat_open_input(&pFormatCtx, pContext->videoName, NULL, NULL) != 0)
    {
        fprintf(stderr, "Couldn't open file\n");
        return 1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        fprintf(stderr, "Couldn't find stream information\n");
        return 1;
    }
    videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }
    if (videoStream < 0)
    {
        fprintf(stderr, "Didn't find a video stream\n");
        return 1;
    }
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec == NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        return 1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        return 1;
    }
    pContext->pFormatCtx = pFormatCtx;
    pContext->pCodecCtx = pCodecCtx;
    return 0;
}

int Play(Context *pContext)
{
    AVFormatContext *pFormatCtx = pContext->pFormatCtx;
    AVCodecContext *pCodecCtx = pContext->pCodecCtx;
    AVPacket packet;
    int numBytes;
    uint8_t *buffer;
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();
    numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    for(;;) {
        if(av_read_frame(pFormatCtx, &packet) < 0)
        {
            fprintf(stderr, "Couldn't read frame\n");
            return 1;
        }
        DecodePacket(pContext, pFrame, pFrameRGB, &packet);
        av_free_packet(&packet);
    }
}

int main(int argc, char *argv[])
{
    Context context;
    Context *pContext = &context;
    pContext->pCodecCtx = NULL;
    pContext->pFormatCtx = NULL;
    pContext->screen = NULL;
    pContext->videoName = NULL;
    if(argc < 2)
    {
        fprintf(stderr, "Please provide a movie file\n");
        return 1;
    }
    pContext->videoName = argv[1];
    if(InitFFmpeg(pContext) != 0)
    {
        fprintf(stderr, "Error initializing FFmpeg\n");
        return 1;
    }
    if(InitSDL(pContext) != 0)
    {
        fprintf(stderr, "Error initializing SDL\n");
        return 1;
    }
    if(Play(pContext) != 0)
    {
        fprintf(stderr, "Error playing video\n");
        return 1;
    }
    return 0;
}
