# CS370 Lab 3 : Pipeline Simulator
## Author: Shane Wechsler
## REDID: 823526399

# Compiliation:
* makefile provided, run `make` from commandline to compile all targets.

# Running the Code:
* Run program with or without a third "sysString" argument.
* Examples:
    - `./pipeSim lab3c.s`
    - `./pipeSim palindrome.s racecar` if we want to test a palindrome

# How it Works:

## Implementation:

* Pipeline: The five stages (IF, ID, EX, MEM, WB) are run backwards because in order for the pipeline to simulate simul execution, the stage after the next cannot see the 
information pushed in the same cycle. IF stage fetches the instruction at PC and pushes it to the IFID queue as a long. The ID stage picks up the instruction pushed last cycle
and decodes it into opcode, R1, R2, immediates. The switch logic in ID determines which of those are important and what to push to the information queues for the EX stage.
The EX stage picks up the opcode from the IDEX stage and decides what to do with the information in the information queues, it also checks the signals set by the stages run
before it to check if it needs to pick up a forward. The MEM stage picks up the result and opcode from the EX stage and decides if it's destination register matches the 
registers that are to be run in the EX stage next, and sets the forward signals accordingly. The only read instr is LB. The WB stage is similar to the MEM stage, it checks if
it's destination register is to be run in the next EX phase and sets signals accordingly. It writes back to the destination if applicable. 

* Forwarding: Even though forwarding between instructions should be done in the EX and MEM stage, the forwarding logic here is done in the MEM and WB stage. This is because the 
pipeline is run backwards (WB -> IF). We set the controls in MEM and WB, because the EX stage is technically run after MEM/WB, which gives the EX stage time to catch the results
in the "later" stages. We have queues `destinations`,`RegAs`, and `RegBs` to determine if forwarding is needed, registers `forwardEXReg` and `forwardMEMReg` to hold the results, and the logic is pushed to forwardAcontrol and forwardBcontrol to set the A/B inputs in the EX stage.

* Syscall: Syscall is completed in the WB stage, mainly to ensure all instructions before it have been completed, the pipeline only kills when 
the syscall gets the `exit` signal. 

## Reading Source Code:

* `Data` mode:
    - parses the next lines after `.data` found as ascii coded strings or byte reserves in simulated dataMemory.
    - `.space` signals that it should reserve bytes for data, and make it unavailible to unauthorized writes, we use these reserved bytes for syscall in this program's case.
    - otherwise reads entire line and writes them char by char, byte by byte into the next free space availible in DataMemory(userData)

* `Text` mode:
    - Reads the lines after `.text` as instructions
    - Reads the tokens in each instruction and properly assigns them to R1, R2, label variables to encode them to binary format.
    - Encodes the instruction and writes it to userText in the proper address.

# 'Partial' Disassembled Code:

* Calculated label values based on the initial PC address of 0x040000 and DataMemory address of 0x0100000 and hardcoded them into 'lab3c.s`
I also included the other samples to prove that it can be run with lab3a.s and lab3b.s.
