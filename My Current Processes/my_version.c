#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *this;
    char toPrint;
    this = fopen("/etc/os-release", "r"); //open the first file and check existance
    if (this == NULL) {
        perror("Error opening file: ");
    }
    toPrint = fgetc(this);
    while (toPrint != EOF) //print everything before end of file
    {
        printf ("%c", toPrint);
        toPrint = fgetc(this);
    }
  
    fclose(this);

    this = fopen("/proc/version", "r"); //same as above
    if (this == NULL) {
        perror("Error opening file: ");
    }
    toPrint = fgetc(this);
    while (toPrint != EOF)
    {
        printf ("%c", toPrint);
        toPrint = fgetc(this);
    }
  
    fclose(this);

    return 0;
} 