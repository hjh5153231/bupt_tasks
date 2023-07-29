#include <iostream>
#include <stdio.h>
#include <vector>
#include "al.h"
#include "alc.h"
#include <conio.h>
using namespace std;
int main(){
    system("chcp 65001");
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
    //音频播放源
    ALuint source;
    //音频数据
    ALuint buffer;
    //音频格式
    ALenum audioFormat=AL_FORMAT_STEREO16;
    //声道数目
    ALshort channels = 2;
    //音频采样率
    ALsizei sample_rate = 44100;
    //是否循环播放
    ALboolean loop = 1;
    //播放源的位置
    ALfloat position[] = {0.0f,0.0f,0.0f};
    //播放的速度
    ALfloat velocity[] = {0.0f,0.0f,0.0f};
    alGenBuffers(1,&buffer);
    alGenSources(1,&source);
    cout<<"请输入播放的pcm文件url"<<endl;
    string s_filepath;
    getline(cin,s_filepath);
    const char* filepath=s_filepath.c_str();
    FILE *f = fopen(filepath,"rb");
    fseek(f,0,SEEK_END);
    long length  = ftell(f);
    rewind(f);
    char* data = (char*)malloc(length);
    fread(data,sizeof(char),length,f);
    fclose(f);
    alBufferData(buffer,audioFormat,data,length,sample_rate);
    if (alGetError() != AL_NO_ERROR) {
        return -1;
    }
    //为source绑定数据
    alSourcei(source,AL_BUFFER,buffer);
    //音高倍数
    alSourcef(source,AL_PITCH,1.0f);
    //声音的增益
    alSourcef(source,AL_GAIN,1.0f);
    //设置位置
    alSourcefv(source,AL_POSITION,position);
    //设置声音移动速度
    alSourcefv(source,AL_VELOCITY,velocity);
    //设置是否循环播放
    alSourcei(source,AL_LOOPING,loop);
    //播放音乐
    alSourcePlay(source);
    ALint sourceState;
    alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    if(sourceState==AL_PLAYING){
        cout<<"music is playing."<<endl;
    }
    float volume = 1.0f;
    while (true) {
        // 处理键盘输入
        if (_kbhit()) {
            int key = _getch();

            // 切换暂停/播放
            if (key == ' ') {
                ALint sourceState;
                alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
                if (sourceState == AL_PLAYING) {
                    alSourcePause(source);
                } else if (sourceState == AL_PAUSED) {
                    alSourcePlay(source);
                }
            }
            // 调整音量
            else if (key == 0xE0 || key==0) {
                key = _getch();
                if (key == 0x48) { // 上箭头
                    volume += 0.1f;
                } else if (key == 0x50) { // 下箭头
                    volume -= 0.1f;
                }
                // 确保音量在合法范围内 [0.0, 1.0]
                if (volume < 0.0f) {
                    volume = 0.0f;
                } else if (volume > 1.0f) {
                    volume = 1.0f;
                }
                cout<<"调节音量到"<<volume<<endl;
                alSourcef(source, AL_GAIN, volume);
            }
            else if (key == 'R' || key == 'r') {
                alSourceRewind(source); // 从头开始播放
                ALint iTotal = 0;
                ALint iCurrent = 0;
                ALint uiBuffer = 0;
                alGetSourcei(source, AL_BUFFER, &uiBuffer);
                alGetBufferi(uiBuffer, AL_SIZE, &iTotal);
                iCurrent = iTotal * 0;
                alSourcei(source, AL_BYTE_OFFSET, iCurrent);
                alSourcePlay(source);
                cout<<"重新播放"<<endl;
            }
            else if (key == 13) { // 回车键的ASCII码为13
                alSourceStop(source);
            }
            // 获取音频源的状态
            alGetSourcei(source, AL_SOURCE_STATE, &sourceState);

            if (sourceState == AL_PLAYING) {
                // 输出播放状态信息
                cout << "Music is playing." << endl;
            } else if (sourceState == AL_PAUSED) {
                // 输出暂停状态信息
                cout << "Music is paused." << endl;
            } else if (sourceState == AL_STOPPED) {
                // 输出停止状态信息
                cout << "Music has stopped playing." << endl;
                break; // 退出循环，结束程序
            }
        }
    }
    //释放资源
    free(data);
    alcMakeContextCurrent(NULL);
    alDeleteSources(1,&source);
    alDeleteBuffers(1,&buffer);
    alcDestroyContext(context);
    alcCloseDevice(device);
    return 0;
}