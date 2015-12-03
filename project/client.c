#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 257
#define NAME_SIZE 20

#define PORT 8797 //SJNAM DEFINE

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];
char file_send_name[BUF_SIZE];
int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in serv_addr;
  pthread_t snd_thread, rcv_thread;
  void * thread_return;

   if (argc != 4) {
    printf("Usage : %s <IP> <port> <name>\n", argv[0]);
    exit(1);
  }

  sprintf(name, "[%s]", argv[3]);
  sock = socket(PF_INET, SOCK_STREAM, 0);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));
//    serv_addr.sin_port = htons(PORT);

  if (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
    error_handling("connect() error");

  pthread_create(&snd_thread, NULL, send_msg, (void*) &sock);
  pthread_create(&rcv_thread, NULL, recv_msg, (void*) &sock);
  pthread_join(snd_thread, &thread_return);
  pthread_join(rcv_thread, &thread_return);
  close(sock);
  return 0;
}

void * send_msg(void * arg)   // send thread main
{
  int sock = *((int*) arg);
  char name_msg[NAME_SIZE + BUF_SIZE];
  char* ptr;
  char* token;
  FILE* fp;
  char sendbuf[BUF_SIZE];
  char tempbuf[BUF_SIZE];

  while (1) {
    fgets(msg, BUF_SIZE, stdin);

    strcpy(tempbuf, msg);

    token = strtok_r(tempbuf, " ", &ptr);

    if(strcmp(token, "/send") == 0){
      sprintf(name_msg, "%s %s", name, msg);
      write(sock, name_msg, strlen(name_msg));

      token = strtok_r(NULL, " ", &ptr);
      token[strlen(token)-1] = '\0';

      fp = fopen(token, "rb");

      usleep(100);
      while(fread(sendbuf, 1, BUF_SIZE, fp) > 0){
        write(sock, sendbuf, BUF_SIZE);
      }
      fclose(fp);
      continue;
    }
      
    else if (strcmp(token, "/fileDown") == 0){
        token = strtok_r(NULL, " ",   &ptr);

        token[strlen(token)-1] = '\0';
        printf("%s", token);
        strcpy(file_send_name, token);
    }
    //when input q or Q, then quit
    if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
      close(sock);
      exit(0);
    }

    //send name and massage
    sprintf(name_msg, "%s %s", name, msg);
    write(sock, name_msg, strlen(name_msg));
  }
  return NULL;
}

void * recv_msg(void * arg)   // read thread main
{
  int sock = *((int*) arg);
  char name_msg[NAME_SIZE + BUF_SIZE];
  int str_len;
  char file_msg[BUF_SIZE] = {0,};  //file buffer

  //whenever client recieves a message from server, print to stdout
  while (1) {
    str_len = read(sock, name_msg, BUF_SIZE);
    if (str_len == -1)
      return (void*) -1;
      
      //file download
      if(strcmp(name_msg, "file : sr->cl") ==  0) {
          //file from server
          
          FILE *fp;
          printf("받을 파일은 %s\n", file_send_name);
          
          
          if ( (fp = fopen(file_send_name, "wb")) == NULL) {
              fprintf(stderr, "Error! Cannot open file...\n");
              exit(1);
          }
          //sleep(4);
          while(1)
          {
              read(sock, file_msg, BUF_SIZE);
              printf("%s", file_msg);
              if(!strcmp(file_msg, "FileEnd_sr->cl"))
                  break;
              fwrite(file_msg, 1, BUF_SIZE, fp);
          }
          
          fclose(fp);
          
          printf("(!Notice)File receive finished \n");
      }
      
      
  
    name_msg[str_len] = 0;
    fputs(name_msg, stdout);
  }
  return NULL;
}

void error_handling(char *msg) {
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}
