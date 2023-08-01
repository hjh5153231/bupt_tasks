#pragma once
#include <string>
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <thread>
#include "al.h"
#include "alc.h"
extern "C"
{
#include "SDL.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
};
#define MAX_AUDIO_FRME_SIZE 192000
#define NUMBUFFERS (3)
using namespace std;
//定义SDL自定义事件
class AudioPlayer{
public:
    int thread_exit;
    int thread_pause;
    double audio_clock;//存储当前视频播放时刻
    bool isEnd=false;
    ALuint source;
    //ffmpeg
    AVFormatContext* fmtCtx=NULL;
    int ret;//保存函数返回的tag值
    AudioPlayer();
    int init(const char* filepath);
    int playfile();
    void replay();
    void changeVolume(float volume);
    void toSeek(int seek);
private:
    SwrContext* swr = swr_alloc();//重采样
    AVCodecParameters* avCodecPara=NULL;
    AVCodecContext* codecCtx = NULL;
    AVFrame* frame = av_frame_alloc();//解压缩数据
    int audioindex = -1;
    ALuint buffers[NUMBUFFERS];//音频缓冲区
    void initopenal(ALuint source);
    int readPCMFrame(ALuint & bufferID);
    int readPCMFrame();
};