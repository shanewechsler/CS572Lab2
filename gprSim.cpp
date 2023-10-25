#include "Simulator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

Memory userData = DataMemory(USER_DATA_BASE);
Memory userText = CodeMemory(USER_TEXT_BASE);

int GPRs[NUMBER_OF_REGISTERS];

using namespace std;

unsigned int dataEntries;

void loadData(char *dataEntry, ){
    unsigned int startingAddress = (USER_TEXT_BASE + (1024 * dataEntries));
    
    const int len = strlen(dataEntry);
    for(int i = 0; i < len; i++){
        userData.WriteToAddress(long(dataEntry[i]), startingAddress+i);
    }
}

void SYSCALL(){
    if(GPRs[20] == 8){

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
    GPRs[Rdest] = userData.ReadAddress(Rbase + offset);
}

void LI(int Rdest, int imm){
    GPRs[Rdest] = imm;
}

void SUBI(int Rdest, int Rsrc1, int imm){
    GPRs[Rdest] = GPRs[Rsrc1] - imm;
}


int main(int argc, char **argv){
    Assembler codeWriter = Assembler();
    Disassembler codeTranslator = Disassembler();

    bool dataMode = false;

    ifstream codeFile(argv[1]);

    string instruction;

    const char *delimiters = " \t\r\n\v\f ,./$()";

    while(getline(codeFile, instruction)){

        unsigned int currentTextAddr = USER_TEXT_BASE;

        char instr_c[instruction.length()];
        char *operands[4];

        /*converts ctring line into c string */
        strcpy(instr_c, instruction.c_str());

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
            case 2:
                label = atoi(operands[1]);
            case 3:
                Rdest = atoi(operands[1]);
                label = atoi(operands[2]);
            case 4:
                Rdest = atoi(operands[1]);
                if(strcmp(operands[INSTR_IDX], "lb") == 0){
                    Rsrc1 = atoi(operands[3]); //offset should be the last operand in our codes, to allow it to be large. 
                    label = atoi(operands[2]);
                }else{
                    Rsrc1 = atoi(operands[2]);
                    label = atoi(operands[3]);
                }
        }
        
        if(!dataMode){
            long instruction = codeWriter.assembleInstr(operands[INSTR_IDX], Rdest, Rsrc1, label);
            userText.WriteToAddress(instruction, currentTextAddr);
            currentTextAddr += 4;
        }else{
            loadData(instr_c);
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
                LB(R1, last, R1);
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
                SYSCALL();
                C += C_SYSCALL;
                break;
        }
    }

}