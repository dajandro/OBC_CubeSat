/*
Daniel Orozco
@ UVG
*/

#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <time.h>
#include <wiringPi.h>
#include <unistd.h>
#include <sys/types.h>

#define uart_device "/dev/ttyS0"
#define n_retry_bus 4
#define n_retry_read 10
#define n_retry_read_bus_master 5
#define max_time_retry_bus 60
#define max_time_retry_read 2
#define max_time_retry_read_bus_master 2
#define max_time_ventaja 10
#define max_buffer_size 20
#define alive_char '1'
#define bus_master '2'

#define ledBusMaster 0

#define i2c_path "/home/pi/Desktop/./master"

int uart0_filestream = -1;

//----- TX BYTES -----
unsigned char tx_buffer[20];
unsigned char *p_tx_buffer;
//----- RX BYTES -----
unsigned char rx_buffer[max_buffer_size];

void launch_i2c(){
    pid_t pid = fork();
    if(pid==0){
        static char *argv[] = {"","",NULL};
        execv(i2c_path, argv);
    }
}

void set_bus_master(char c){
    tx_buffer[1] = c;
}

void as_main(){
    digitalWrite(ledBusMaster, HIGH);
    set_bus_master(bus_master);    
    printf("Running as Main\n");
    // run i2c
    launch_i2c();
    int alive = -1;
    while(1){
	    usleep(1000000);
        int s = send_alive_signal(uart0_filestream);
	}
}

void as_backup(){
    digitalWrite(ledBusMaster, LOW);
    printf("Running as Backup\n");
    int alive = -1;    
    while(alive){
        printf("Main alive\n");
        rx_buffer[0] = '\0';
        rx_buffer[1] = '\0';
        alive = read_alive_signal(uart0_filestream);
	}
    as_main();
}

int main (void){

    //int uart0_filestream = -1;
    uart0_filestream = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode

    if (uart0_filestream == -1){
        //ERROR - CAN'T OPEN SERIAL PORT
        printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
    }

    if(wiringPiSetup() == -1){ //when initialize wiringPi failed, print message to screen
        printf("wiringPi setup failed !\n");
    }

    pinMode(ledBusMaster, OUTPUT);

    // tiempo de ventaja por si inician al mismo tiempo
    usleep(max_time_ventaja * 1000000);

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;		//<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = alive_char;
    *p_tx_buffer++ = bus_master;

    if (read_bus_master(uart0_filestream))
        as_main();
    else
        as_backup();
}

int reopen_uart(int uart_filestream){
    int try = 0;
    uart_filestream = open(uart_device, O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
    if (uart_filestream != -1)
        return uart_filestream;
    while(try<n_retry_bus){
        if (try == n_retry_bus-1){
            printf("Last try to re open UART\n");
            usleep(max_time_retry_bus * 1000000);
            close(uart_filestream);
            uart_filestream = open(uart_device, O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
            if (uart_filestream != -1)
                return uart_filestream;
        }
        if (uart_filestream == -1){
            printf("Trying to re open UART\n");
            if (try == n_retry_bus){
                printf("Unable to open UART\n");
            }
            close(uart_filestream);
            uart_filestream = open(uart_device, O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
            if (uart_filestream != -1)
                return uart_filestream;
        }
        try++;
    }
    return -1;
}

int send_alive_signal(int uart_filestream){
    if (uart_filestream == -1)
        uart_filestream = reopen_uart(uart_filestream);
    if (uart_filestream == -1){
        printf("Unable to send data\n");
        return 0;
    }
    rx_buffer[0] = '\0';
    rx_buffer[1] = '\0';
    int count = write(uart_filestream,  &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));		//Filestream, bytes to write, number of bytes to write
    if (count < 0)
        return 0;
    //printf("Main ALIVE!\n");
    return 1;
}

int read_alive_signal(int uart_filestream){
    if (uart_filestream == -1)
        uart_filestream = reopen_uart(uart_filestream);
    if (uart_filestream == -1){
        printf("Unable to read data\n");
        return 0;
    }
    int retry = 0;
    while(retry < n_retry_read){
        int rx_length = read(uart_filestream, (void*)rx_buffer, max_buffer_size-1);		//Filestream, buffer to store in, number of bytes to read (max)
        rx_buffer[rx_length] = '\0';
        if(rx_buffer[0] == alive_char){
            rx_buffer[0] = '\0';
            rx_buffer[1] = '\0';
            return 1;
        }
    	rx_buffer[0] = '\0';
    	rx_buffer[1] = '\0';
        //send_alive_signal(uart_filestream);
        usleep(max_time_retry_read * 1000000);
        retry++;
    }
    return 0;
}

int read_bus_master(int uart_filestream){
    if (uart_filestream == -1)
        uart_filestream = reopen_uart(uart_filestream);
    if (uart_filestream == -1){
        printf("Unable to read data\n");
        return 0;
    }
    int retry = 0;
    while(retry < n_retry_read_bus_master){
        int rx_length = read(uart_filestream, (void*)rx_buffer, max_buffer_size-1);		//Filestream, buffer to store in, number of bytes to read (max)
        rx_buffer[rx_length] = '\0';
        if(rx_buffer[1] == '1')
            return 0;
    	rx_buffer[0] = '\0';
    	rx_buffer[1] = '\0';
        usleep(max_time_retry_read_bus_master * 1000000);
        retry++;
    }
    return 1;
}