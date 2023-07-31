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
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)
#define SFM_REPLAY_EVENT  (SDL_USEREVENT + 3)

bool isReplay=false;
int fps;
int videoOffset;

int refreshScreen(int &thread_exit,int &thread_pause){
    thread_exit=0;
    thread_pause=0;
    while(!thread_exit){
        if(!thread_pause){
            SDL_Event event;
            event.type=SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
		if(videoOffset==1){
			SDL_Delay(600/fps);
		}
		else if(videoOffset==0){
			SDL_Delay(1000/fps);
		}
		else{
			SDL_Delay(1200/fps);
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
	AVFormatContext* fmtCtx=NULL;
    AVCodecParameters* avCodecPara=NULL;
    AVCodecContext* codecCtx = NULL;
    AVCodec* codec = NULL;
    AVPacket* pkt = NULL;
    AVFrame* yuvFrame = av_frame_alloc();
    int ret=0;//保存函数返回的tag值
    int videoStreamIndex=-1;
    int audioStreamIndex=-1;
    double delay;

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
	fps=(int)av_q2d(fmtCtx->streams[videoStreamIndex]->avg_frame_rate)+1;
    std::thread refreshThread(refreshScreen,ref(thread_exit), ref(thread_pause));
    refreshThread.detach();
    //SDL-init done


    int init=0;
    //packet to yuv
    pkt = av_packet_alloc();
    av_new_packet(pkt, w * h); 
	double video_clock=0;
    while(true){
        SDL_WaitEvent(&event);
        if(event.type==SFM_REFRESH_EVENT){
            while(true){
                if(av_read_frame(fmtCtx, pkt)<0){
                    SDL_Event event1;
                    SDL_WaitEvent(&event1);
                    if(event1.type==SDL_KEYDOWN){
                        if(event1.key.keysym.sym==SDLK_F1){
                            av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
                            restart_video=2;
                            av_packet_unref(pkt);
                            continue;
                        }
                        else{
                            continue;
                        }
                    }
                    else{
                        continue;
                    }
                }
                if(pkt->stream_index==videoStreamIndex){
                    break;
                }
            }
            if(avcodec_send_packet(codecCtx,pkt)==0){
                while(avcodec_receive_frame(codecCtx,yuvFrame)==0){
					video_clock=yuvFrame->pts*av_q2d(fmtCtx->streams[videoStreamIndex]->time_base);
					delay=audio_clock-video_clock;
					cout<<"音视频时间差值:"<<delay<<endl;
                    if(init=0 && delay>0.03){
                        av_seek_frame(fmtCtx, videoStreamIndex, (audio_clock+delay)/av_q2d(fmtCtx->streams[videoStreamIndex]->time_base), AVSEEK_FLAG_BACKWARD);
                        init++;
                    }
					if(delay>0.03){
						videoOffset=1;
					}
					else if(delay<0){
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
            if(event.key.keysym.sym==SDLK_SPACE){
                thread_pause=!thread_pause;
                cout<<"out space"<<endl;
            }
            else if(event.key.keysym.sym==SDLK_F1){
                restart_video=2;
                cout<<"out f1"<<endl;
            }
			else if(event.key.keysym.sym==SDLK_UP){
				volumeChanged=true;
				volume += 0.1f;
				if (volume > 1.0f) {
                    volume = 1.0f;
                }
			}
			else if(event.key.keysym.sym==SDLK_DOWN){
				volumeChanged=true;
				volume -= 0.1f;
				if (volume < 0.0f) {
                    volume = 0.0f;
                }
			}
            else if(event.key.keysym.sym==SDLK_LEFT){
                av_seek_frame(fmtCtx, videoStreamIndex, (audio_clock-30.0+delay*(double)fps)/av_q2d(fmtCtx->streams[videoStreamIndex]->time_base), AVSEEK_FLAG_BACKWARD);
                avcodec_flush_buffers(codecCtx);
                audioSeek=-1;
            }
            else if(event.key.keysym.sym==SDLK_RIGHT){
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
        if(restart_video==1){
            cout<<"restart"<<endl;
            av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD); // 将视频重置到开头
            cout<<"restart success"<<endl;
            restart_video--;
        }
    }
    SDL_Quit();
    av_packet_free(&pkt);
    avcodec_close(codecCtx);
    avformat_close_input(&fmtCtx);
    avcodec_parameters_free(&avCodecPara);
    av_frame_free(&yuvFrame);
    return 0;
}