/*
Daniel Orozco
@ UVG
*/

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define n_temperatura 91
#define n_voltaje 46
#define n_estado_carga 46
#define slave1_address 0x13
#define slave2_address 0x08

#define img_path "/home/pi/Desktop/img.bmp"

int deviceHandle1;
int deviceHandle2;
int readBytes1;
int readBytes2;
char buffer1[7];
char data1[16];
char buffer2[7];
char data2[8];

time_t timer;
char time_buffer[26];
struct tm* tm_info;

struct temperatura_bateria
{
	char id[3];
	char time_stamp[26];
	float valor;
} log_temperatura_bateria[n_temperatura];

struct estado_de_carga_bateria
{
	char id[3];
	char time_stamp[26];
	float valor;
} log_estado_de_carga_bateria[n_estado_carga];

struct voltaje_bateria
{
	char id[3];
	char time_stamp[26];
	float valor;
} log_voltaje_bateria[n_voltaje];

int nc1 = 0;
int nc2 = 0;
int nc3 = 0;

int binTwosComplementToInt(char binary[], int bits);
int connect_slave_1(int slave_address);
int connect_slave_2(int slave_address);
void logs_to_txts();
void estado_de_carga();
int get_estado_de_carga();
int validar_estado_de_carga();
void voltaje_de_baterias();
float get_voltaje_de_baterias();
int validar_voltaje_de_baterias(float valor);
void temperatura_de_baterias();
float get_temperatura_de_baterias();
int validar_temperatura_de_baterias(float valor);
void enviar_img(char *fileName);
void enviar_datos_a_transmitir(int tipo);	

int main (void)
{
	printf("Raspberry Pi I2C\n");
	
	// inicializar buffers
	buffer1[0] = 0x00;
	buffer2[0] = 0x00;
 
	time_t start, end;
	double elapsed;
	start = time(NULL);
	int terminate = -1;

	int toca120 = 0;

	if (connect_slave_1(slave1_address) == 1){
		while(terminate){
			end = time(NULL);
			elapsed = difftime(end, start);
			if (elapsed > 5400.0) // test 2 - 1h 30 minutos
			//if (elapsed > 20.0) // test 1 - 20 s
				terminate = 0;
			else {
				if(toca120%2 == 0){
					estado_de_carga();
					voltaje_de_baterias();
					nc1++;
					nc2++;
				}
				temperatura_de_baterias();
				nc3++;
				toca120++;
				logs_to_txts();
				usleep(60000000); // test 2 - 60s
				//usleep(2000000); // test 1 - 2s
			}
		}
		close(deviceHandle1);
		logs_to_txts();
	}

	/* if (connect_slave_2(slave2_address) == 1){
		// drive some tests
        	//send_picture();
		enviar_datos_a_transmitir(0);
		//enviar_datos_a_transmitir(1);
		// close connection and return
		close(deviceHandle2);
	} */
	
	return 0;
}

int binTwosComplementToInt(char binary[], int bits){
	int potencia = pow(2,bits-1);
	int res = 0;
	for(int i=0; i<bits; i++){
		if (i==0 && binary[i]!='0')
			res += potencia * -1;
		else
			res += (binary[i]-'0')*potencia;
		potencia /= 2;
	}
	return res;
}

int connect_slave_1(int slave_address){
	// open device on /dev/i2c-0
	if ((deviceHandle1 = open("/dev/i2c-1", O_RDWR)) < 0) {
		printf("Error: no se pudo abrir la interfaz I2C! %d\n", deviceHandle1);
		return 0;
	}
	
	// connect to arduino as i2c slave
	if (ioctl(deviceHandle1, I2C_SLAVE, slave_address) < 0) {
		printf("Error: no hay respuesta del esclavo!\n");
		return 0;
	}
	
	// begin transmission and request acknowledgement
	readBytes1 = write(deviceHandle1, buffer1, 1);
	if (readBytes1 != 1)
	{
		printf("Error: No se recibio el ACK-Bit, no se pudo establecer la conexion!\n");
		return 0;
	}
	printf("Esclavo %d conectado\n", slave_address);
	return 1;
}

int connect_slave_2(int slave_address){
	// open device on /dev/i2c-0
	if ((deviceHandle2 = open("/dev/i2c-1", O_RDWR)) < 0) {
		printf("Error: no se pudo abrir la interfaz I2C! %d\n", deviceHandle2);
		return 0;
	}
	
	// connect to arduino as i2c slave
	if (ioctl(deviceHandle2, I2C_SLAVE, slave_address) < 0) {
		printf("Error: no hay respuesta del esclavo!\n");
		return 0;
	}  
	
	// begin transmission and request acknowledgement
	readBytes2 = write(deviceHandle2, buffer2, 1);
	if (readBytes2 != 1)
	{
		printf("Error: No se recibio el ACK-Bit, no se pudo establecer la conexion!\n");
		return 0;
	}
	printf("Esclavo %d conectado\n", slave_address);
	return 1;
}

void logs_to_txts(){
	FILE * datos_file;

	datos_file = fopen("/home/pi/Desktop/dataC1.txt","a");
	for(int i=0; i<nc1; i++){
		char entry[36] = "";
		strcpy(entry, log_estado_de_carga_bateria[i].id);
		strcat(entry, ",");
		strcat(entry, log_estado_de_carga_bateria[i].time_stamp);
		strcat(entry, ",%f\n");
		fprintf(datos_file, entry, log_estado_de_carga_bateria[i].valor);
	}
	fclose(datos_file);

	datos_file = fopen("/home/pi/Desktop/dataC2.txt","a");
	for(int i=0; i<nc2; i++){
		char entry[36] = "";
		strcpy(entry, log_voltaje_bateria[i].id);
		strcat(entry, ",");
		strcat(entry, log_voltaje_bateria[i].time_stamp);
		strcat(entry, ",%f\n");
		fprintf(datos_file, entry, log_voltaje_bateria[i].valor);
	}
	fclose(datos_file);

	datos_file = fopen("/home/pi/Desktop/dataC3.txt","a");
	for(int i=0; i<nc3; i++){
		char entry[36] = "";
		strcpy(entry, log_temperatura_bateria[i].id);
		strcat(entry, ",");
		strcat(entry, log_temperatura_bateria[i].time_stamp);
		strcat(entry, ",%f\n");
		fprintf(datos_file, entry, log_temperatura_bateria[i].valor);
	}
	fclose(datos_file);
}

void estado_de_carga(){
	int value = get_estado_de_carga();
	value = (validar_estado_de_carga(value)==1) ? value : -1;
	timer = time(NULL);
	tm_info = localtime(&timer);
	strftime(time_buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
	strcpy(log_estado_de_carga_bateria[nc1].id, "P1");
	strcpy(log_estado_de_carga_bateria[nc1].time_stamp, time_buffer);
	log_estado_de_carga_bateria[nc1].valor = (float)value;
}

int get_estado_de_carga(){
    printf("Obteniendo valores de BQ28Z610\n");
    char inst[8] = "00000000";
    int value = 0;
    readBytes1 = write(deviceHandle1, inst, 8);
    
    // read success
    readBytes1 = read(deviceHandle1, data1, 16);
    if (readBytes1 != 16)
	{
		printf("Error: Datos no recibidos!\n");
		value = -1;
	}
	else
	{
		for(int i=0; i<readBytes1; i++){
			int a = (int)pow(2,i);
			int b = data1[i]-'0';
			value += a*b;
		}
    	}
    printf("BQ28Z610 respuesta: %d\n", value);
    return value;
}

int validar_estado_de_carga(int valor){
	return ((valor>=0)&&(valor<=65535));
}

void voltaje_de_baterias(){
	float value = get_voltaje_de_baterias();
	value = (validar_voltaje_de_baterias(value)==1) ? value : -1.0;
	timer = time(NULL);
	tm_info = localtime(&timer);
	strftime(time_buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
	strcpy(log_voltaje_bateria[nc2].id, "P2");
	strcpy(log_voltaje_bateria[nc2].time_stamp, time_buffer);
	log_voltaje_bateria[nc2].valor = value;
}

float get_voltaje_de_baterias(){
    printf("Obteniendo valores de INA3221\n");
    char inst[8] = "00000001";
    float value = 0.0;
    readBytes1 = write(deviceHandle1, inst, 8);

    // read success
    readBytes1 = read(deviceHandle1, data1, 16);
    if (readBytes1 != 16)
	{
		printf("Error: Datos no recibidos!\n");
		value = -1;
	}
	else
	{
		int comp2_value = binTwosComplementToInt(data1, 16);
		int f_comp2_value = (float) comp2_value;
		value = f_comp2_value/200.0;
    	}
    printf("INA3221 respuesta: %f\n", value);
    return value;
}

int validar_voltaje_de_baterias(float valor){
	return ((valor>=-16400.0)&&(valor<=16400.0));
}

void temperatura_de_baterias(){
	float value = get_temperatura_de_baterias();
	value = (validar_temperatura_de_baterias(value)==1) ? value : -1.0;
	timer = time(NULL);
	tm_info = localtime(&timer);
	strftime(time_buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
	strcpy(log_temperatura_bateria[nc3].id, "P3");
	strcpy(log_temperatura_bateria[nc3].time_stamp, time_buffer);
	log_temperatura_bateria[nc3].valor = value;
}

float get_temperatura_de_baterias(){
    printf("Obteniendo valores de TMP101\n");
    char inst[8] = "00000010";
    float value = 0.0;
    readBytes1 = write(deviceHandle1, inst, 8);

    // read success
    readBytes1 = read(deviceHandle1, data1, 16);
    if (readBytes1 != 16)
	{
		printf("Error: Datos no recibidos!\n");
		value = -1;
	}
	else
	{
		int comp2_value = binTwosComplementToInt(data1, 12);
		int f_comp2_value = (float) comp2_value;
		value = f_comp2_value/16.0;
    	}
    printf("TMP101 respuesta: %f\n", value);
    return value;
}

int validar_temperatura_de_baterias(float valor){
	return ((valor>=-128.0)&&(valor<=128.0));
}

void enviar_img(char *fileName){
    FILE *file = fopen(fileName, "r");
	int c;
	char img_chr[1];

    if (file == NULL) return NULL; //could not open file
    
    while ((c = fgetc(file)) != EOF) {
		img_chr[0] = (char)c;
		readBytes2 = write(deviceHandle2, img_chr, 1);
    }
}

void enviar_datos_a_transmitir(int tipo){
	printf("Conectando con modulo de comunicaciones\n");
    char inst[8] = "00000000";
	readBytes2 = write(deviceHandle2, inst, 8);

	char value[64];
	char pixel[32] = "11010101110101011101010111010101";

	// read success
   	readBytes2 = read(deviceHandle2, data2, 8);
	if (readBytes2 != 8)
	{
		printf("Error: Datos no recibidos!\n");
	}
	else
	{
		printf("Resuesta del modulo de comunicaciones: %s\n", data2);
		if(strcmp(data2,"00000001\3")==0){
			switch(tipo){
				// Logs
				case 0:
					printf("Enviando 6.61 KBytes de logs\n");
					timer = time(NULL);
					tm_info = localtime(&timer);
					strftime(time_buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
					printf("At time: ");
					puts(time_buffer);

					for(int i=0; i<n_estado_carga; i++){
						readBytes2 = write(deviceHandle2, log_estado_de_carga_bateria[i].id, 3);
						readBytes2 = write(deviceHandle2, log_estado_de_carga_bateria[i].time_stamp, 26);
						snprintf(value, sizeof value, "%f", log_estado_de_carga_bateria[i].valor);
						readBytes2 = write(deviceHandle2, value, 8);
					}

					for(int i=0; i<n_voltaje; i++){
						readBytes2 = write(deviceHandle2, log_voltaje_bateria[i].id, 3);
						readBytes2 = write(deviceHandle2, log_voltaje_bateria[i].time_stamp, 26);
						snprintf(value, sizeof value, "%f", log_voltaje_bateria[i].valor);
						readBytes2 = write(deviceHandle2, value, 8);
					}

					for(int i=0; i<n_temperatura; i++){
						readBytes2 = write(deviceHandle2, log_temperatura_bateria[i].id, 3);
						readBytes2 = write(deviceHandle2, log_temperatura_bateria[i].time_stamp, 26);
						snprintf(value, sizeof value, "%f", log_temperatura_bateria[i].valor);
						readBytes2 = write(deviceHandle2, value, 8);
					}
						
					printf("Transmision finalizada\n");
					timer = time(NULL);
					tm_info = localtime(&timer);
					strftime(time_buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
					printf("At time: ");
					puts(time_buffer);
					break;
				case 1:
					printf("Enviando imagen de 900 KBytes\n");
					timer = time(NULL);
					tm_info = localtime(&timer);
					strftime(time_buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
					printf("At time: ");
					puts(time_buffer);

					enviar_img(img_path);
						
					printf("Transmision finalizada\n");
					timer = time(NULL);
					tm_info = localtime(&timer);
					strftime(time_buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
					printf("At time: ");
					puts(time_buffer);
					break;
					/* printf("Enviando imagen de 900 KBytes\n");
					timer = time(NULL);
					tm_info = localtime(&timer);
					strftime(time_buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
					printf("At time: ");
					puts(time_buffer);

					for(int i=0; i<740; i++){
						for(int j=0; j<45; j++){
							readBytes2 = write(deviceHandle2, pixel, 16);
						}
					}
						
					printf("Transmision finalizada\n");
					timer = time(NULL);
					tm_info = localtime(&timer);
					strftime(time_buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
					printf("At time: ");
					puts(time_buffer);
					break; */
				default: break;
			}
		}
    }
}