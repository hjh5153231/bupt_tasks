#include "VideoPlayer.h"


using namespace std;
VideoPlayer::VideoPlayer(){
    AVFormatContext* fmtctx=NULL;
    ret=0;//保存函数返回的tag值
    speed=1;
    video_clock=0;//存储当前视频播放时刻
}
int VideoPlayer::init(const char* filepath){
    fmtCtx=avformat_alloc_context();
    if((ret=avformat_open_input(&fmtCtx,filepath,NULL,NULL))!=0){
        cout<<"打开视频文件失败,error code:"<<ret<<endl;
        return -1;
    }
    if((ret=avformat_find_stream_info(fmtCtx,NULL))<0){//给fmtCtx补充信息
        cout<<"找不到视频信息"<<endl;
        return -1;
    }
    return 0;
}
int VideoPlayer::refreshScreen(){
    thread_exit=0;
    thread_pause=0;
    while(!thread_exit){
        if(!thread_pause){
            SDL_Event event;
            event.type=SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
		SDL_Delay(1000*speed/fps);
    }
    thread_exit=0;
    thread_pause=0;
    SDL_Event event;
    event.type=SFM_BREAK_EVENT;
    SDL_PushEvent(&event);
    return 0;
}

void VideoPlayer::checkProcess(double audio_clock){//在快进和快退之后将视频进度与音频进度对齐
    bool isChecked=0;
    double delay;
    while(!isChecked){//一直读取视频帧到视频和音频对齐
        while(true){
            if(av_read_frame(fmtCtx, pkt)<0){
                return;
            }
            if(pkt->stream_index==videoStreamIndex){
                break;
            }
        }
        if(avcodec_send_packet(codecCtx,pkt)==0){
            while(avcodec_receive_frame(codecCtx,yuvFrame)==0){
                video_clock=yuvFrame->pts*av_q2d(fmtCtx->streams[videoStreamIndex]->time_base);
                delay=audio_clock-video_clock;
                if(delay<=0.03){
                    av_packet_unref(pkt);
                    isChecked=true;
                }
            }
        }
        av_packet_unref(pkt);
    }
}

void VideoPlayer::toSeek(int seek,double audio_clock){
    av_seek_frame(fmtCtx, videoStreamIndex, (audio_clock)/av_q2d(fmtCtx->streams[videoStreamIndex]->time_base), AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(codecCtx);
    checkProcess(audio_clock);
}

int VideoPlayer::playfile(bool &replay,bool &volumeChanged,float &volume,int &seek){
    AVCodecParameters* avCodecPara=NULL;
    AVCodec* codec = NULL;
    int ret=0;//保存函数返回的tag值

    //SDL变量声明
	SDL_Window *screen; 
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
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
    fps=(int)av_q2d(fmtCtx->streams[videoStreamIndex]->avg_frame_rate)+1;
    SDL_Event event;
    std::thread refreshThread([this] { refreshScreen(); });
    refreshThread.detach();

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
                    break;
                }
                if(pkt->stream_index==videoStreamIndex){
                    break;
                }
            }
            if(isEnd){//如果视频已经结束，那么就不要再更新画面
                av_packet_unref(pkt);
                continue;
            }
            if(avcodec_send_packet(codecCtx,pkt)==0){//解码，播放
                while(avcodec_receive_frame(codecCtx,yuvFrame)==0){
                    video_clock=yuvFrame->pts*av_q2d(fmtCtx->streams[videoStreamIndex]->time_base);//播放前更新视频时钟
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
            else if(event.key.keysym.sym==SDLK_F1){//重放，设置信号量
                replay=true;
                av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
                if(isEnd){
                    isEnd=false;
                }
                cout<<"replay"<<endl;
            }
            else if(event.key.keysym.sym==SDLK_UP){//音量上升
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
                seek=-1;
            }
            else if(event.key.keysym.sym==SDLK_RIGHT){//快进30秒
                seek=1;
            }
        }
        else if(event.type==SDL_QUIT){//结束
            thread_exit=1;
        }
        else if(event.type==SFM_BREAK_EVENT){//结束
            break;
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