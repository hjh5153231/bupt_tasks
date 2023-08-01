#pragma once
#include <string>
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <thread>
extern "C"
{
#include "SDL.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
};
using namespace std;
//定义SDL自定义事件
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)
class VideoPlayer{
public:
    int thread_exit;
    int thread_pause;
    double video_clock;//存储当前视频播放时刻
    bool isEnd;
    //ffmpeg
    AVFormatContext* fmtCtx=NULL;
    int ret;//保存函数返回的tag值
    int fps;//帧率
    double speed;
    VideoPlayer();
    int init(const char* filepath);
    int playfile(bool &replay,bool &volumeChanged,float &volume,int &seek);
    void toSeek(int seek,double audio_clock);
private:
    AVFrame* yuvFrame = av_frame_alloc();
    AVCodecContext* codecCtx = NULL;
    AVPacket* pkt = NULL;
    int videoStreamIndex=-1;
    void checkProcess(double audio_clock);
    int refreshScreen();
};