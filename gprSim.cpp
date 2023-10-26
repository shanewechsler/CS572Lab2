#include "Simulator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

Memory userData = Memory(USER_DATA_BASE, 0); //data is split into bytes for string, no shifting
Memory userText = Memory(USER_TEXT_BASE, 2); //code is split into words, shift by 2 to get array index

char* sysString; //string to test

int GPRs[NUMBER_OF_REGISTERS];

using namespace std;

unsigned int nextFreeSpace = USER_DATA_BASE; //first address availible in data memory

void sectionData(unsigned int bytes){
    nextFreeSpace = nextFreeSpace + bytes; //reserve x no of bytes in data memory
}

void writeData(char* asciiToWrite, unsigned int startingAddress){
    unsigned int addressPointer = startingAddress;

    const int len = strlen(asciiToWrite);
    for(int i = 0; i < len; i++){
        userData.WriteToAddress(long(asciiToWrite[i]), addressPointer); //write each char byte by byte into memory
        addressPointer++;
    }
}

void readAndPrintData(unsigned int startingAddress){
    unsigned int addressPointer = startingAddress;
    while(userData.ReadAddress(addressPointer) != 0){
        cout << char(userData.ReadAddress(addressPointer)); //read each char byte by byte from data memory
        addressPointer++;
    }
    cout << endl;
}

void SYSCALL(bool &userMode){
    if(GPRs[20] == 8){ //syscall 8 signal writes sys string into memory.
        writeData(sysString, GPRs[10]);
    }else if(GPRs[20] == 4){ //syscall signal 4 reads data at memory
        readAndPrintData(GPRs[10]);
    }else if(GPRs[20] == 10){ //syscall signal 10 exits program
        userMode = false;
    }
}

void ADDI(int Rdest, int Rsrc1, int imm){
    GPRs[Rdest] = GPRs[Rsrc1] + imm;
}

void B(unsigned int &PC, int label){
    PC += label;
}

void BEQZ(unsigned int &PC, int Rsrc1, int label){
    if(GPRs[Rsrc1] == 0){
        PC += label;
    }
}

void BGE(unsigned int &PC, int Rsrc1, int Rsrc2, int label){
    if(GPRs[Rsrc1] >= GPRs[Rsrc2]){
        PC += label;
    }
}

void BNE(unsigned int &PC, int Rsrc1, int Rsrc2, int label){
    if(GPRs[Rsrc1] != GPRs[Rsrc2]){
        PC += label;
    }
}

void LA(unsigned int PC, int Rdest, int label){
    GPRs[Rdest] = (PC + label);
}

void LB(int Rdest, int offset, int Rbase){
    GPRs[Rdest] = userData.ReadAddress(GPRs[Rbase] + offset);
}

void LI(int Rdest, int imm){
    GPRs[Rdest] = imm;
}

void SUBI(int Rdest, int Rsrc1, int imm){
    GPRs[Rdest] = GPRs[Rsrc1] - imm;
}


int main(int argc, char **argv){

    if (argc < 3){
        cout << "Second command line input shpuld be palindrome to test." << endl;
        exit(1);
    }

    
    Assembler codeWriter = Assembler();
    Disassembler codeTranslator = Disassembler();

    bool dataMode = false;

    ifstream codeFile(argv[1]);
    sysString = argv[2];

    string instruction;

    const char *delimiters = " \t\r\n\v\f ,./$()";

    unsigned int currentTextAddr = USER_TEXT_BASE; //first availible address in codeMemory

    while(getline(codeFile, instruction)){

        char instr_c[instruction.length()];
        char instr_data[instruction.length()];
        char *operands[4];

        /*converts ctring line into c string */
        strcpy(instr_c, instruction.c_str());
        strcpy(instr_data, instruction.c_str());

        char *newToken = strtok(instr_c, delimiters); //splits the new line into operands

        int i = 0;
        while(newToken != nullptr && i < 4){
            operands[i] = newToken;
            newToken = strtok(NULL, delimiters);
            i++;
        }

        int Rdest = 0;
        int Rsrc1 = 0;
        int label = 0;
        switch(i){
            case 0: //nothing in the line, read the next line
                continue;
            case 1: //only one "operand" found check if it's .data or .text, if not store no operand instr in userText
                if(strcmp(operands[INSTR_IDX], "data") == 0){
                    dataMode = true;
                    continue;
                }else if(strcmp(operands[INSTR_IDX], "text") == 0){
                    dataMode = false;
                    continue;
                }
                break;
            case 2:
                if(dataMode && (strcmp(operands[INSTR_IDX], "space") == 0)){
                    sectionData(atoi(operands[1]));
                    continue;
                }
                label = atoi(operands[1]);
                break;
            case 3:
                Rdest = atoi(operands[1]);
                label = atoi(operands[2]);
                break;
            case 4:
                Rdest = atoi(operands[1]);
                if(strcmp(operands[INSTR_IDX], "lb") == 0){
                    Rsrc1 = atoi(operands[3]); //offset should be the last operand in our codes, to allow it to be large. 
                    label = atoi(operands[2]);
                }else{
                    Rsrc1 = atoi(operands[2]);
                    label = atoi(operands[3]);
                }
                break;
        }
        
        if(!dataMode){
            long instruction = codeWriter.assembleInstr(operands[INSTR_IDX], Rdest, Rsrc1, label);
            userText.WriteToAddress(instruction, currentTextAddr);
            currentTextAddr += 4;
        }else{
            unsigned int dataAddress = nextFreeSpace;

            sectionData(strlen(instr_data) + 1);
            writeData(instr_data, dataAddress);
        }
    }

    unsigned int PC = USER_TEXT_BASE;
    unsigned int IC = 0;
    unsigned int C = 0;
    bool userMode = true;
    while(userMode){
        long instruction = userText.ReadAddress(PC);
        PC += 4;
        IC++;
        int opcode = codeTranslator.GetOpcode(instruction);
        int R1 = codeTranslator.GetR1(instruction);
        int R2 = codeTranslator.GetR2(instruction);
        int last = codeTranslator.GetLabelOrOffset(instruction);
        switch(opcode){
            case ADDI_SIG:
                ADDI(R1, R2, last);
                C += C_ADDI;
                break;
            case B_SIG:
                B(PC, last);
                C += C_B;
                break;
            case BEQZ_SIG:
                BEQZ(PC, R1, last);
                C += C_BEX;
                break;
            case BGE_SIG:
                BGE(PC, R1, R2, last);
                C += C_BEX;
                break;
            case BNE_SIG:
                BNE(PC, R1, R2, last);
                C += C_BEX;
                break;
            case LA_SIG:
                LA(PC, R1, last);
                C += C_LA;
                break;
            case LB_SIG:
                LB(R1, last, R2);
                C +=  C_LB;
                break;
            case LI_SIG:
                LI(R1, last);
                C += C_LI;
                break;
            case SUBI_SIG:
                SUBI(R1, R2, last);
                C += C_SUBI;
                break;
            case SYSCALL_SIG:
                SYSCALL(userMode);
                C += C_SYSCALL;
                break;
        }
    }
    cout << "Instuction Count: " << IC << endl;
    cout << "Cycle Count: " << C << endl;
    cout << "Speedup from Single Cycle: " << (float) (8*IC)/C << endl;

}