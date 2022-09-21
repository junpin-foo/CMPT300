#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "secret_headers.h"
#include <time.h>
#include <string.h>


struct info{
	char *name;
	uid_t userID;
	gid_t groupID;
	mode_t st_mode;
	nlink_t st_nlink;
	ino_t st_ino;
	off_t st_size;
    time_t st_mtime2;
};

struct node {
   struct info data;
   struct node *next;
};

struct node *head = NULL;
struct node *current = NULL;


//display the list
void printList() {
   struct node *ptr = head;
   printf("%s\n", "[" );
	
   //start from the beginning
   while(ptr != NULL) {
      printf("(%s) ",ptr->data.name);
      ptr = ptr->next;
   }
	
   printf("%s\n", "]" );
}

void insertFirst(struct info data) {
   //create a link
   struct node *link = (struct node*) malloc(sizeof(struct node));
	
   link->data = data;

   //point it to old first node
   link->next = head;
	
   //point first to new first node
   head = link;
}

struct node* deleteFirst() {

   //save reference to first link
   struct node *tempLink = head;
	
   //mark next to first link as first 
   head = head->next;
	
   //return the deleted link
   return tempLink;
}

int length() {
   int length = 0;
   struct node *current;
	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }
	
   return length;
}

void sort(){
	struct node *current;
  	struct node *next;
	struct info temp;

	int len = length();

	for (int i = 0; i < len; i++) {
		current = head;
		next = head->next;

		for (int j = i + 1; j < len; j++) {

			// swapping if not lexicographical order
			if (strcmp(current->data.name, next->data.name) > 0) {
				temp = current->data;
				current->data = next->data;
				next->data = temp;
			}

			current = current->next;
         	next = next->next;
		}
	}
}

int isEmpty() {
   if(head == NULL) return 1;
   else return 0;
}

//LINKED LIST ^^^^^^^^^^^^^^^^^^^
void recursiveR(const char *dir);
void myls(const char *dir);
void print_myls();
int i = 0, l = 0, R = 0; //options

//Dir function ^^^^^^^^^^^^^^^^^^^^

void recursiveR(const char *dir){

	struct dirent *d;
	DIR *dh;

    if ((dh = opendir(dir)) == NULL) {
        return;
    }

	printf("\n%s:\n", dir); //dir name

    while ((d = readdir(dh)) != NULL) {

        if(strcmp(d->d_name, "..") == 0 || d->d_name[0] == '.') 
			continue;

		char path_location[1024] = {0};
	
		strcat(path_location, dir);
		strcat(path_location, "/");
		strcat(path_location, d->d_name);

		struct info temp;
		struct stat sb;

		if(stat(path_location, &sb) == -1){
			puts("Error stat \n");
			exit(EXIT_FAILURE);
		}
		temp.name = d->d_name; //assign info
		temp.userID = sb.st_uid;
		temp.groupID = sb.st_gid;
		temp.st_mode = sb.st_mode;
		temp.st_nlink = sb.st_nlink;
		temp.st_ino = sb.st_ino;
		temp.st_size = sb.st_size;
		temp.st_mtime2 = sb.st_mtime;

		insertFirst(temp);
    }

	sort();
	print_myls();

	DIR *dh2 = opendir(dir);

	size_t length = strlen(dir);
	char *path = malloc(length + 1 + NAME_MAX);
	strcpy(path, dir);
	path[length] = '/';

	while((d = readdir(dh2)) != NULL){
		memset(path, 0, 100);
        if (d->d_type == DT_DIR && d->d_name[0] != '.' && strcmp(d->d_name, "..") != 0) { //directory
			strcat(path, dir);
            strcat(path, "/");
            strcat(path, d->d_name);

            recursiveR(path);
        }
	}

	closedir(dh);
    closedir(dh2); 
}

void myls(const char *dir){

	if(R){
		recursiveR(dir);
		return;
	}

    struct dirent *d;
	DIR *dh = opendir(dir);
	
	size_t length = strlen(dir);
	char *path = malloc(length + 1 + NAME_MAX);
	strcpy(path, dir);
	path[length] = '/';
	
    if (!dh){ //is a single file
		struct info temp;
		struct stat sb;

		if(stat(dir, &sb) == -1){
			puts("Error  : Nonexistent files or directories");
			exit(EXIT_FAILURE);
		}

		strcpy(path, dir);
		temp.name = path;
		temp.userID = sb.st_uid;
		temp.groupID = sb.st_gid;
		temp.st_mode = sb.st_mode;
		temp.st_nlink = sb.st_nlink;
		temp.st_ino = sb.st_ino;
		temp.st_size = sb.st_size;
		temp.st_mtime2 = sb.st_mtime;

		insertFirst(temp);
		print_myls();
		return;
	}

    while ((d = readdir(dh)) != NULL)
	{
		struct info temp;
		struct stat sb;
		strcpy(path + length + 1, d->d_name);
		if(stat(path, &sb) == -1){ //stat need path not name not working for ..
			puts("Error stat");
			exit(EXIT_FAILURE);
		}

		if(strcmp(d->d_name, "..") == 0 || d->d_name[0] == '.') continue;

		temp.name = d->d_name; //assign info
		temp.userID = sb.st_uid;
		temp.groupID = sb.st_gid;
		temp.st_mode = sb.st_mode;
		temp.st_nlink = sb.st_nlink;
		temp.st_ino = sb.st_ino;
		temp.st_size = sb.st_size;
		temp.st_mtime2 = sb.st_mtime;

		insertFirst(temp);
	}
	sort();
	print_myls();
	closedir(dh);
}

void print_myls(){

	while(!isEmpty()){

		struct node *temp_node = deleteFirst();
		struct info temp_info = temp_node->data;

		if(i){
			printf("%ld ", temp_info.st_ino);
		}

		if(l){
			// protection, from stackoverflow https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
			printf("%s", (S_ISDIR(temp_info.st_mode)) ? "d" : "-");
			printf("%s", (temp_info.st_mode & S_IRUSR) ? "r" : "-");
			printf("%s", (temp_info.st_mode & S_IWUSR) ? "w" : "-");
			printf("%s", (temp_info.st_mode & S_IXUSR) ? "x" : "-");
			printf("%s", (temp_info.st_mode & S_IRGRP) ? "r" : "-");
			printf("%s", (temp_info.st_mode & S_IWGRP) ? "w" : "-");
			printf("%s", (temp_info.st_mode & S_IXGRP) ? "x" : "-");
			printf("%s", (temp_info.st_mode & S_IROTH) ? "r" : "-");
			printf("%s", (temp_info.st_mode & S_IWOTH) ? "w" : "-");
			printf("%s", (temp_info.st_mode & S_IXOTH) ? "x" : "-");

			printf(" %ld ", temp_info.st_nlink ); //number of hardlinks

			struct passwd *pw = getpwuid(temp_info.userID); //owner name
			printf(" %s ", pw->pw_name );

			struct group  *gr = getgrgid(temp_info.groupID); //group name
			printf(" %s ", gr->gr_name);

			printf(" %8ld ", temp_info.st_size); //size in bytes

			time_t modi_time = temp_info.st_mtime2;
			struct tm temp;
			localtime_r(&modi_time, &temp);

			switch(temp.tm_mon + 1){ //month mmm
				case 1:
					printf("%s","Jan");
					break;
				case 2:
					printf("%s","Feb");
					break;
				case 3:
					printf("%s","Mar");
					break;
				case 4:
					printf("%s","Apr");
					break;
				case 5:
					printf("%s","May");
					break;
				case 6:
					printf("%s","Jun");
					break;
				case 7:
					printf("%s","Jul");
					break;
				case 8:
					printf("%s","Aug");
					break;
				case 9:
					printf("%s","Sep");
					break;
				case 10:
					printf("%s","Oct");
					break;
				case 11:
					printf("%s","Now");
					break;
				case 12:
					printf("%s","Dec");
					break;
			}
			printf(" %2d ", temp.tm_mday); //day dd

			printf(" %4d ", temp.tm_year + 1900); //year yyyy

			printf(" %02d:", temp.tm_hour); //hour hh

			printf("%02d ", temp.tm_min); //min mm
		}

		printf("%s\n", temp_info.name);

	}
}

int myls_flag = 0; //how many time myls is called

int main (int argc, char *argv[]){

    if (argc == 1){ //current directory
		myls(".");
		i = 0, l = 0, R = 0;
	}
    else{// as many as possible
		int option_flag = 0;

		int k = 1;
		while(k < argc){
			if (argv[k][0] == '-'){//Checking if option is passed
				char *p = (char*)(argv[k] + 1);
				while(*p){
					if(*p == 'i') i = 1;
					else if(*p == 'l') l = 1;
					else if(*p == 'R') R = 1;
					else{
						puts("Error : Unsupported Option \n");
						exit(EXIT_FAILURE);
					}
					p++;
				}
				option_flag = 1;
			}
			else{
				myls_flag++;
				myls(argv[k]);
			}
			
			k++;
		}
		if(myls_flag == 0){ //after going through all arguments, is no path given, myls_flag = 0, run myls on current dir
			myls(".");
		
		}
	}
    return 0;
}


