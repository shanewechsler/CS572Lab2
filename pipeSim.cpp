#include "Simulator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <queue>

Memory userData = Memory(USER_DATA_BASE, 0); //data is split into bytes for string, no shifting
Memory userText = Memory(USER_TEXT_BASE, 2); //code is split into words, shift by 2 to get array index

Assembler codeWriter = Assembler();
Disassembler codeTranslator = Disassembler();

char* sysString; //string to test
int IC = 0;
int NC = 0;

int GPRs[NUMBER_OF_REGISTERS];

int forwardAControl = 0; 
int forwardBControl = 0;

int forwardEXReg = 0; //the result value from the EX stage that may be forwarded
int forwardMEMReg = 0; //result from MEM

using namespace std;

queue<int> Rsrc1s; //the values needed from registers to be used in EX
queue<int> Rsrc2s;
queue<int> RegAs; //the actual register numbers for use by forwarding logic
queue<int> RegBs;
queue<int> immediates;

queue<long> IFID;
queue<int> IDEX;
queue<int> EXMEM;
queue<int> MEMWB;
queue<int> dests;

unsigned int nextFreeSpace = USER_DATA_BASE; //first address availible in data memory
unsigned int currentTextAddr = USER_TEXT_BASE; //first availible address in codeMemory

void writeData(char* asciiToWrite, unsigned int startingAddress);
void readAndPrintData(unsigned int startingAddress);
void SYSCALL(bool &userMode);

void IF(unsigned int &PC){
    if(PC < currentTextAddr){ //while PC is in range of the code base
        IFID.push(userText.ReadAddress(PC));
        PC += 4;
        IC++;
    }
}

void ID(){
    if(!IFID.empty()){
        long instruction = IFID.front(); //get instruction from queue
        IFID.pop();
        int opcode = codeTranslator.GetOpcode(instruction);
        IDEX.push(opcode); //push the opcode forward so the EX knows what to do next cycle
        int r1 = codeTranslator.GetR1(instruction);
        int r2 = codeTranslator.GetR2(instruction);
        int last = codeTranslator.GetLabelOrOffset(instruction);
    
        int A = 0; //simulating "don't cares"
        int B = 0;
        int imm = 0;
        int dest = 0;

        int reg1 = -1; //negative 1 because -1 will never equal a register number in later forwarding logic
        int reg2 = -1;

        //set information needed by the EX stage based on opcode
        switch(opcode){
            case ADD_SIG:
                A = GPRs[r2];
                reg1 = r2;
                B = GPRs[last];
                reg2 = last;
                dest = r1;
                break;
            case ADDI_SIG:
                A = GPRs[r2];
                reg1 = r2;
                imm = last;
                dest = r1;
                break;
            case B_SIG:
                imm = last;
                break;
            case BEQZ_SIG:
                A = GPRs[r1];
                reg1 = r1;
                imm = last;
                break;
            case BGE_SIG:
                A = GPRs[r1];
                reg1 = r1;
                B = GPRs[r2];
                reg2 = r2;
                imm = last;
                break;
            case BNE_SIG:
                A = GPRs[r1];
                reg1 = r1;
                B = GPRs[r2];
                reg2 = r2;
                imm = last;
                break;
            case LA_SIG:
                imm = last;
                dest = r1;
                break;
            case LB_SIG:
                A = GPRs[r2];
                reg1 = r2;
                imm = last;
                dest = r1;
                break;
            case LI_SIG:
                imm = last;
                dest = r1;
                break;
            case SUBI_SIG:
                A = GPRs[r2];
                reg1 = r2;
                imm = last;
                dest = r1;
                break;
            case NOP_SIG:
                NC++; //nop, we just need to record it.
                break;
        }

        Rsrc1s.push(A); //push all information including "dont cares"
        Rsrc2s.push(B);
        RegAs.push(reg1);
        RegBs.push(reg2);
        immediates.push(imm);
        dests.push(dest);
    }

}

void EX(unsigned int &PC){
    if(!IDEX.empty()){
        int opcode = IDEX.front();
        IDEX.pop();

        EXMEM.push(opcode); 
    
        int A = Rsrc1s.front();
        int B = Rsrc2s.front();
        int imm = immediates.front();

        switch(forwardAControl){ //look at forward signals and decide if src A needs to be changed
            case 0:
                break;
            case 1:
                A = forwardEXReg;
                break;
            case 2:
                A = forwardMEMReg;
                break;
        }

        switch(forwardBControl){
            case 0:
                break;
            case 1:
                B = forwardEXReg;
                break;
            case 2:
                B = forwardMEMReg;
                break;
        }

        Rsrc1s.pop();
        Rsrc2s.pop();
        RegAs.pop(); //forwarding logic is done, no need to hold onto register numbers.
        RegBs.pop();
        immediates.pop();

        int result = 0;
        switch(opcode){
            case ADD_SIG:
                result = A + B;
                break;
            case ADDI_SIG:
                result = A + imm;
                break;
            case B_SIG:
                PC += imm;
                break;
            case BEQZ_SIG:
                if(A == 0){
                    PC += imm;
                }
                break;
            case BGE_SIG:
                if(A >= B){
                    PC += imm;
                }
                break;
            case BNE_SIG:
                if(A != B){
                    PC += imm;
                }
                break;
            case LA_SIG:
                result = PC + imm;
                break;
            case LB_SIG:
                result = A + imm;
                break;
            case LI_SIG:
                result = imm;
                break;
            case SUBI_SIG:
                result = A - imm;
                break;
        }
        EXMEM.push(result);
    }
}

void MEM(){
    if(!EXMEM.empty()){
        int opcode = EXMEM.front(); //opcode always comes first
        EXMEM.pop();
        int ALUResult = EXMEM.front();
        EXMEM.pop();

        MEMWB.push(opcode);

        forwardEXReg = ALUResult;
        int forwardPossibility = dests.front(); //front of queue now will be the destination here, because WB runs before MEM technically.

        if(opcode <= ADDI_SIG || (opcode >= LA_SIG && opcode <= SUBI_SIG)){ //if opcode signals having a dest.
            if (forwardPossibility == RegAs.front()){
                forwardAControl = 1; //A matches, signal from "EX" stage..
            }else if(forwardPossibility == RegBs.front()){
                forwardBControl = 1;
            }
        }

        int memResult = ALUResult;
        if(opcode == LB_SIG){
            memResult = userData.ReadAddress(ALUResult); //LB is the only read.
        }

        MEMWB.push(memResult);
    }
}

void WB(bool &userMode){
    if(!MEMWB.empty()){
        int opcode = MEMWB.front();
        MEMWB.pop();
        int result = MEMWB.front();
        MEMWB.pop();

        int destination = dests.front();
        dests.pop();

        forwardAControl = 0; //reset them to 0 on every cycle
        forwardBControl = 0;
        forwardMEMReg = result;
        if(opcode <= ADDI_SIG || (opcode >= LA_SIG && opcode <= SUBI_SIG)){
            if(destination == RegAs.front()){
                forwardAControl = 2;
            }else if(destination == RegBs.front()){
                forwardBControl = 2;
            }
            GPRs[destination] = result; //actual WB
        }else if (opcode == SYSCALL_SIG){
            SYSCALL(userMode); //complete syscall
        }
    }
}

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


int main(int argc, char **argv){
    bool dataMode = false;

    ifstream codeFile(argv[1]);

    if(argc > 2){
        sysString = argv[2];
    }
    string instruction;

    const char *delimiters = " \t\r\n\v\f ,./$()";

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
    unsigned int C = 0;
    bool userMode = true;
    while(userMode){
        C += 1;
        WB(userMode);
        MEM();
        EX(PC);
        ID();
        IF(PC);
    }
    cout << "Instruction count: " << IC << endl;
    cout << "Cycle count: " << C << endl;
    cout << "NOP count: " << NC << endl;
}