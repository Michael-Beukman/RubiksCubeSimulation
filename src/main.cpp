#include "graphics_window.h"
#include<iostream>
using namespace std;


int main(){
    Graphics_Window myWindow(1600,1200);
    myWindow.Start();
    cout <<"window end "<<endl;
    return 0;
}