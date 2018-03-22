#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define ipAddress "18.219.115.13" // Public IP for the EC2 instance

using namespace std;

int sendMessage(char* message);
char* makeFrame(char* data);
string int_to_hex(int n);
char* readArduino();


int main() {    
    char* data;
    char* request_frame;

    // ** Wait until we have some input from arduino **
    data = readArduino();

    request_frame = makeFrame(data);

    // Send frame as request to AWS server
    sendMessage(request_frame);
}

int sendMessage(char* message) {

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, ipAddress, &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    send(sock , message , strlen(message) , 0 );
    printf("Message sent\n");
    return 0;
}

char* makeFrame(char* data) {
    // Current date/time based on current system
    time_t now = time(0);
    // Convert now to tm struct
    tm *ltm = localtime(&now);

    // Frame start
    string request_frame = "c0";
    // Device ID
    // ** Change for ID from data **
    request_frame += "000000";

    char tmp[1];
    tmp[0] = data[0];
    int tmp_1 = atoi(tmp)*10;    
    tmp[0] = data[1];
    int tmp_2 = atoi(tmp)*1;    
    int id = tmp_1 + tmp_2;

    request_frame += int_to_hex(id);
    // Command
    // Only one command at the time
    request_frame += "01";
    // Package Length
    // Will remain the same size with only one command
    request_frame += "16";

    // Convert date and time to string
    request_frame += int_to_hex((1900 + ltm->tm_year)-2000);
    request_frame += int_to_hex(1 + ltm->tm_mon);
    request_frame += int_to_hex(ltm->tm_mday);
    request_frame += int_to_hex(ltm->tm_hour);
    request_frame += int_to_hex(ltm->tm_min);
    request_frame += int_to_hex(ltm->tm_sec);

    // Coordinates
    // ** Change with coordinates from data **

    // Coordinates X and Y (module location)
    request_frame += "0102";
    // Coordinate X (destination)
    tmp[0] = data[2];
    tmp_1 = atoi(tmp)*10;    
    tmp[0] = data[3];
    tmp_2 = atoi(tmp)*1;    
    int coord_x = tmp_1 + tmp_2;
    request_frame += int_to_hex(coord_x);
    //Coordinate Y (destination)
    tmp[0] = data[4];
    tmp_1 = atoi(tmp)*10;    
    tmp[0] = data[5];
    tmp_2 = atoi(tmp)*1;    
    int coord_y = tmp_1 + tmp_2;    
    request_frame += int_to_hex(coord_y);

    // Frame end
    request_frame += "c0";

    // Convert String to char array
    char* final_frame = new char[request_frame.length()];
    strcpy(final_frame, request_frame.c_str());

    cout << "\nConstructed Frame to be sent:\n" << request_frame << endl;

    return final_frame;
}

string int_to_hex(int num) {
    char* result = new char;

    if (num < 16)
        sprintf(result, "0%x", num);
    else
        sprintf(result, "%x", num);

    return string(result);
}

char* readArduino() {
    FILE *file;
    int c, i = 0;
    int* data = new int[20];
    char* data_final = new char[7];
    char* tmp = new char;

    file = fopen("/dev/ttyACM0","rb");  // Opening device file
    
    while(1) {
        if (file) {    
            while ((c = getc(file)) != EOF) {
                putchar(c);
                // Stores read data into char array
                data[i] = c;
                i++;
            }                
            if (i == 0) {
                cout << "... ";
                sleep(1);
            } else
                break;
        }
        if (i > 0)
            break;        
    }
    fclose(file);

    // Get only the data we need
    // ** Temporal, only works for two-digit numbers **
    sprintf(tmp, "%c", data[1]);
    data_final[0] = tmp[0];
    sprintf(tmp, "%c", data[2]);
    data_final[1] = tmp[0];
    sprintf(tmp, "%c", data[4]);
    data_final[2] = tmp[0];
    sprintf(tmp, "%c", data[5]);
    data_final[3] = tmp[0];
    sprintf(tmp, "%c", data[7]);
    data_final[4] = tmp[0];
    sprintf(tmp, "%c", data[8]);
    data_final[5] = tmp[0];

    cout << "Data read from Arduino:\n";

    for(i = 0; i < 6; i+=2) {        
        cout << data_final[i] << data_final[i+1] << endl;
    }

    return data_final;
}