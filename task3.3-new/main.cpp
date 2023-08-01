#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <thread>
#include "VideoPlayer.h"
#include "AudioPlayer.h"
#include <functional>


bool replay=false;
bool volumeChanged=false;
float volume=1.0f;
int seek=0;
void videoPlayThread(VideoPlayer &videoplayer){
	videoplayer.playfile(std::ref(replay),std::ref(volumeChanged),std::ref(volume),std::ref(seek));
}

void audioPlayThread(AudioPlayer &audioplayer){
	audioplayer.playfile();
}

void synchronous(double &speed,double &audio_clock,double &video_clock){
	double delay;
	while(true){
		delay=audio_clock-video_clock;
		cout<<"音视频差值:"<<delay<<endl;
		if(delay>0.03){
			speed=0.6;
		}
		else if(delay<0){//由于音频的时钟受缓存大小影响，因此不设定为-0.03
			speed=1.4;
		}
		else{
			speed=1;
		}
	}
}
#undef main
int main()
{
    system("chcp 65001");
    cout<<"请输入要解码并播放的mp4文件url"<<endl;
    string s_filepath;
    getline(cin,s_filepath);
    const char* filepath=s_filepath.c_str();
	VideoPlayer myVideoPlayer;
	if(myVideoPlayer.init(filepath)!=0){
		cout<<"视频播放器初始化失败"<<endl;
		return -1;
	}
	AudioPlayer myAudioPlayer;
	if(myAudioPlayer.init(filepath)!=0){
		cout<<"音频播放器初始化失败"<<endl;
		return -1;
	}
	std::thread audioPlay(audioPlayThread,std::ref(myAudioPlayer));//创建音频线程
    audioPlay.detach();//线程后台运行

	std::thread videoPlay(videoPlayThread,std::ref(myVideoPlayer));//创建视频线程
    videoPlay.detach();//线程后台运行

	std::thread synchronousThread(synchronous,std::ref(myVideoPlayer.speed),std::ref(myAudioPlayer.audio_clock),std::ref(myVideoPlayer.video_clock));
	synchronousThread.detach();

	while (true) {
		if(myVideoPlayer.thread_pause!=myAudioPlayer.thread_pause){
			myAudioPlayer.thread_pause=myVideoPlayer.thread_pause;
		}
		if(myVideoPlayer.thread_exit!=myAudioPlayer.thread_exit){
			myAudioPlayer.thread_exit=myVideoPlayer.thread_exit;
		}
		if(replay){
			myAudioPlayer.replay();
			replay=false;
		}
		if(volumeChanged){
			myAudioPlayer.changeVolume(volume);
			volumeChanged=false;
		}
		if(seek==1){
			myAudioPlayer.thread_pause=1;
			myVideoPlayer.thread_pause=1;
			myAudioPlayer.toSeek(1);
			myVideoPlayer.toSeek(1,myAudioPlayer.audio_clock);
			seek=0;
			myAudioPlayer.thread_pause=0;
			myVideoPlayer.thread_pause=0;
		}
		else if(seek==-1){
			myAudioPlayer.thread_pause=1;
			myVideoPlayer.thread_pause=1;
			myAudioPlayer.toSeek(-1);
			myVideoPlayer.toSeek(-1,myAudioPlayer.audio_clock);
			seek=0;
			myAudioPlayer.thread_pause=0;
			myVideoPlayer.thread_pause=0;
		}
		else{
			
		}
    }

	return 0;
}
