#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>


typedef struct{
    char name[50];
    struct tm *time;
    int value;
}info;

typedef struct{
    char name[100];
    char value[100];
}EnvVar;

int index_log_array = 0;
info log_array[1000];

int script_mode_flag = 0; // 0 = off, 1 = on, default off

void storeInfo(info in){
    log_array[index_log_array] = in;
    index_log_array++;
}

char *cshell_read_line(void){

  char *line = NULL;
  ssize_t bufsize = 0; //getline allocate a buffer

  if (getline(&line, &bufsize, stdin) == -1){
    if (feof(stdin)){
        exit(EXIT_SUCCESS);  // test end of file
    }
    else{
        perror("readline");
        exit(EXIT_FAILURE);
    }
  }

  return line;
}

int index_EV = 0;
EnvVar environment_variable[50];

void assign_helper(EnvVar in){
    if(index_EV == 0){ //first one
        environment_variable[0] = in;
        index_EV++;
    }
    else{
        for(int k = 0; k < index_EV; k++){ //repeating name
            if(strcmp(in.name, environment_variable[k].name) == 0){
                strcpy(environment_variable[k].value, in.value);
                return;
            }
        }
        environment_variable[index_EV] = in; //no repeat found, add new
        index_EV++;
    }
}

#define delimiters "\t\r\n "
void cshell_assign_variable(char *line){

    info temp;
    time_t rawtime;
    struct tm *store;
    time( &rawtime );
    store = localtime( &rawtime );
    char *lineIN = strtok(line, "\n");
    strcpy(temp.name, lineIN);
    temp.time = store;

    int pos = 0;
    char *tokens[10];
    char *token;

    char *tokens2[10];
    char *token2;

    token2 = strtok(line, delimiters);
    while (token2 != NULL) {
        tokens[pos] = token2;
        pos++;
        token2 = strtok(NULL, delimiters);
    }

    tokens2[pos] = NULL;
    
    if(pos > 1){
        temp.value = 1;
        storeInfo(temp);
        printf("Variable value expected\n");
    }
    else if (line == NULL) {
        temp.value = 1;
        storeInfo(temp);
        printf("cshell: expected argument to \"$var=<arg0>\"");
    }
    else{
        pos = 0;
        token = strtok(line, "=");
        token = token+1; //remove $ symbol
        int len = strlen(token);

        if(len == 0){ //no var name given
            printf("cshell: expected argument to \"$var=<arg0>\"\n");
            temp.value = 1;
            storeInfo(temp);
            return;
        }
    
        while(token != NULL){
            tokens[pos] = token;
            pos++;
            token = strtok(NULL, "=");
        }

        if(pos == 1){ //no value given
            printf("cshell: expected argument to \"$var=<arg0>\"\n");
            temp.value = 1;
            storeInfo(temp);
            return;
        }
        tokens[pos] = NULL; // null terminating 

        temp.value = 0;
        EnvVar temp;
        strcpy(temp.name, tokens[0]);
        strcpy(temp.value, tokens[1]);
        assign_helper(temp);
    }

    storeInfo(temp);
    return;
}

char **cshell_split_line(char *line){
    int pos = 0;
    int buf = 64;
    char **tokens = malloc(buf * sizeof(char*));
    char *token;

    if (!tokens){
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }
    else if(line[0] == '$'){ //set enviro variable
        cshell_assign_variable(line);
        tokens[0] = NULL;
    }
    else{
        token = strtok(line, delimiters); // break down string if contain delimiters
        while (token != NULL) {
            tokens[pos] = token;
            pos++;

         char **cshell_split_line(char *line){
    int pos = 0;
    int buf = 64;
    char **tokens = malloc(buf * sizeof(char*));
    char *token;

    if (!tokens){
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }
    else if(line[0] == '$'){ //set enviro variable
        cshell_assign_variable(line);
        to   }

            token = strtok(NULL, delimiters);
        }

        tokens[pos] = NULL; // null terminating 
    }
    return tokens;
}

int cshell_launch(char **args) //only for non build in commands
{
    pid_t pid, wpid;
    int status;
    int fds[2];
    
    char buf[100];
    if(pipe(fds) == -1){
        perror("pipe");
        return(-1);
    }
    memset(buf,0,100);

    info temp;
    time_t rawtime;
    struct tm *store;
    time( &rawtime );
    store = localtime( &rawtime );
    strcpy(temp.name, args[0]);

    pid = fork();
    if (pid == 0){ // Child process

        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        dup2(fds[1], STDERR_FILENO);
        close(fds[1]);

        if (execvp(args[0], args) == -1) { //find out how to save info only when SUCCESS
            perror("~");
            //printf("Missing keyword or command, or permission problem\n");
        }
        exit(EXIT_FAILURE);
    } 
    else if (pid > 0) { //Parent Process

        wpid = wait(NULL);
        close(fds[1]);
        while(read(fds[0], buf, sizeof(buf)!=0)){
            if(strcmp(buf,"~") == 0){
                printf("Missing keyword or command, or permission problem\n");
                temp.value = 1;
                temp.time = store;
                storeInfo(temp);
                return 1;
            }
            else{
                temp.value = 0;
                printf("%s", buf);
            }
        }
        temp.time = store;
        storeInfo(temp);
        close(fds[0]);
    }
        
    else{ //pid < 0 forking error
        perror("cshell");
    }

    return 1;
}

int cshell_theme(char **args);
int cshell_exit(char **args);
int cshell_print(char **args);
int cshell_log(char **args);

char *buildin_str[] = {"theme", "print", "exit", "log"};

int cshell_num_buildins(){
    return sizeof(buildin_str)/ sizeof(char *);
}

int (*buildin_func[]) (char **) = { //call build in functions
  &cshell_theme,
  &cshell_print,
  &cshell_exit,
  &cshell_log
};

int cshell_execute(char **args){
    int i ;
    if (args[0] == NULL){ //no command
        return 1;
    }

    for(i = 0; i < cshell_num_buildins(); i++){
        if (strcmp(args[0], buildin_str[i]) == 0) //if we get buildins commands
            return (*buildin_func[i])(args);
    }
    return cshell_launch(args); //if not run launch (non build in)
}


int cshell_num_builtins() {
  return sizeof(buildin_str) / sizeof(char *);
}

//flag for theme
//0 : normal
//1 : red
//2 : green
//3 : blue
int theme_flag = 0;

int cshell_theme(char **args){

    info temp;
    time_t rawtime;
    struct tm *store;
    time( &rawtime );
    store = localtime( &rawtime );
    strcpy(temp.name, "theme");
    temp.time = store;

    if (args[1] == NULL) {
    printf("cshell: expected argument to \"theme <color>\"\n");
    }
    else{
        if(strcmp(args[1], "red") == 0){
            printf("\033[1;31m");
            theme_flag = 1;
            temp.value = 0;
        }
        else if(strcmp(args[1], "green") == 0){
            printf("\033[0;32m");
            theme_flag = 2;
            temp.value = 0;
        }
        else if(strcmp(args[1], "blue") == 0){
            printf("\033[0;34m");
            theme_flag = 3;
            temp.value = 0;
        }
        else{
            printf("unsupported theme: %s\n", args[1]);
            temp.value = 1;
        }
    }
    storeInfo(temp);
  
  return 1;
}

int cshell_print(char **args){  

    info tempInfo;
    time_t rawtime;
    struct tm *store;
    time( &rawtime );
    store = localtime( &rawtime );
    strcpy(tempInfo.name, "print");
    tempInfo.time = store;

    if (args[1] == NULL) {
        printf("cshell: expected argument to \"print <arg0>...\"");
        tempInfo.value = 1;
    }
    else{
        int count = 0; 
        while(args[++count] != NULL);
   
        for(int i = 1; i < count; i++){ // 1 = first argument
            if(args[i][0] == '$'){
                char *temp = args[i]+1;

                if(index_EV == 0){
                    printf("Variable not Found ");
                    tempInfo.value = 1;
                }
                
                for(int k = 0; k < index_EV; k++){
                    if(strcmp(temp, environment_variable[k].name) == 0){
                        printf("%s ", environment_variable[k].value);
                        tempInfo.value = 0;
                    }
                    else{
                        printf("Variable not Found ");
                        tempInfo.value = 1;
                    }
                }        
            }
            else{
                tempInfo.value = 0;
                printf("%s ", args[i]);
            }
        }
    }
    printf("\n");

    storeInfo(tempInfo);
    return 1;
}

int cshell_log(char **args)
{
    info temp;
    time_t rawtime;
    struct tm *store;
    time( &rawtime );
    store = localtime( &rawtime );
    strcpy(temp.name, "log");
    temp.time = store;

    if(index_log_array == 0){
        printf("Not logs found \n");
        temp.value = 1;
    }
    for(int i = 0; i < index_log_array; i++){
        printf("%s %s %d \n", asctime(log_array[i].time), log_array[i].name, log_array[i].value);
        temp.value = 0;
    }

    storeInfo(temp);

    return 1;
}

int cshell_exit(char **args)
{
    printf("%s", "Bye!\n");
    return 0;
}

void init_loop()
{
    char *line;
    char **args;
    int status;

    do{
        printf("cshell$ ");

        printf("\033[0m"); //reset colour

        line = cshell_read_line();
        if(line == NULL){
            continue;
        }

        if(theme_flag == 1){
            printf("\033[1;31m");
        }
        else if(theme_flag == 2){
            printf("\033[0;32m");
        }
        else if(theme_flag == 3){
            printf("\033[0;34m");
        }
        else{
            printf("\033[0m");
        }

        args = cshell_split_line(line);
        status = cshell_execute(args);

        free(line);
        free(args);
    }while (status);
}

int script_mode(char* filename){
    FILE *fptr;
    ssize_t read;
    char * line = NULL;
    size_t len = 0;
    char **args;
    int status;

    fptr = fopen(filename,"r");
    if (!fptr) {
        printf("Unable to read script file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fptr)) != -1){
        args = cshell_split_line(line);
        cshell_execute(args);
    }
    fclose(fptr);

    args = cshell_split_line("exit");
    cshell_execute(args);

    return 0;
}

int main(int argc, char *argv[]){

    char *file;
    if(argv[1] != NULL){//enter script mode
        file = argv[1];
        script_mode_flag = 1;
        script_mode(file);
    }

    if(script_mode_flag == 0){
        init_loop();
    }
    
    return 0;
} 

//find out how to save info when exec success only
//add logs script to every other function 
    // info temp;
    // time_t rawtime;
    // struct tm *store;
    // time( &rawtime );
    // store = localtime( &rawtime );
    // strcpy(temp.name, "print");
    // temp.time = store;
    // temp.value = 0;
    // storeInfo(temp);
    
//pipe