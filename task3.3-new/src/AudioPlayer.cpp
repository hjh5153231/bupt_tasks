#include "AudioPlayer.h"
AudioPlayer::AudioPlayer(){
    AVFormatContext* fmtctx=NULL;
    ret=0;//保存函数返回的tag值
    audio_clock=0;//存储当前视频播放时刻
	isEnd=false;
}

int AudioPlayer::init(const char* filepath){
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
    return 0;
}

int AudioPlayer::readPCMFrame(ALuint & bufferID){
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));//压缩数	
	//重采样参数设置
	int out_nb_samples = 1024;
	enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;//输入采样格式
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;	//输出采样格式
	int in_sample_rate = codecCtx->sample_rate;//输入采样率
	int out_sample_rate = 44100;	//输出采样率
	uint64_t in_ch_layout = codecCtx->channel_layout;	//输入声道布局
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;	
	int out_channels = av_get_channel_layout_nb_channels(out_ch_layout);//根据通道布局类型获取通道数
	int nb_out_channel = av_get_channel_layout_nb_channels(out_ch_layout);//输出声道个数
	int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
	uint8_t* out_buffer_audio;

	codecCtx->channel_layout=av_get_default_channel_layout(codecCtx->channels);
	//swr
	swr_alloc_set_opts(swr, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
	swr_init(swr);

	int ret;
	ALint  state;
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
					return -1;
				}
				else if (ret < 0) {
					cout << "avcodec_receive_frame：" << AVERROR(ret) << endl;
					return -1;
				}
				if (ret >= 0) {
					out_buffer_audio = (uint8_t*)av_malloc(MAX_AUDIO_FRME_SIZE * 2);//*2是保证输出缓存大于输入数据大小
					//重采样
					swr_convert(swr, &out_buffer_audio, MAX_AUDIO_FRME_SIZE, (const uint8_t * *)frame->data, frame->nb_samples);//frame中的data重采样后存储到out_buffer_audio

					out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, frame->nb_samples, out_sample_fmt, 1);//获取该音频帧的size
					audio_clock = av_q2d(codecCtx->time_base) * frame->pts;//更新音频时钟
					alBufferData(bufferID,AL_FORMAT_STEREO16, out_buffer_audio, out_buffer_size, out_sample_rate);//压入缓存
					alSourceQueueBuffers(source,1,&bufferID);//加入播放队列
				}
			}
			av_packet_unref(packet);
			break;
		}
	}
	return 0;
}

int AudioPlayer::readPCMFrame(){//读取一帧，但不进入缓存和播放队列
	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));//压缩数	
	//重采样参数设置
	int out_nb_samples = 1024;
	enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;//输入采样格式
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;	//输出采样格式
	int in_sample_rate = codecCtx->sample_rate;//输入采样率
	int out_sample_rate = 44100;	//输出采样率
	uint64_t in_ch_layout = codecCtx->channel_layout;	//输入声道布局
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;	
	int out_channels = av_get_channel_layout_nb_channels(out_ch_layout);//根据通道布局类型获取通道数
	int nb_out_channel = av_get_channel_layout_nb_channels(out_ch_layout);//输出声道个数
	int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
	uint8_t* out_buffer_audio;

	codecCtx->channel_layout=av_get_default_channel_layout(codecCtx->channels);
	//swr
	swr_alloc_set_opts(swr, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
	swr_init(swr);

	int ret;
	ALint  state;
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
					return -1;
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
					audio_clock = av_q2d(codecCtx->time_base) * frame->pts;
				}
			}
			av_packet_unref(packet);
			break;
		}
	}
	return 0;
}


void AudioPlayer::initopenal(ALuint source)
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

	alSourcei(source, AL_LOOPING, AL_FALSE);
	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
	alListener3f(AL_POSITION, 0, 0, 0);

}

void AudioPlayer::replay(){
    av_seek_frame(fmtCtx, audioindex, 0, AVSEEK_FLAG_BACKWARD);
	avcodec_flush_buffers(codecCtx);
	if(isEnd){
		for (int i = 0; i < NUMBUFFERS; i++) {
			readPCMFrame(buffers[i]);
		}
		isEnd=false;
	}
	audio_clock=0;
	alSourcePlay(source);
}
void AudioPlayer::changeVolume(float volume){
    alSourcef(source, AL_GAIN, volume);
}

void AudioPlayer::toSeek(int seek){
    av_seek_frame(fmtCtx, audioindex, (audio_clock+30.0*seek)/av_q2d(fmtCtx->streams[audioindex]->time_base), AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(codecCtx);
    readPCMFrame();
}


int AudioPlayer::playfile(){
    AVCodec* codec = NULL;
    //openAL
    ALCdevice *device = NULL;
    ALCcontext *context = NULL;
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
	initopenal(source);//初始化
	alGenBuffers(NUMBUFFERS, buffers); //创建缓冲区
    ALint processed;//已处理的缓冲数
	ALint  state;//播放器的状态
	ALint iQueuedBuffers;//当前播放队列中的缓冲数
    //播放开始前填充缓冲区
	for (int i = 0; i < NUMBUFFERS; i++) {
		readPCMFrame(buffers[i]);
	}
	audio_clock=0;
	alSourcePlay(source);//开始播放

	while(true){
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if (thread_exit) {//线程退出，释放资源
			alSourceStop(source);
			alSourcei(source, AL_BUFFER, 0);
			alDeleteBuffers(NUMBUFFERS, buffers);
			alDeleteSources(1, &source);
			break;
		}
		else if(isEnd){
			if(state==AL_PLAYING){
				alSourcePause(source);
			}
		}
		processed = 0;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		//当存在缓冲区被处理完毕，填充新的音频帧到缓冲区
		while (processed > 0) {
			ALuint bufferID = 0;
			alSourceUnqueueBuffers(source, 1, &bufferID);
			readPCMFrame(bufferID);
			processed--;
		}
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if(thread_pause){//暂停
			if(state==AL_PLAYING){
				alSourcePause(source);
			}
		}
		else if (state != AL_PLAYING){//播放器不在播放
			alGetSourcei(source, AL_BUFFERS_QUEUED, &iQueuedBuffers);
			if (iQueuedBuffers) {//播放队列中还有数据就继续播放
				alSourcePlay(source);
				isEnd=false;
			}
			else{
				isEnd=true;
			}
		}
	}

	alSourceStop(source);
	alSourcei(source, AL_BUFFER, 0);
	alDeleteBuffers(NUMBUFFERS, buffers);
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