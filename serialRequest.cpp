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
#include <vector>
#include <math.h>

#define PORT 8081
#define IP_ADDRESS "18.219.115.13" // Public IP for the EC2 instance
#define MODULE_LOCATION "0102"

using namespace std;

int sendMessage(char* message);
char* makeFrame(vector<int> data);
string int_to_hex(int n);
vector<int> readArduino();


int main() {    
    vector<int> data;
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
    if(inet_pton(AF_INET, IP_ADDRESS, &serv_addr.sin_addr)<=0) {
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

char* makeFrame(vector<int> data) {
    // Current date/time based on current system
    time_t now = time(0);
    // Convert now to tm struct
    tm *ltm = localtime(&now);

    // Frame start
    string request_frame = "c0";
    // Device ID
    request_frame += "000000";
    request_frame += int_to_hex(data[0]);
    // Command
    // ** Only one command at the time **
    request_frame += "01";
    // Package Length
    // ** Will remain the same size with only one command **
    request_frame += "16";

    // Convert date and time to string
    request_frame += int_to_hex((1900 + ltm->tm_year)-2000);
    request_frame += int_to_hex(1 + ltm->tm_mon);
    request_frame += int_to_hex(ltm->tm_mday);
    request_frame += int_to_hex(ltm->tm_hour);
    request_frame += int_to_hex(ltm->tm_min);
    request_frame += int_to_hex(ltm->tm_sec);

    // Coordinates X and Y (module location)
    request_frame += MODULE_LOCATION;
    // Destination coordinates
    request_frame += int_to_hex(data[1]);
    request_frame += int_to_hex(data[2]);

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

vector<int> readArduino() {
    FILE *file;
    vector<int> data, data_final;
    int c;

    file = fopen("/dev/ttyACM0","rb");  // Opening device file
    
    while(1) {
        if (file) {    
            while ((c = getc(file)) != EOF) {
                char tmp = c;
                putchar(c);                
                // Stores read data into int list
                if(c != 32 && c != 13 && c != 10) {                    
                    data.push_back(atoi(&tmp));
                }
            }                
            if (data.empty()) {
                cout << "... ";
                sleep(1);
            } else
                break;
        }
        if (!data.empty())
            break;        
    }
    fclose(file);
    
    cout << "Data read from Arduino:\n";

    // Converts to actual numbers
    for (int i = 0, digit_length = data.size()/3; i < data.size(); i += digit_length) {
        double digit_acc = 0;

        for (int p = 0, j = i + digit_length-1; p < digit_length; p++, j--) {
            digit_acc += data[j] * pow(10.0, p);
        }

        data_final.push_back((int) digit_acc);

        cout << data_final[data_final.size()-1] << endl;
    }
    cout << endl;

    return data_final;
}