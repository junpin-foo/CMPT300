#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>


int main() {
    DIR *dir;
    struct dirent *dirp;
    char buf[256];

    dir = opendir("/proc");
    if(dir){
        while ((dirp = readdir(dir)) != NULL) // scan through the entire directory /proc

            if(isdigit(dirp->d_name[0])) //check if name is a number or not
                printf("%s\n",dirp->d_name);
            

        closedir(dir);
    }
    else{
        perror("Directory doesn't exist.");
    }
    return 0;
} 