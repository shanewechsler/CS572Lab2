# CS370 Lab 2 : GPR Simulator
## Author: Shane Wechsler
## REDID: 823526399

# Compiliation:
* makefile provided, run `make` from commandline to compile all targets.

# Running the Code:
* Code should be run with this format: `./gprSim palindrome.s "string_to_test"`
* Example: `./gprSim palindrome.s racecar`

* Simulates the system call of reading in the string from the "system" which I wanted to make as a command line input. 

# How it Works:

## Reading Source Code:

* `Data` mode:
    - parses the next lines after `.data` found as ascii coded strings or byte reserves in simulated dataMemory.
    - `.space` signals that it should reserve bytes for data, and make it unavailible to unauthorized writes, we use these reserved bytes for syscall in this program's case.
    - otherwise reads entire line and writes them char by char, byte by byte into the next free space availible in DataMemory(userData)

* `Text` mode:
    - Reads the lines after `.text` as instructions
    - Reads the tokens in each instruction and properly assigns them to R1, R2, label variables to encode them to binary format.
    - Encodes the instruction and writes it to userText in the proper address.

## Execution
* `User` mode:
    - execution mode, begins to decode and disassemble the codes in userText into readable operands.
    -> initialize PC, C, and IC
    -> drop into userMode and begin reading instructions
    -> read instruction at PC address
    -> increment PC
    -> get opcode, R1, R2, and label from instruction
    -> switch case based on opcode, run proper instruction with operands. 
    -> run until final syscall and display IC, C, and speedup.

# 'Partial' Disassembled Code:

* Calculated label values based on the initial PC address of 0x040000 and DataMemory address of 0x0100000 and hardcoded them into `palindrome.s`
