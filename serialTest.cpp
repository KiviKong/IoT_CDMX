#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    FILE *file;
    int c;
    file = fopen("/dev/ttyACM0","rb");  //Opening device file
    
    while(1) {
        if (file) {    
            while ((c = getc(file)) != EOF)
                putchar(c);

            sleep(1);
        }
    }
    fclose(file);
}