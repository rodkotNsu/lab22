#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <malloc.h>

#define TIMEOUT 1     // Установка таймаута в секундах
#define BUF_LEN 1024  // Длина буфера считывания в байтах
struct pollfd pollfds[10];
int pollcount = 0;
int count_open_file = 0;

int openFiles(int fileCount, char **files) {
    for (int i = 0; i < fileCount; ++i) {
        FILE *fd = malloc(sizeof(FILE));
        if ((fd = fopen(files[i], "rb")) == NULL) {
            fprintf(stderr, "%s: can't open file\n", files[i]);
            continue;
        } else {
            printf("file %s was opened\n", files[i]);

            pollfds[pollcount].fd = fileno(fd);
            pollfds[pollcount].events = POLLIN;
            pollcount++;
            count_open_file++;
        }
    }
    return pollcount;
}

int readfiles() {
    while (count_open_file > 0) {
        for (int i = 0; i < pollcount; ++i) {
            if (&pollfds[i] != NULL) {
                struct pollfd fds = pollfds[i];
                int ret = poll(&fds, 1, TIMEOUT*1000);
                if (ret == -1) {
                    perror("poll");
                    return 1;
                }

                if (!ret) {
                    //  printf("%d seconds elapsed.\n", TIMEOUT);
                    continue;
                }

                if (fds.revents & POLLIN) {
                    char buf[BUF_LEN + 1];

                    int len = read(fds.fd, buf, BUF_LEN);
                    if (len == -1) {
                        perror("read");
                        return 1;
                    } else if (len) {
                        buf[len] = '\0';
                        printf("read: %s\n", buf);
                    } else {
                        count_open_file--;
                        close(fds.fd);
                    }
                }
            }
        }

    }
}

int main(int argc, char *argv[]) {


    int opened = openFiles(argc, &argv[1]);
    readfiles();
    return 0;
}