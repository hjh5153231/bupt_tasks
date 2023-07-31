#include <iostream>
extern "C"{
    #include "SDL.h"  
}
#include <Windows.h>
//自定义SDL事件
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

using namespace std;
//全局变量定义
int thread_exit=0;
int thread_pause=0;
int restart_video=0;

//视频线程函数，刷新屏幕
int refreshScreen(void *opaque){
    //初始化
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
    cout<<"请输入要播放的yuv文件url"<<endl;
    string s_filepath;
    string tmp;
    int w,h;
    const int bpp=12;
    getline(cin,s_filepath);
    const char* filepath=s_filepath.c_str();
    cout<<"请输入视频的宽："<<endl;
    getline(cin,tmp);
    w=stoi(tmp);
    cout<<"请输入视频的高："<<endl;
    getline(cin,tmp);
    h=stoi(tmp);
    unsigned char buffer[w*h*bpp/8];//定义固定大小缓存来存储yuv帧信息
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
    //windows弹出窗口在最后面，调用这个后正常置顶弹出
    HWND hwnd = GetActiveWindow();
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    //初始化SDL变量
    sdlRenderer=SDL_CreateRenderer(screen,-1,0);
    sdlTexture=SDL_CreateTexture(sdlRenderer,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,w,h);
    sdlRect.x=0;
    sdlRect.y=0;
    sdlRect.w=w;
    sdlRect.h=h;
    //打开yuv文件
    FILE *fp=NULL;
    fp=fopen(filepath,"rb+");
    if(fp==NULL){
        cout<<"can not open file"<<endl;
        return -1;
    }

    //利用SDL事件机制，创建线程来控制画面刷新
    SDL_Thread *video_tid;
    SDL_Event event;
    video_tid=SDL_CreateThread(refreshScreen,NULL,NULL);




    //主线程一直循环，对不同的事件作不同的处理
    while(true){
        SDL_WaitEvent(&event);
        if(event.type==SFM_REFRESH_EVENT){//若当前需要刷新则从fp中读取一帧数据并刷新屏幕
            if (fread(buffer, 1, w*h*bpp/8, fp) != w*h*bpp/8){//若yuv播放结束，则利用事件阻塞暂停主线程，直到按F1进行重播
				// Loop
                SDL_Event event1;
                SDL_WaitEvent(&event1);
                if(event1.type==SDL_KEYDOWN){
                    if(event1.key.keysym.sym==SDLK_F1){
                        fseek(fp, 0, SEEK_SET);
                        fread(buffer, 1, w*h*bpp/8, fp);
                    }
                }
                else{
                    continue;
                }
			}
            SDL_UpdateTexture( sdlTexture, NULL, buffer, w);
            SDL_RenderClear( sdlRenderer );   
			SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect);  
			SDL_RenderPresent( sdlRenderer );  
        }
        else if(event.type==SDL_KEYDOWN){
            if(event.key.keysym.sym==SDLK_SPACE){//若用户按空格，那么则对thread_pause进行取反来控制暂停
                thread_pause=!thread_pause;
                cout<<"out space"<<endl;
            }
            else if(event.key.keysym.sym==SDLK_F1){//F1重新播放视频
                restart_video=1;
                cout<<"out f1"<<endl;
            }
        }
        else if(event.type==SDL_QUIT){//退出
            thread_exit=1;
        }
        else if(event.type==SFM_BREAK_EVENT){//退出
            break;
        }
        if(restart_video){//若重播被触发，则将视频调整到开头重新播放
            cout<<"restart"<<endl;
            fseek(fp, 0, SEEK_SET);
            cout<<"restart success"<<endl;
            restart_video=0;
        }
    }
    

    SDL_Quit();
    return 0;
}
