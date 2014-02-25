#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int mainSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr addr;
    addr.sa_family = AF_INET;
    *((short*)&addr + 1) = 24953;
    *((int*)&addr + 1) = htonl(inet_network("127.0.0.1"));

    printf("%d\n", connect(mainSocket, &addr, sizeof(addr)));
    char *c = "hello";
    send(mainSocket, c, 5, 0);


    return 0;
}
