/*
Daniel Orozco
@ UVG
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (void){
    srand(time(NULL));
    int seconds = (rand() % 1500) + 300; // random between 5-30 mins [300 - 1800 in seconds]
    FILE * datos_file;
    datos_file = fopen("/home/pi/Desktop/reboots.txt","a");
    fprintf(datos_file, "%d\n", seconds);
    fclose(datos_file);
    usleep(seconds*1000000);
    int reboot = system("sudo reboot");
    return 0;
}