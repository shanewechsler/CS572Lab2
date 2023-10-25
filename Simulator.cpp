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

unsigned int CodeMemory::ConvertAddressToIndex(unsigned int address){
    return (address - this->baseAddress) >> 2; //32 bit addresses are in bytes, but we're storing words so divide by 8
}

unsigned int CodeMemory::ConvertIndexToAddress(unsigned int index){
    return (index + this->baseAddress) << 2;
}

unsigned int DataMemory :: ConvertAddressToIndex(unsigned int address){
    return (address - this->baseAddress);
}

unsigned int DataMemory :: ConvertIndexToAddress(unsigned int index){
    return (index + this->baseAddress);
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
    unsigned long initInstr = 0x0;

    initInstr  = (initInstr | (opcodes[op] << 32));

    initInstr = (initInstr | (r1 << 27));

    initInstr = (initInstr | (r2 << 22));

    initInstr = (initInstr | last);

    return initInstr;
}

int Disassembler :: GetOpcode(long instruction){
    int opcode = (0xFF00000000 & instruction) >> 32;
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
    int last = (0x3FFFFF & instruction);
    return last;
}