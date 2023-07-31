#include <iostream>
extern "C"{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h" 
    #include "SDL.h"  
}
#include <Windows.h>
//自定义SDL事件
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

using namespace std;
//定义全局变量
int thread_exit=0;
int thread_pause=0;
int restart_video=0;

//视频线程函数，刷新屏幕
int refreshScreen(void *opaque){
    thread_exit=0;
    thread_pause=0;
    //不退出就一直以固定延迟push一个自定义的event来使主线程刷新屏幕
    while(!thread_exit){
        if(!thread_pause){//如果暂停了，那么就不要刷新屏幕画面
            SDL_Event event;
            event.type=SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        SDL_Delay(40);
    }
    //线程退出，push自定义退出事件来使主线程退出
    thread_exit=0;
    thread_pause=0;
    SDL_Event event;
    event.type=SFM_BREAK_EVENT;
    SDL_PushEvent(&event);
    return 0;
}

#undef main
int main(){
    system("chcp 65001");
    cout<<"请输入要解码并播放的mp4文件url"<<endl;
    string s_filepath;
    getline(cin,s_filepath);
    const char* filepath=s_filepath.c_str();
    //ffmpeg变量声明
    AVFormatContext* fmtCtx=NULL;
    AVCodecParameters* avCodecPara=NULL;
    AVCodecContext* codecCtx = NULL;
    AVCodec* codec = NULL;
    AVPacket* pkt = NULL;
    AVFrame* yuvFrame = av_frame_alloc();
    int ret=0;//保存函数返回的tag值
    int videoStreamIndex=-1;

    //SDL变量声明
	SDL_Window *screen; 
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;

    fmtCtx=avformat_alloc_context();
    if((ret=avformat_open_input(&fmtCtx,filepath,NULL,NULL))!=0){
        cout<<"打开视频文件失败,error code:"<<ret<<endl;
        return -1;
    }
    if((ret=avformat_find_stream_info(fmtCtx,NULL))<0){//给fmtCtx补充信息
        cout<<"找不到视频信息"<<endl;
        return -1;
    }
    for(int i=0;i<fmtCtx->nb_streams;i++){//找到视频流
        if(fmtCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
            videoStreamIndex=i;
            break;
        }
    }
    if(videoStreamIndex==-1){
        cout<<"找不到视频流"<<endl;
        return -1;
    }
    // av_dump_format(fmtCtx, videoStreamIndex, filepath, 0);
    avCodecPara=fmtCtx->streams[videoStreamIndex]->codecpar;
    codec=avcodec_find_decoder(avCodecPara->codec_id);
    if(codec==NULL){
        cout<<"找不到解码器"<<endl;
        return -1;
    }
    codecCtx=avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx,avCodecPara);
    if(codecCtx==NULL){
        cout<<"分配解码器context失败"<<endl;
        return -1;
    }
    if((ret=avcodec_open2(codecCtx,codec,NULL))<0){
        cout<<"打不开解码器"<<endl;
        return -1;
    }
    int w = codecCtx->width;//视频宽度
    int h = codecCtx->height;//视频高度

    //SDL-init
    screen=SDL_CreateWindow("task3.1.b window",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,w,h,SDL_WINDOW_OPENGL);
    if(!screen){
        cout<<"SDL创建窗口失败"<<endl;
        return -1;
    }
    HWND hwnd = GetActiveWindow();
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    sdlRenderer=SDL_CreateRenderer(screen,-1,0);
    sdlTexture=SDL_CreateTexture(sdlRenderer,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,w,h);
    sdlRect.x=0;
    sdlRect.y=0;
    sdlRect.w=w;
    sdlRect.h=h;
    SDL_Thread *video_tid;
    SDL_Event event;
    video_tid=SDL_CreateThread(refreshScreen,NULL,NULL);

    //packet to yuv
    pkt = av_packet_alloc();
    av_new_packet(pkt, w * h); //分配一个packet
    bool isReplay=false;//控制视频重播
    bool isEnd=false;//标记视频是否播放结束
    //主线程一直循环，对不同的事件作不同的处理
    while(true){
        SDL_WaitEvent(&event);
        if(event.type==SFM_REFRESH_EVENT){//若当前需要刷新则从fp中读取一帧数据并刷新屏幕
            while(true){
                if(av_read_frame(fmtCtx, pkt)<0){//若yuv播放结束，则利用事件阻塞暂停主线程，直到按F1进行重播
                    isEnd=true;
                    SDL_Event event1;
                    SDL_WaitEvent(&event);
                    if(event.type==SDL_KEYDOWN){
                        if(event.key.keysym.sym==SDLK_F1){
                            isReplay=true;
                        }
                    }
                    // thread_exit=1;
                }
                if(pkt->stream_index==videoStreamIndex){
                    break;
                }
            }
            if(isReplay){//如果结束后按F1触发重播，则将视频调整到开头
                av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD); // 将视频重置到开头
                isReplay=false;
            }
            if(isEnd){//如果视频已经结束，那么就不要再更新画面
                av_packet_unref(pkt);
                isEnd=false;
                continue;
            }
            if(avcodec_send_packet(codecCtx,pkt)==0){//解码，播放
                while(avcodec_receive_frame(codecCtx,yuvFrame)==0){
                    SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
                    yuvFrame->data[0], yuvFrame->linesize[0],
                    yuvFrame->data[1], yuvFrame->linesize[1],
                    yuvFrame->data[2], yuvFrame->linesize[2]);
                    SDL_RenderClear(sdlRenderer);
                    SDL_RenderCopy(sdlRenderer,sdlTexture,NULL,&sdlRect);
                    SDL_RenderPresent(sdlRenderer);
                }
            }
            av_packet_unref(pkt);
        }
        else if(event.type==SDL_KEYDOWN){
            if(event.key.keysym.sym==SDLK_SPACE){//控制暂停
                thread_pause=!thread_pause;
                cout<<"out space"<<endl;
            }
            else if(event.key.keysym.sym==SDLK_F1){//重播
                restart_video=1;
                cout<<"out f1"<<endl;
            }
        }
        else if(event.type==SDL_QUIT){//结束
            thread_exit=1;
        }
        else if(event.type==SFM_BREAK_EVENT){//结束
            break;
        }
        if(restart_video){
            cout<<"restart"<<endl;
            av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD); // 将视频重置到开头
            cout<<"restart success"<<endl;
            restart_video=0;
        }
    }
    //释放资源
    SDL_Quit();
    av_packet_free(&pkt);
    avcodec_close(codecCtx);
    avformat_close_input(&fmtCtx);
    avcodec_parameters_free(&avCodecPara);
    av_frame_free(&yuvFrame);
    return 0;
}
