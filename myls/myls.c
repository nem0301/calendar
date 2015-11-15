#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

char getKind(mode_t mode){
    int kind = mode & S_IFMT;
    switch (kind){
        case S_IFIFO:
            return '-';
        case S_IFCHR:
            return 'c';
        case S_IFDIR:
            return 'd';
        case S_IFBLK:
            return 'b';
        case S_IFREG:
            return '-';
        case S_IFLNK:
            return 'l';
        case S_IFSOCK:
            return '-';
        default:
            return '-';
    }
}

void getAutho(char* dest, mode_t mode){
    int i;
    char wrx[3] = {'r', 'w', 'x'};
    mode_t autho = mode & 0777;

    for ( i = 8; i >= 0; i--){
        if ( autho & 01 )
            dest[i] = wrx[i%3];
        else
            dest[i] = '-';
        autho >>= 1;
    }
    dest[9] = '\0';
}


void lsDir(const char* dirName, const char* preDir, int allSet, int inodeSet, int listSet, int recurSet){
    //for dir and file list
    struct dirent** myfile;
    struct stat mystat;
    int i, n;
    int kind;
    char autho[10];
    mode_t mode;
    struct passwd* pw;
    struct group* grp;
    time_t t;
    time_t t2;
    struct tm* tms;

    int inodeLen = 0;
    int linkLen = 0;
    int unameLen = 0;
    int gnameLen = 0;
    int sizeLen = 0;
    int filenameLen = 0;

    char lengBuf[100];
    char dirBuf[100];
    char cwd[100];
    char previousDir[100];
    char tempBuf[100];

    int newlineCount = 0;

    getcwd(cwd, sizeof(cwd));
    chdir(dirName);

    n = scandir(".", &myfile, 0, alphasort);

    if(recurSet){
        printf("%s:\n", preDir);
    }



    //for padding
    for(i = 0; i < n; i++){
        if ( myfile[i]->d_name[0] == '.' && !allSet){
            continue;
        }
        stat(myfile[i]->d_name, &mystat);

        sprintf(lengBuf, "%d", (int)mystat.st_ino);
        if (inodeLen <  (int) strlen(lengBuf))
            inodeLen = (int) strlen(lengBuf);

        sprintf(lengBuf, "%d", (int)mystat.st_nlink);
        if (linkLen <  (int) strlen(lengBuf))
            linkLen = (int) strlen(lengBuf);

        strcpy(lengBuf, getpwuid(mystat.st_uid)->pw_name);
        if (unameLen <  (int) strlen(lengBuf))
            unameLen = (int) strlen(lengBuf);

        strcpy(lengBuf, getgrgid(mystat.st_gid)->gr_name);
        if (gnameLen <  (int) strlen(lengBuf))
            gnameLen = (int) strlen(lengBuf);

        sprintf(lengBuf, "%d", (int)mystat.st_size);
        if (sizeLen <  (int) strlen(lengBuf))
            sizeLen = (int) strlen(lengBuf);

        strcpy(lengBuf, myfile[i]->d_name);
        if (filenameLen <  (int) strlen(lengBuf))
            filenameLen = (int) strlen(lengBuf);
        

    }
        
    if (listSet){   
        for(i = 0; i < n; i++){
            if ( myfile[i]->d_name[0] == '.' && !allSet){
                continue;
            }
            //stat
            stat(myfile[i]->d_name, &mystat);

            //mode
            mode = mystat.st_mode;

            //uid, gid
            pw = getpwuid(mystat.st_uid);
            grp = getgrgid(mystat.st_gid);
        
            //time
            t = mystat.st_ctime;
            tms = gmtime(&t);                
            
            time(&t2);

            t2 = t2 - t;

            getAutho(autho, mode);
            
            if (inodeSet){
                printf("%*d ", inodeLen, (int)mystat.st_ino);
            }

            printf("%c%s ", getKind(mode), autho);
            printf("%*d ", linkLen, (int)mystat.st_nlink);
            printf("%*s ", unameLen, pw->pw_name);
            printf("%*s ", gnameLen, grp->gr_name);
            printf("%*d ", sizeLen, (int)mystat.st_size);
            if (t2 < 15811200)
                printf("%02d월 %02d %02d:%02d ", tms->tm_mon, tms->tm_mday, tms->tm_hour, tms->tm_min);
            else
                printf("%02d월 %02d %5d ", tms->tm_mon, tms->tm_mday, tms->tm_year + 1900);

            printf("%s ", myfile[i]->d_name);
            
            if (i < n+1)  
                printf("\n");
               
        }
    } else {
        for(i = 0; i < n; i++){
            if ( myfile[i]->d_name[0] == '.' && !allSet){
                continue;
            }
            stat(myfile[i]->d_name, &mystat);
            if (inodeSet){
                printf("%*d %-*s ", inodeLen, (int)mystat.st_ino, filenameLen, myfile[i]->d_name);
                sprintf(tempBuf, "%d", (int)mystat.st_ino);
                newlineCount += (inodeLen + filenameLen);
            } else {
                printf("%-*s ", filenameLen, myfile[i]->d_name);
                newlineCount += filenameLen;
            }

            if ( newlineCount > 80){
                printf("\n");
                newlineCount = 0;
            }
        }
        printf("\n");
    }

    if (recurSet){
        for(i = 0; i < n; i++){
            stat(myfile[i]->d_name, &mystat);
            mode = mystat.st_mode;
            if (getKind(mode) == 'd' && !( strcmp(".", myfile[i]->d_name) == 0 || strcmp("..", myfile[i]->d_name) == 0)){
                printf("\n");
                sprintf(dirBuf, "%s",myfile[i]->d_name);
                sprintf(previousDir, "%s/%s", preDir, myfile[i]->d_name);
                lsDir(dirBuf, previousDir, allSet, inodeSet, listSet, recurSet);
            }            
        }
    }

    chdir(cwd);
}

int main(int argc, char* argv[]){

    //for index or etc
    int n;

    //for option
    extern char* optarg;
    extern int optind;
    struct option longOptArgs[] = {
        {"all", 0, NULL, 'a'},
        {"inode", 0, NULL, 'i'},
        {"format", 1, NULL, 'l'},
        {"recursive", 0, NULL, 'R'},
        {NULL, 0, NULL, 0}
    };
    int allSet = 0;
    int inodeSet = 0;
    int listSet = 0;
    int recurSet = 0;


    while (( n = getopt_long(argc, argv, "ailR", longOptArgs, NULL)) != -1){
        switch (n) {
            case 'a':
                allSet = 1;
                break;
            case 'i':
                inodeSet = 1;
                break;
            case 'l':
                if (optarg == NULL){
                    listSet = 1;
                    break;
                }
                if ( (strcmp("verbose", optarg) == 0) || (strcmp("long", optarg) == 0 ) )
                    listSet = 1;
                break;
            case 'R':
                recurSet = 1;
                break;
            default:
                break;
        }
    }

    if(argv[optind] == NULL){
        lsDir(".", ".", allSet, inodeSet, listSet, recurSet);
    } else {
        lsDir(argv[optind], argv[optind], allSet, inodeSet, listSet, recurSet);
    }
    return 0;
}
