#pragma once
#include <string>
using namespace std;

int refreshScreen(int &thread_exit,int &thread_pause);
int sdlRender(string s_filepath,double &audio_clock,int &thread_exit,int &thread_pause,bool &volumeChanged,float &volume,bool &isEnd,int &restart_video,int &audioSeek);