#include "Simulator.h"
#include <cstring>
#include <string>
#include <iostream>
#include <map>

unsigned int Memory::getBaseAddress(){
    return this->baseAddress;
}

long Memory::ReadAddress(unsigned int address){
    unsigned int index = ConvertAddressToIndex(address);
    return this->memoryEntries[index];
}

void Memory::WriteToAddress(long word, unsigned int address){ //just longs and not ulongs becase I wanted to be able to store negative values in data
    unsigned int index = ConvertAddressToIndex(address);
    this->memoryEntries[index] = word;
}

unsigned int Memory::ConvertAddressToIndex(unsigned int address){
    return (address - this->baseAddress) >> shiftSize; //shift to get index
}

unsigned int Memory::ConvertIndexToAddress(unsigned int index){
    return (index + this->baseAddress) << shiftSize;
}


void Assembler::initMap(){
    this->opcodes["addi"] = ADDI_SIG;
    this->opcodes["b"] = B_SIG;
    this->opcodes["beqz"] = BEQZ_SIG;
    this->opcodes["bge"] =  BGE_SIG;
    this->opcodes["bne"] = BNE_SIG;
    this->opcodes["la"] = LA_SIG;
    this->opcodes["lb"] = LB_SIG;
    this->opcodes["li"] = LI_SIG;
    this->opcodes["subi"] = SUBI_SIG;
    this->opcodes["syscall"] =  SYSCALL_SIG;
}

long Assembler::assembleInstr(std::string op, int r1, int r2, int last){
    long initInstr = 0x0; //code format: bytes 40-32 opcode, 32-27 R1, 27-22 R2, 22-0 label/offset/immediate

    initInstr  = (initInstr | (ulong(opcodes[op]) << 32)); //opcode, r1, r2 are all positive

    initInstr = (initInstr | (ulong(r1) << 27));

    initInstr = (initInstr | (ulong(r2) << 22));

    initInstr = (initInstr | (0x3FFFFF & last)); //signed integer, last 22 bytes.

    return initInstr;
}

int Disassembler :: GetOpcode(long instruction){
    int opcode = (0xFF00000000 & instruction) >> 32; //masks to isolate different codes
    return opcode;
}

int Disassembler :: GetR1(long instruction){
    int R1 = (0xF8000000 & instruction) >> 27;
    return R1;
}

int Disassembler :: GetR2(long instruction){
    int R2 = (0x7C00000 & instruction) >> 22;
    return R2;
}

int Disassembler :: GetLabelOrOffset(long instruction){
    int last = (0x3FFFFF & instruction) << 10; //shift to the end of 32 so it recognizes that it's signed
    int sint = last >> 10; //shift back, if negative, will be recognized
    return sint;
}