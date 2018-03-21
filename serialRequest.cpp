#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define ipAddress "18.222.104.109" // Public IP for the EC2 instance

using namespace std;

int sendMessage(char* message);
char* makeFrame(int* data);
string int_to_hex(int n);
int* readArduino();


int main() {    
    int* data;
    char* request_frame;

    // ** Wait until we have some input from arduino **
    // data = readArduino();

    request_frame = makeFrame(data);

    cout << "Constructed Frame to be sent:\n" << request_frame << endl;

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

char* makeFrame(int* data) {
    // Current date/time based on current system
    time_t now = time(0);
    // Convert now to tm struct
    tm *ltm = localtime(&now);

    // Frame start
    string request_frame = "c0";
    // Device ID
    // ** Change for ID from data **
    request_frame += "e2a11e11";
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
    request_frame += "01020304";

    // Convert String to char array
    char* final_frame = new char[request_frame.length()];
    strcpy(final_frame, request_frame.c_str());

    return final_frame;
}

string int_to_hex(int num) {
    char* result = new char;

    if (num < 10)
        sprintf(result, "0%x", num);
    else
        sprintf(result, "%x", num);

    return string(result);
}

int* readArduino() {
    FILE *file;
    int c, i = 0;
    int* data = new int[7];

    file = fopen("/dev/ttyACM0","rb");  // Opening device file
    
    while(1) {
        if (file) {    
            while ((c = getc(file)) != EOF) {
                putchar(c);
                // Stores read data into char array
                data[i] = c;
                i++;
            }                

            sleep(1);
        }
    }
    fclose(file);
    return data;
}