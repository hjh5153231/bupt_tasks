#include <iostream>
#include <stdio.h>
#include <vector>
#include "al.h"
#include "alc.h"
#include <conio.h>
#include <queue>
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

typedef struct _AUTOFRAME {
	void* data;
	int size;
	int samplerate;
	double audio_clock;
	int64_t audio_timestamp;
}AUTOFRAME, * PAUTOFRAME;
//定义宏
#define BUFFER_NUM 3
#define	SERVICE_UPDATE_PERIOD 20
#define MAX_AUDIO_FRME_SIZE 192000
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

using namespace std;
bool volumeChanged=false;
float volume = 1.0;
int thread_exit=0;
int thread_pause=0;
int restart_video=0;
queue<PAUTOFRAME> queueData; 
ALuint source;
double audio_clock = 0;
double delay;
int videoOffset=0;
int fps;
bool isReplay=false;
bool isEnd=false;

int refreshScreen(void *opaque){
    thread_exit=0;
    thread_pause=0;
    while(!thread_exit){
        if(!thread_pause){
            SDL_Event event;
            event.type=SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
		if(videoOffset==1){
			SDL_Delay(950/fps);
		}
		else if(videoOffset==0){
			SDL_Delay(1000/fps);
		}
		else{
			SDL_Delay(50);
		}
    }
    thread_exit=0;
    thread_pause=0;
    SDL_Event event;
    event.type=SFM_BREAK_EVENT;
    SDL_PushEvent(&event);
    return 0;
}

int sdlRender(string s_filepath){
	const char* filepath=s_filepath.c_str();
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
    video_tid=SDL_CreateThread(refreshScreen,NULL,NULL);
    //SDL-init done

    //packet to yuv
    pkt = av_packet_alloc();
    av_new_packet(pkt, w * h); 
	double video_clock=0;
    while(true){
        SDL_WaitEvent(&event);
        if(event.type==SFM_REFRESH_EVENT){
            while(true){
                if(av_read_frame(fmtCtx, pkt)<0){
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
            if(isReplay){
                av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD); // 将视频重置到开头
                isReplay=false;
				isEnd=false;
            }
            if(isEnd){
                av_packet_unref(pkt);
                continue;
            }
            if(avcodec_send_packet(codecCtx,pkt)==0){
                while(avcodec_receive_frame(codecCtx,yuvFrame)==0){
					video_clock=yuvFrame->pts*av_q2d(fmtCtx->streams[videoStreamIndex]->time_base);
					delay=audio_clock-video_clock;
					cout<<"音视频时间差值:"<<delay<<endl;
					if(delay>0.03){
						videoOffset=1;
					}
					else if(delay<-0.03){
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
                restart_video=1;
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
        }
        else if(event.type==SDL_QUIT){
            thread_exit=1;
        }
        else if(event.type==SFM_BREAK_EVENT){
            break;
        }
        if(restart_video){
            cout<<"restart"<<endl;
            av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD); // 将视频重置到开头
            cout<<"restart success"<<endl;
            restart_video=0;
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


//初始化openal
void setopenal(ALuint source)
{
	ALfloat SourceP[] = { 0.0, 0.0, 0.0 };
	ALfloat SourceV[] = { 0.0, 0.0, 0.0 };
	ALfloat ListenerPos[] = { 0.0, 0, 0 };
	ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
	ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };
	alSourcef(source, AL_PITCH, 1.0);
	alSourcef(source, AL_GAIN, 1.0);
	alSourcefv(source, AL_POSITION, SourceP);
	alSourcefv(source, AL_VELOCITY, SourceV);
	alSourcef(source, AL_REFERENCE_DISTANCE, 50.0f);
	alSourcei(source, AL_LOOPING, AL_FALSE);
	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
	alListener3f(AL_POSITION, 0, 0, 0);

}


int SoundCallback(ALuint & bufferID) {
	if (queueData.empty()) return -1;
	PAUTOFRAME frame = queueData.front();
	queueData.pop();
	if (frame == nullptr)
		return -1;
	//把数据写入buffer
	alBufferData(bufferID, AL_FORMAT_STEREO16, frame->data, frame->size, frame->samplerate);
	//将buffer放回缓冲区
	alSourceQueueBuffers(source, 1, &bufferID);
	audio_clock = frame->audio_clock;
	queueData.push(frame);
	//释放数据
	// if (frame) {
	// 	av_free(frame->data);
	// 	delete frame;
	// }
	return 0;
}






#undef main
int main()
{
    system("chcp 65001");
    cout<<"请输入要解码并播放的mp4文件url"<<endl;
    string s_filepath;
    getline(cin,s_filepath);
    const char* filepath=s_filepath.c_str();
	//ffmpeg对于封装文件进行解码
	AVFormatContext* fmtCtx=NULL;
    AVCodecParameters* avCodecPara=NULL;
    AVCodecContext* codecCtx = NULL;
    AVCodec* codec = NULL;
    int audioindex = -1;
	

	fmtCtx = avformat_alloc_context();//初始化
	if (avformat_open_input(&fmtCtx, filepath, NULL, NULL) != 0)//打开音频/视频流
	{
		cout << "Couldn't open the input file" << endl;
		return -1;
	}

	if (avformat_find_stream_info(fmtCtx, NULL) < 0)//查找视频/音频流信息
	{
		cout << "Couldn't find the stream information" << endl;
		return -1;
	}

	//查找音频流
	for (int i = 0; i < fmtCtx->nb_streams; i++)
	{
		if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioindex = i;
			break;
		}
	}
	if (audioindex == -1)
	{
		cout << "Couldn't find a audio stream" << endl;
		return -1;
	}

	//查找解码器
	codecCtx = avcodec_alloc_context3(NULL);
	avcodec_parameters_to_context(codecCtx, fmtCtx->streams[audioindex]->codecpar);
	codec = avcodec_find_decoder(codecCtx->codec_id);
	codecCtx->pkt_timebase = fmtCtx->streams[audioindex]->time_base;
	if (codec == NULL)//查找解码器
	{
		cout << "Couldn't find decoder" << endl;
		return -1;
	}
	//打开解码器
	if (avcodec_open2(codecCtx, codec, NULL) < 0)
	{
		cout << "Couldn't open decoder" << endl;
		return -1;
	}
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));//解码前数据
	AVFrame* frame = av_frame_alloc();//解码后音频帧
	SwrContext* swr = swr_alloc();//重采样
	//重采样参数设置
	int out_nb_samples = 1024;
	enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;//输入采样格式
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;	//输出采样格式
	int in_sample_rate = codecCtx->sample_rate;//输入采样率
	int out_sample_rate = 44100;	//输出采样率
	uint64_t in_ch_layout = codecCtx->channel_layout;	//输入声道布局
	uint64_t out_ch_layout = in_ch_layout;	
	int out_channels = av_get_channel_layout_nb_channels(out_ch_layout);//根据通道布局类型获取通道数
	int nb_out_channel = av_get_channel_layout_nb_channels(out_ch_layout);//输出声道个数
	int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
	uint8_t* out_buffer_audio;

	codecCtx->channel_layout=av_get_default_channel_layout(codecCtx->channels);
	//swr
	swr_alloc_set_opts(swr, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
	swr_init(swr);
	//openAL
	ALCdevice *device = NULL;
    ALCcontext *context = NULL;
    device = alcOpenDevice(NULL);
    if (!device){
        fprintf(stderr,"fail to open device\n");
        return -1;
    }
    context = alcCreateContext(device,NULL);
    if (!context) {
        fprintf(stderr,"fail to create context.\n");
        return -1;
    }
    alcMakeContextCurrent(context);
    if (alGetError() != AL_NO_ERROR) {
        return -1;
    }
	
    alGenSources(1, &source);
	setopenal(source);//初始化


	int ret;
	while (av_read_frame(fmtCtx, packet) >= 0) {//读取下一帧数据
		if (packet->stream_index == audioindex) {
			ret = avcodec_send_packet(codecCtx, packet);
			if (ret < 0) {
				cout << "avcodec_send_packet：" << ret << endl;
				continue;
			}
			while (ret >= 0) {
				ret = avcodec_receive_frame(codecCtx, frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;
				}
				else if (ret < 0) {
					cout << "avcodec_receive_frame：" << AVERROR(ret) << endl;
					return -1;
				}

				if (ret >= 0) {
					out_buffer_audio = (uint8_t*)av_malloc(MAX_AUDIO_FRME_SIZE * 2);//*2是保证输出缓存大于输入数据大小
					//重采样
					swr_convert(swr, &out_buffer_audio, MAX_AUDIO_FRME_SIZE, (const uint8_t * *)frame->data, frame->nb_samples);

					out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, frame->nb_samples, out_sample_fmt, 1);
					PAUTOFRAME pAUTOFRAME = new AUTOFRAME;
					pAUTOFRAME->data = out_buffer_audio;
					pAUTOFRAME->size = out_buffer_size;
					pAUTOFRAME->samplerate = out_sample_rate;
					audio_clock = av_q2d(codecCtx->time_base) * frame->pts;
					pAUTOFRAME->audio_clock = audio_clock;

					queueData.push(pAUTOFRAME);  //解码后数据存入队列
				}
			}
		}
		av_packet_unref(packet);
	}

	ALuint m_buffers[BUFFER_NUM];
	alGenBuffers(BUFFER_NUM, m_buffers); //创建缓冲区

	std::thread sdlplay{sdlRender, s_filepath};
	sdlplay.detach();

	for (int i = 0; i < BUFFER_NUM; i++) {
		SoundCallback(m_buffers[i]);
	}
	alSourcePlay(source);
	ALint processed;
	ALint  state;
	ALint iQueuedBuffers;
	while (!queueData.empty()){
		processed = 0;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		while (processed > 0) {
			ALuint bufferID = 0;
			alSourceUnqueueBuffers(source, 1, &bufferID);
			SoundCallback(bufferID);
			processed--;
		}
		alGetSourcei(source, AL_SOURCE_STATE, &state);

		if(volumeChanged){
			volumeChanged=false;
			alSourcef(source, AL_GAIN, volume);
		}
		if(thread_pause){
			alSourcePause(source);
		}
		else if(isEnd){
			alSourcePause(source);
		}
		else if (state != AL_PLAYING){
			alGetSourcei(source, AL_BUFFERS_QUEUED, &iQueuedBuffers);
			if (iQueuedBuffers) {
				alSourcePlay(source);
			}
			else {
				break;
			}
		}
		if (thread_exit) {
			 alSourceStop(source);
			 alSourcei(source, AL_BUFFER, 0);
			 alDeleteBuffers(BUFFER_NUM, m_buffers);
			 alDeleteSources(1, &source);
			 break;
		 }
	}
	alSourceStop(source);
	alSourcei(source, AL_BUFFER, 0);
	alDeleteBuffers(BUFFER_NUM, m_buffers);
	alDeleteSources(1, &source);



	av_frame_free(&frame);
	swr_free(&swr);


	ALCcontext* pCurContext = alcGetCurrentContext();
	ALCdevice* pCurDevice = alcGetContextsDevice(pCurContext);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(pCurContext);
	alcCloseDevice(pCurDevice);


	return 0;
}