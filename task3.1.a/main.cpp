#include <iostream>
extern "C"{
    #include "SDL.h"  
}
#include <Windows.h>

#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)
using namespace std;
int thread_exit=0;
int thread_pause=0;
int restart_video=0;

int refreshScreen(void *opaque){
    thread_exit=0;
    thread_pause=0;
    while(!thread_exit){
        if(!thread_pause){
            SDL_Event event;
            event.type=SFM_REFRESH_EVENT;
            SDL_PushEvent(&event);
        }
        SDL_Delay(40);
    }
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
    cout<<"请输入要播放的yuv文件url"<<endl;
    string s_filepath;
    string tmp;
    int w,h;
    getline(cin,s_filepath);
    const char* filepath=s_filepath.c_str();
    cout<<"请输入视频的宽："<<endl;
    getline(cin,tmp);
    w=stoi(tmp);
    cout<<"请输入视频的高："<<endl;
    getline(cin,tmp);
    h=stoi(tmp);

    //SDL变量声明
	SDL_Window *screen; 
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;



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
    FILE *fp=NULL;
    fp=fopen(filepath,"rb+");
    if(fp==NULL){
        cout<<"can not open file"<<endl;
        return -1;
    }

    SDL_Thread *video_tid;
    SDL_Event event;
    video_tid=SDL_CreateThread(refreshScreen,NULL,NULL);
    //SDL-init done


    bool isReplay=false;
    bool isEnd=false;
    while(true){
        SDL_WaitEvent(&event);
        if(event.type==SFM_REFRESH_EVENT){
            
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
        }
        else if(event.type==SDL_QUIT){
            thread_exit=1;
        }
        else if(event.type==SFM_BREAK_EVENT){
            break;
        }
        if(restart_video){
            cout<<"restart"<<endl;
            //重置到开头
            cout<<"restart success"<<endl;
            restart_video=0;
        }
    }
    

    SDL_Quit();
    return 0;
}
