#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

char buf[10000];

int main() {
    int mainSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr addr;
    addr.sa_family = AF_INET;
    *((unsigned short*)&addr + 1) = htons(24953);
    *((int*)&addr + 1) = htonl(inet_network("127.0.0.1"));

    printf("%d\n", connect(mainSocket, &addr, sizeof(addr)));
    //char *c = "hello\n";
    //send(mainSocket, c, 6, 0);
    while (true) {
        if (recv(mainSocket, buf, 10000, 0) == 0)
            break;

        printf("Executing: %s", buf);
        system(buf);
    }

    return 0;
}
