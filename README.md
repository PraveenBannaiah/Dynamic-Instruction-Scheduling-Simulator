**THIS PROJECT WAS COMPLETED UNDER THE GUIDANCE OF DR.ERIC ROTTENBERG AT NC STATE UNIVERSITY** 

**Project Description** 
 
  In this project, I have constructed a simulator for an out-of-order superscalar processor that 
  fetches and issues N instructions per cycle. Only the dynamic scheduling mechanism was 
  modeled in detail, i.e., perfect caches and perfect branch prediction are assumed. 

**Traces**
 
  The simulator reads a trace file in the following format: 
  <PC> <operation type> <dest reg #> <src1 reg #> <src2 reg #>  
  <PC> <operation type> <dest reg #> <src1 reg #> <src2 reg #>  
  ... 
  Where: 
  o <PC> is the program counter of the instruction (in hex). 
  o <operation type> is either “0”, “1”, or “2”. 
  o <dest reg #> is the destination register of the instruction. If it is -1, then the 
  instruction does not have a destination register (for example, a conditional branch 
  instruction). Otherwise, it is between 0 and 66. 
  o <src1 reg #> is the first source register of the instruction. If it is -1, then the 
  instruction does not have a first source register. Otherwise, it is between 0 and 66. 
  o <src2 reg #> is the second source register of the instruction. If it is -1, then the 
  instruction does not have a second source register. Otherwise, it is between 0 and 66. 
  For example: 
  ab120024  0   1  2  3 
  ab120028  1   4  1  3 
  ab12002c  2  -1  4  7 
  Means: 
  “operation type 0”  R1, R2, R3 
  “operation type 1”  R4, R1, R3 
  “operation type 2”  -, R4, R7                    // no destination register!  

**Inputs to the Simulator**
  
  sim <ROB_SIZE> <IQ_SIZE> <WIDTH> <tracefile> 

**Outputs of the simulator**
  
  The simulator outputs the following after completion of the run: 
  1. Simulator command. 
  2. Processor configuration.
  3. Simulation results: 
    a. Dynamic instruction count. (Total number of retired instructions.) 
    b. Cycles. (Total number of cycles to retire all instructions.) 
    c. Instructions per cycle (IPC). (item a divided by item b, above)

**Pipeline**
 
  <img width="462" alt="image" src="https://github.com/user-attachments/assets/c8c51b31-901f-4ee3-952a-47ca7e25c36c" />
