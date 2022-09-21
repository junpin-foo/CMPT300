#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main() {
    long total;
    long free;
    long buf;
    long cache;
    long slab;

    FILE *this;
    char toPrint[256] = {0};
    this = fopen("/proc/meminfo", "r"); //open file in this location

    if (this == NULL) {
        perror("Error opening file: ");
    }
    
    while (fgets(toPrint, sizeof(toPrint), this) != NULL){ //find values of this titles
        sscanf(toPrint, "MemTotal: %ld kB", &total);
        sscanf(toPrint, "MemFree: %ld kB", &free);
        sscanf(toPrint, "Buffers: %ld kB", &buf);
        sscanf(toPrint, "Cached: %ld kB", &cache);
        sscanf(toPrint, "Slab: %ld kB", &slab);

    }
    // printf("total %ld ..." , total);
    // printf("free %ld ..." , free);
    // printf("buf %ld ..." , buf);
    // printf("cache %ld ..." , cache);
    // printf("slab %ld ..." , slab);

    double eqn = (total-free-buf-cache-slab); //equation to solve memory utilization
    double eqn2 = eqn/total * 100;
    printf("%.2f\n", eqn2 ); 
  
    fclose(this);

    // char toPrint2;
    // this = fopen("/proc/meminfo", "r");
    // toPrint2 = fgetc(this);
    // while (toPrint2 != EOF)
    // {
    //     printf ("%c", toPrint2);
    //     toPrint2 = fgetc(this);
    // }
  
    // fclose(this);

    return 0;
} 