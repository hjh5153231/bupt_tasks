#include <iostream>
extern "C"{
    #include "libavcodec/avcodec.h"
}
using namespace std;

int main(){
    cout<<avcodec_configuration()<<endl;
}
