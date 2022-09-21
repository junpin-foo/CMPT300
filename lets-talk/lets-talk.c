#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>  
#include <string.h>
#include "list.h"
#include <netdb.h>
#include <sys/types.h>
#include <time.h>

#define MAXBUFF 4002

#define key 20 // for encryption 

List *list_in;
List *list_out;

struct sockaddr TEMPaddr_global; //sockaddr_in

char *remotehost;
char *remoteport;
char *localport;
int sock_send;
int sock_recv;
int sock_send_status;

void init_list(){
    list_in = List_create();
    list_out = List_create();
}

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_status = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_status = PTHREAD_MUTEX_INITIALIZER;

struct timespec timeToWait;
struct timespec now;

pthread_t tr1, tr2, tr3, tr4, tr5;

unsigned int seconds=0;
unsigned int milliseconds=0;
unsigned int totaltime=0,count_down_time_in_secs=0,time_left=0;

clock_t startTime,countTime;

#define TIMER 2 // for status check

int status_flag = 0; //1 = on
int online_flag = 0; //1 = online, 0 = offline
int thread_flag = 0; //0 = not ran, 1 = ran

char *encrypt(char *in){
    int len = strlen(in);
    for(int i = 0; i < len; i++){
        *(in + i) = (*(in + i) + key) % 256;
    }
    return in;
}

char *decrypt(char *in){
    int len = strlen(in);
    for(int i = 0; i < len; i++){
        if(*(in + i) - key  < 0){
            *(in + i) = (*(in + i) - key) + 256;
        }
        else{
            *(in + i) = (*(in + i) - key);
        }
    }
    return in;
}

void *status_function(void *arguments){
    
    thread_flag = 1;
    clock_gettime(CLOCK_REALTIME, &now);
    timeToWait.tv_sec = now.tv_sec + TIMER;
    pthread_mutex_lock(&lock_status);
    
    pthread_cond_timedwait(&cond_status, &lock_status, &timeToWait);
   
    if(online_flag == 0){
        fflush(stdout);
        printf("Offline \n");
        fflush(stdout);
    }
    pthread_mutex_unlock(&lock_status);

    status_flag = 0;
    online_flag = 0; 
    
    return NULL;
}

void *kbInput_function(void *_args){

    while(true){
        char* kbtemp;
        char user_input[MAXBUFF];
        fgets(user_input, MAXBUFF, stdin);
        int len = strlen(user_input);
        if((len > 0)&&(user_input[len-1]=='\n'))
            user_input[len-1] = '\0';

        pthread_mutex_lock(&lock1);

        kbtemp = (char*)malloc(sizeof(char)*(len+1));
        strncpy(kbtemp,user_input,len);
        kbtemp[len] = '\0';
        
        int success = List_add(list_in, kbtemp);
        if( success == -1){
            printf("Error Adding To List...");
            exit(EXIT_FAILURE); 
        }

        pthread_cond_signal(&cond1);
        pthread_mutex_unlock(&lock1);

    }

    return NULL;
}

struct addrinfo address, *add_pointer;
struct addrinfo *Sender;

void *sender_function(void *arguments){

    memset(&address, 0 ,sizeof (struct addrinfo));
    address.ai_family = AF_INET;
    address.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(remotehost, remoteport, &address, &Sender) != 0 ){
        printf("Address not found\n");
        exit(EXIT_FAILURE); 
    }
    
    for(add_pointer = Sender; add_pointer!=NULL; add_pointer=add_pointer->ai_next){
        sock_send = socket(add_pointer->ai_family, add_pointer->ai_socktype, add_pointer->ai_protocol);
        if (sock_send ==-1)
            continue;
        
        break;
    }
    if(add_pointer==NULL){
        printf("Error creating socket \n");
        exit(EXIT_FAILURE); 
    }

    while(true){
        pthread_mutex_lock(&lock1);
        
        pthread_cond_wait(&cond1, &lock1);
        
        while(List_count(list_in) > 0){
            int bytes;
            List_first(list_in);
            char *temp = List_remove(list_in);

            int len = strlen(temp);
            if((len > 0)&&(temp[len-1]=='\n'))
                temp[len-1] = '\0';

            if(strcmp(temp, "!status") == 0){
                status_flag = 1;
                int status = pthread_create(&tr5, NULL, &status_function, NULL); 
                if(status != 0){
                    perror("Fail to create thread status");
                    exit(EXIT_FAILURE);
                }
            }

            char *encrypt_temp = encrypt(temp);// encryption

            bytes = sendto(sock_send, encrypt_temp, strlen(temp), 0, (struct sockaddr *) add_pointer->ai_addr, add_pointer->ai_addrlen);
            if (bytes < 0) {
                printf("Error Sending...");
                exit(EXIT_FAILURE); 
            }

            char *decrypt_temp = decrypt(encrypt_temp);// decryption

            if(strcmp(decrypt_temp, "!exit") == 0){
                free(temp);
                pthread_cancel(tr1);
                pthread_cancel(tr3);
                pthread_cancel(tr4);
                if(thread_flag == 1)
                    pthread_cancel(tr5);
                pthread_cancel(tr2);
                return NULL;
            }

        }
        pthread_mutex_unlock(&lock1);
    }

    return NULL;
}

struct addrinfo address, *add_pointer_recv;
struct addrinfo *Receiver;

void *receiver_function(void *arguments){

    memset(&address, 0 ,sizeof (struct addrinfo));
    address.ai_family = AF_INET;
    address.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(remotehost, localport, &address, &Receiver) != 0 ){
        printf("Address not found\n");
        exit(EXIT_FAILURE); 
    }
    
    for(add_pointer_recv = Receiver; add_pointer_recv!=NULL; add_pointer_recv=add_pointer_recv->ai_next){
        sock_recv = socket(add_pointer_recv->ai_family, add_pointer_recv->ai_socktype, add_pointer_recv->ai_protocol);
        if (sock_recv ==-1)
            continue;
        
        if(bind(sock_recv, add_pointer_recv->ai_addr, add_pointer_recv->ai_addrlen) ==-1){
            close(sock_recv);
            perror("Error binding...");
            continue;
        }
        break;
    }
    if(add_pointer_recv==NULL){
        printf("Error creating socket\n");
        exit(EXIT_FAILURE); 
    }

    char data_received[MAXBUFF];
    char *temp;
    int bytes;
    socklen_t AddrSize = sizeof (TEMPaddr_global);

    while(true){
        memset(&data_received, 0, MAXBUFF);
        bytes = recvfrom(sock_recv, data_received, MAXBUFF, 0,(struct sockaddr *) &TEMPaddr_global, &AddrSize);
        pthread_mutex_lock(&lock2);
        if (bytes == -1) {
            printf("Error Receiving...");
            exit(EXIT_FAILURE); 
        }

        temp = (char*)malloc(sizeof(char)*(bytes+1));
        strncpy(temp, data_received, bytes);
        temp[bytes] = '\0';

        char *decrypt_temp = decrypt(temp);// decryption

        if(strcmp(decrypt_temp, "!status") == 0 && status_flag == 0){ //receiver
            int bytes;
            char *encrypt_temp = encrypt(decrypt_temp);// encryption

            bytes = sendto(sock_send, encrypt_temp, strlen(encrypt_temp), 0, (struct sockaddr *) add_pointer->ai_addr, add_pointer->ai_addrlen);
            if (bytes < 0) {
                printf("Error Sending...");
                exit(EXIT_FAILURE); 
            }
        }

        else if(strcmp(decrypt_temp, "!status") == 0 && status_flag == 1){ //sender 

            fflush(stdout);
            online_flag = 1;
            printf("Online \n");   
            fflush(stdout);
            pthread_cond_signal(&cond_status); //release recv thread to run
     
        }
        else{
             List_add(list_out, decrypt_temp);
        }

        

        pthread_cond_signal(&cond2);
        pthread_mutex_unlock(&lock2);
    }
    return NULL;
}

void *output_function(){

    while(true){
        pthread_mutex_lock(&lock2);
        pthread_cond_wait(&cond2, &lock2);

        if(List_count(list_out) > 0){
            fflush(stdout);
            List_first(list_out);
            char *temp;
            temp = List_remove(list_out);
            if(strcmp(temp, "!exit") == 0){
                pthread_cancel(tr1);
                pthread_cancel(tr2);
                pthread_cancel(tr3);
                if(thread_flag == 1)
                    pthread_cancel(tr5);
                free(temp);
                pthread_cancel(tr4);
                return NULL;
            }
            fflush(stdout); //new
            printf("%s\n", temp);
            fflush(stdout);
        }
        pthread_mutex_unlock(&lock2);
    }
    return NULL;
}

void chat(){
    int kbInput, sender, receiver, output;

    init_list();

    printf("Welcome to LetS-Talk! Please type your messages now. \n");
    
        
    kbInput = pthread_create(&tr1, NULL, &kbInput_function, NULL);
    if(kbInput != 0){
        perror("Fail to create thread KBinput");
        exit(EXIT_FAILURE);
    }

    sleep(1);

    sender = pthread_create(&tr2, NULL, &sender_function, NULL);
    if(sender != 0){
        perror("Fail to create thread sender");
        exit(EXIT_FAILURE);
    }

    receiver = pthread_create(&tr3, NULL, &receiver_function, NULL);
    if(receiver != 0){
        perror("Fail to create thread receiver");
        exit(EXIT_FAILURE);
    }

    sleep(1);
    
    output = pthread_create(&tr4, NULL, &output_function, NULL); 
    if(output != 0){
        perror("Fail to create thread output");
        exit(EXIT_FAILURE);
    }

    pthread_join(tr1, NULL);
    pthread_join(tr2, NULL);
    pthread_join(tr3, NULL);
    pthread_join(tr4, NULL);
    pthread_join(tr5, NULL);

    freeaddrinfo(Sender);
    freeaddrinfo(Receiver);

    close(sock_send);
    close(sock_recv);

    List_free(list_in, free);
    List_free(list_out, free);
}


int main(int argc, char *argv[]){

    if(argc <= 3 || argc > 4) {
        printf("Usage: \n ./lets-talk <local port> <remote host> <remote port>\n");
        printf("Examples: \n ./lets-talk 3000 192.168.0.513 3001\n ./lets-talk 3000 some-computer-name 3001\n");
        return 1;
    }

    localport = argv[1];
    remotehost = argv[2];
    remoteport = argv[3];

    chat();

    return 0;
} 
