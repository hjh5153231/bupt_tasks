#include <iostream>
#include <stdio.h>
#include "al.h"
#include "alc.h"
#include <conio.h>
#include <Windows.h>
#include <thread>
#include "VideoThread.h"
extern "C"
{
#include "SDL.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
};
//定义SDL自定义事件
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)
#define SFM_REPLAY_EVENT  (SDL_USEREVENT + 3)
//视频线程函数的全局变量
bool isReplay=false;
int fps;//帧率
int videoOffset;//标记是否需要调整视频进度来进行同步

int refreshScreen(int &thread_exit,int &thread_pause){//
    thread_exit=0;
    thread_pause=0;
    while(!thread_exit){
        if(!thread_pause){
            SDL_Event event;
            event.type=SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
		if(videoOffset==1){//快进
			SDL_Delay(600/fps);
		}
		else if(videoOffset==0){//正常
			SDL_Delay(1000/fps);
		}
		else{
			SDL_Delay(1200/fps);//慢放
		}
    }
    thread_exit=0;
    thread_pause=0;
    SDL_Event event;
    event.type=SFM_BREAK_EVENT;
    SDL_PushEvent(&event);
    return 0;
}
int sdlRender(string s_filepath,double &audio_clock,int &thread_exit,int &thread_pause,bool &volumeChanged,float &volume,bool &isEnd,int &restart_video,int &audioSeek){
	const char* filepath=s_filepath.c_str();
    //ffmpeg变量定义
	AVFormatContext* fmtCtx=NULL;
    AVCodecParameters* avCodecPara=NULL;
    AVCodecContext* codecCtx = NULL;
    AVCodec* codec = NULL;
    AVPacket* pkt = NULL;
    AVFrame* yuvFrame = av_frame_alloc();
    int ret=0;//保存函数返回的tag值
    //视频音频流index
    int videoStreamIndex=-1;
    int audioStreamIndex=-1;
    double delay;

    //SDL变量声明
	SDL_Window *screen; 
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
    //分配ffmpeg变量，初始化
    fmtCtx=avformat_alloc_context();
    if((ret=avformat_open_input(&fmtCtx,filepath,NULL,NULL))!=0){
        cout<<"打开视频文件失败,error code:"<<ret<<endl;
        return -1;
    }
    if((ret=avformat_find_stream_info(fmtCtx,NULL))<0){
        cout<<"找不到视频信息"<<endl;
        return -1;
    }
    for(int i=0;i<fmtCtx->nb_streams;i++){
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
    //使窗口在最前方
    HWND hwnd = GetActiveWindow();
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    //SDL变量初始化
    sdlRenderer=SDL_CreateRenderer(screen,-1,0);
    sdlTexture=SDL_CreateTexture(sdlRenderer,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,w,h);
    sdlRect.x=0;
    sdlRect.y=0;
    sdlRect.w=w;
    sdlRect.h=h;
    //利用SDL事件机制控制视频刷新
    SDL_Thread *video_tid;
    SDL_Event event;
	fps=(int)av_q2d(fmtCtx->streams[videoStreamIndex]->avg_frame_rate)+1;
    std::thread refreshThread(refreshScreen,ref(thread_exit), ref(thread_pause));//创建线程，参数为全局变量的引用
    refreshThread.detach();//线程后台运行
    //SDL-init done


    int init=0;
    //packet to yuv
    pkt = av_packet_alloc();
    av_new_packet(pkt, w * h); //分配packet
	double video_clock=0;//存储当前视频播放时刻
    while(true){
        SDL_WaitEvent(&event);
        if(event.type==SFM_REFRESH_EVENT){
            while(true){//在视频没结束的情况下提取下一个packet
                if(av_read_frame(fmtCtx, pkt)<0){
                    SDL_Event event1;
                    SDL_WaitEvent(&event1);
                    if(event1.type==SDL_KEYDOWN){
                        if(event1.key.keysym.sym==SDLK_F1){
                            av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
                            restart_video=2;//设定restart_video为2来使视频和音频一起重置，不会造成线程引用冲突
                            av_packet_unref(pkt);
                            continue;
                        }
                        else{//别的事件不管
                            continue;
                        }
                    }
                    else{//别的事件不管
                        continue;
                    }
                }
                if(pkt->stream_index==videoStreamIndex){
                    break;
                }
            }
            if(avcodec_send_packet(codecCtx,pkt)==0){//解码，读取下一帧yuv
                while(avcodec_receive_frame(codecCtx,yuvFrame)==0){
					video_clock=yuvFrame->pts*av_q2d(fmtCtx->streams[videoStreamIndex]->time_base);//播放前更新视频时钟
					delay=audio_clock-video_clock;//控制差值在一定范围内，音视频动态同步
					cout<<"音视频时间差值:"<<delay<<endl;
                    if(init=0 && delay>0.03){
                        av_seek_frame(fmtCtx, videoStreamIndex, (audio_clock+delay)/av_q2d(fmtCtx->streams[videoStreamIndex]->time_base), AVSEEK_FLAG_BACKWARD);//初始调整
                        init++;
                    }
					if(delay>0.03){
						videoOffset=1;
					}
					else if(delay<0){//由于音频的时钟受缓存大小影响，因此不设定为-0.03
						videoOffset=-1;
					}
					else{
						videoOffset=0;
					}
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
            if(event.key.keysym.sym==SDLK_SPACE){//空格控制暂停
                thread_pause=!thread_pause;
                cout<<"out space"<<endl;
            }
            else if(event.key.keysym.sym==SDLK_F1){//F1控制重放
                restart_video=2;
                cout<<"out f1"<<endl;
            }
			else if(event.key.keysym.sym==SDLK_UP){//控制音量上升
				volumeChanged=true;
				volume += 0.1f;
				if (volume > 1.0f) {
                    volume = 1.0f;
                }
			}
			else if(event.key.keysym.sym==SDLK_DOWN){//音量下降
				volumeChanged=true;
				volume -= 0.1f;
				if (volume < 0.0f) {
                    volume = 0.0f;
                }
			}
            else if(event.key.keysym.sym==SDLK_LEFT){//快退30秒
                av_seek_frame(fmtCtx, videoStreamIndex, (audio_clock-30.0+delay*(double)fps)/av_q2d(fmtCtx->streams[videoStreamIndex]->time_base), AVSEEK_FLAG_BACKWARD);//通过时钟计算对应时间戳
                avcodec_flush_buffers(codecCtx);
                audioSeek=-1;
            }
            else if(event.key.keysym.sym==SDLK_RIGHT){//快进30秒
                av_seek_frame(fmtCtx, videoStreamIndex, (audio_clock+30.0+delay*(double)fps)/av_q2d(fmtCtx->streams[videoStreamIndex]->time_base), AVSEEK_FLAG_BACKWARD);
                avcodec_flush_buffers(codecCtx);
                audioSeek=1;
            }
        }
        else if(event.type==SDL_QUIT){
            thread_exit=1;
        }
        else if(event.type==SFM_BREAK_EVENT){
            break;
        }
        if(restart_video==1){//等音乐重置完，再重置视频
            cout<<"restart"<<endl;
            av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD); // 将视频重置到开头
            cout<<"restart success"<<endl;
            restart_video--;
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