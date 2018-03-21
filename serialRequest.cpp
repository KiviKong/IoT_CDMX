#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <iostream>

using namespace std;

int* readArduino();
string makeFrame();
string int_to_hex(int n);

int main() {    
    int* data;
    string request_frame;

    // ** Wait until we have some input from arduino **
    // data = readArduino();

    request_frame = makeFrame(data);

    cout << "Constructed Frame:\n" << request_frame << endl;

    // ** Make request to AWS **
    
}

string makeFrame(int* data) {
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
    //  ** Only one command at the time **
    request_frame += "01";
    // Package Length
    // ** Will remain the same size with only one command **
    request_frame += to_string(16);

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

    return request_frame;
}

string int_to_hex(int num) {
    char* result = new char;
    cout << "Number to convert: " << num << endl;
    
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