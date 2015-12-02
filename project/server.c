#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

#define BUF_SIZE 257
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_msg(char * msg, int len, int clnt_sock);
void error_handling(char * msg);
void execution(int clnt_sock);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
pthread_mutex_t mutx_for_arg;

char* args_g[10];

int main(int argc, char *argv[]) {
  int serv_sock, clnt_sock;
  struct sockaddr_in serv_adr, clnt_adr;
  int clnt_adr_sz;
  pthread_t t_id;

  if (argc != 2) {
    printf("Usage : %s <port>\n", argv[0]);
    exit(1);
  }

  //initialize mutex
  pthread_mutex_init(&mutx, NULL);
  pthread_mutex_init(&mutx_for_arg, NULL);

  //open server socket, bind and listen
  serv_sock = socket(PF_INET, SOCK_STREAM, 0);

  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_adr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
    error_handling("bind() error");
  if (listen(serv_sock, 5) == -1)
    error_handling("listen() error");

  /* when client try to connect to server, server give one of clnt_socks.
   * Next, server call handle_clnt thread and detach thread for returning resource
   */
  while (1) {
    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, &clnt_adr_sz);

    pthread_mutex_lock(&mutx);
    clnt_socks[clnt_cnt++] = clnt_sock;
    pthread_mutex_unlock(&mutx);

    pthread_create(&t_id, NULL, handle_clnt, (void*) &clnt_sock);
    pthread_detach(t_id);
    printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
  }
  close(serv_sock);
  return 0;
}

void* handle_clnt(void* arg) {
  int clnt_sock = *(int*) arg;
  int str_len = 0, i, j, k, n;
  char msg[BUF_SIZE];
  char* ptr;
  char* token;
  char buf[100];
  char tempbuf[100];
  char recvbuf[BUF_SIZE];
  FILE* fp;

  //waiting client message
  while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0) {
    strcpy(buf, msg);
    buf[str_len - 1] = '\0';

    token = strtok_r(buf, " ", &ptr);
    token = strtok_r(NULL, " ", &ptr);
    if (token == NULL)
      continue;

    //if first charater in the string is '/', then it is command string
    if (strcmp("/exec", token) == 0) {
      i = 0;
      token = strtok_r(NULL, " ", &ptr);
      pthread_mutex_lock(&mutx_for_arg);
      while (token) {
        args_g[i] = (char*) malloc(strlen(token));
        strcpy(args_g[i], token);
        token = strtok_r(NULL, " ", &ptr);
        i++;
      }

      for (k = i; k < 10; k++)
        args_g[k] = (char*) NULL;

      execution(clnt_sock);

      for (k = 0; k < i; k++) {
        printf("%s\n", args_g[k]);
        free(args_g[k]);
      }
      pthread_mutex_unlock(&mutx_for_arg);
    } else if (strcmp("/send", token) == 0) {
      printf("send\n");
      token = strtok_r(NULL, " ", &ptr);
      sprintf(tempbuf, "server is receiving %s\n", token);
      write(clnt_sock, tempbuf, strlen(tempbuf));

      sprintf(tempbuf, "%s_received", token);
      printf("%s", tempbuf);
      fp = fopen(tempbuf, "wb");

      while ( (n = read(clnt_sock, recvbuf, BUF_SIZE)) > 0) {
        fwrite(recvbuf, 1, BUF_SIZE, fp);
        if( n < BUF_SIZE ){
          break;
        }
      }

      fclose(fp);
    } else {
      send_msg(msg, str_len, clnt_sock);
    }
  }

  //if client is disconnected, then remove client
  pthread_mutex_lock(&mutx);
  for (i = 0; i < clnt_cnt; i++) {   // remove disconnected client
    if (clnt_sock == clnt_socks[i]) {
      while (i++ < clnt_cnt - 1)
        clnt_socks[i] = clnt_socks[i + 1];
      break;
    }
  }
  clnt_cnt--;
  pthread_mutex_unlock(&mutx);
  close(clnt_sock);
  return NULL;
}

void send_msg(char * msg, int len, int clnt_sock)   // send to all
{
  int i;

  //common conversation
  //send a massage to all user not for sender
  //if message have '/emotion' string, then it have to be changed to something
  pthread_mutex_lock(&mutx);
  for (i = 0; i < clnt_cnt; i++) {
    if (clnt_sock == clnt_socks[i])
      continue;
    write(clnt_socks[i], msg, len);
  }
  pthread_mutex_unlock(&mutx);

}
void error_handling(char * msg) {
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}

void execution(int clnt_sock) {
  pid_t pid;
  int n, i;
  char msg[257];
  int pd;
  int fd[2];
  int status;
  int flags;
  int count = 0;

  pipe(fd);

  switch (pid = fork()) {
  case -1:
    perror("fork");
    exit(1);
  case 0:
    close(1);
    dup(fd[1]);
    close(2);
    dup(fd[1]);
    execvp(args_g[0], args_g);
    break;
  default:
    flags = fcntl(pd, F_GETFL, 0);
    fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);

    while (waitpid(pid, &status, WNOHANG) == pid);
    while (1) {
      count++;
      n = read(fd[0], msg, 256);
      if (n < 0) {
        usleep(100);
        if (count > 100) {
          break;
        } else
          continue;
      } else {
        msg[n] = '\0';
        write(clnt_sock, msg, strlen(msg));
        write(1, msg, strlen(msg));
      }
      if (n < 256) {
        break;
      }
    }
    break;
  }
}

