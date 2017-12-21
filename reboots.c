/*
Daniel Orozco
@ UVG
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (void){
    srand(time(NULL));
    int seconds = (rand() % 1800) + 1; // random between 1-30 mins [1 - 1800 in seconds]
    usleep(seconds*1000000);
    int reboot = system("sudo reboot");
    return 0;
}