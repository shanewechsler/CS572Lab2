#ifndef SIMULATOR_H
#define SIMULATOR_H

#define USER_TEXT_BASE  0x00040000
#define USER_DATA_BASE  0x00100000

#define MEMORY_ENTRY_COUNT 0x1000

#define OPERAND_LENGTH 32
#define NUMBER_OF_REGISTERS 32

#define INSTR_IDX 0
#define OPERAND_IDX 1

#define ADD_SIG 0x0
#define ADDI_SIG 0x1
#define B_SIG 0x2
#define BEQZ_SIG 0x3
#define BGE_SIG 0x4
#define BNE_SIG 0x5
#define LA_SIG 0x6
#define LB_SIG 0x7
#define LI_SIG 0x8
#define SUBI_SIG 0x9
#define SYSCALL_SIG 0xA
#define NOP_SIG 0xB

#define C_ADDI 6
#define C_B 4
#define C_BEX 5
#define C_LA 5
#define C_LB 6
#define C_LI 3
#define C_SUBI 6
#define C_SYSCALL 8

#include <stack>
#include <string>
#include <fstream>
#include <map>

class Memory{
    protected:
        unsigned int baseAddress;
        
        long memoryEntries[MEMORY_ENTRY_COUNT]; 

        int shiftSize = 2;
    public:
        Memory(){
            this->baseAddress = 0x0;
        }

        Memory(unsigned int baseAddress, int shiftSize){
            this->baseAddress = baseAddress;
            this->shiftSize = shiftSize;
        };
        unsigned int getBaseAddress();

        long ReadAddress(unsigned int address); // "load", reads the data @address

        void WriteToAddress(long word, unsigned int address); //"store", stores double word @address

        unsigned int ConvertAddressToIndex(unsigned int address);

        unsigned int ConvertIndexToAddress(unsigned int index);
};

class Assembler{
    private:
        std::map<std::string, unsigned int> opcodes;

    public:
        Assembler(){
            initMap();
        }

        void initMap();

        long assembleInstr(std::string op, int r1, int r2, int last);
};

class Disassembler{
    public:
        int GetOpcode(long instruction);
        int GetR1(long instruction);
        int GetR2(long instruction);
        int GetLabelOrOffset(long instruction);
};

#endif //SIMULATOR_H