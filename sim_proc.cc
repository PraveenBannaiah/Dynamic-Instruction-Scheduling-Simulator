#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sim_proc.h"
#include<iostream>
/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/

void Fetch(FILE*);
	
void Decode();

void Rename();

void RegRead();

void Dispatch();

void Issue();

void Execute();

void Writeback();

void Retire();

int Advance_Cycle();


//Starting for scalar processor
unsigned long int Pipeline[6][9];                 //9 because we have a 9 stage pipelined
// pipeline[][0] -> Fetch; pipeline[][1] -> Decode ; Pipeline[][2] -> Rename
//Pipeline[0] -> inst there or not; Pipeline[1] ->src1 ; pipeline[2] ->src2; pipeline[3] ->dest ; pipeline[4] -> type ; pipeline[5] ->PC
	
//Resetting the pipeline
for(int i=0;i<9;i++)
	pipeline[0][i] = 0;                //Means there are not instructions in any stage of the pipeline
	

//Reoder Buffer initialisation 
int** ROB = nullptr;
int ROB_head_pointer = 0;
int ROB_tail_pointer = 0;
int ROB_tail_phase = 0;


//Issue Queue initialisation
int** IQ = nullptr;


//Architectural Register File
unsigned int long ARF[67];                              //0-66  , Given in the specs

//Rename Map Table
int RMT_valid_array[67];
int RMT_rob_tag[67];


//Pipeline register contents for scalar processor
int op_type, dest, src1,src2;
unsigned int pc;


int cycle_stop_signal = 0;

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    uint64_t pc; // Variable holds the pc read from input file
    
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    printf("rob_size:%lu "
            "iq_size:%lu "
            "width:%lu "
            "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // The following loop just tests reading the trace and echoing it back to the screen.
    //
    // Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
    // Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
    // inside the Fetch() function.
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
     //   printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly


	
	///////////////////////Assigning memory to the ROB array////////////////////////
	ROB = new int*[params.rob_size];
	for(int i=0;i<params.rob_size;i++)
	{
		if( i!= (params.rob_size - 1))
			ROB[i] = new int[7];                          //[][0] valid; [][1] value; [][2] dst; [][3] rdy; [][4] exc; [][5] mis; [][6] pc
		else                                              //Use 10001, 10002 etc as rob tag and when accesing just do index%1000
			ROB[i] = new unsigned long int[7];
	}
	
	
	
	///////////////////////Assigning memory to the Issue Queue///////////////////////
	IQ = new int*[params.iq_size];
	for(int i=0;i<params.iq_size;i++)
		IQ[i] = new int[6];                               //[][0] valid; [][1]dst tag, rs1 rdy, rs1 tag/value,rs2 rdy, rs2 tag/value 
	
	
	do {
		Retire();
		Writeback();
		Execute();
		Issue();
		Dispatch();
		RegRead();
		Rename();
		Decode();
		Fetch(FP);
	}while(Advance_Cycle());
	
	
    return 0;
}


void Retire()
{
	std::cout<<"\n In Retire stage";
}

void Writeback()
{
	std::cout<<"\n In writeback stage";
}

void Execute()
{
	std::cout<<"\n In Execute stage";
}

void Issue()
{
	std::cout<<"\n In Issue stage";
}

void Dispatch()
{
	std::cout<<"\n In Dispatch stage";
}

void RegRead()
{
	std::cout<<"\n In RegRead stage";
}

void Rename()
{
	std::cout<<"\n In Rename stage";
}

void Decode()
{
	std::cout<<"\n In decode stage";
}

void Fetch(FILE* FP)
{	
	std::cout<<"\n In Fetch stage";
	
	//while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
	if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
	{
		//fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2);
		std::cout<<"\n"<< pc<<" " <<op_type<<" "<< dest<<" "<< src1<<" "<< src2; //Print to check if inputs have been read correctly
		
		/*pipeline[0][0] = 1;                                          //means there is a valid instruction in the stage
		pipeline[1][0] = src1;                                      //These are the values in current cycle, but decode will need the values in previous cycle
		pipeline[2][0] = src2;
		pipeline[3][0] = dest;
		pipeline[4][0] = op_type;
		pipeline[5][0] = pc;*/
	}
	else
		cycle_stop_signal = 1;
}
int Advance_Cycle()
{
	std::cout<<"\n Cycle finished";
	if(!cycle_stop_signal)
	{
		//In the next cycle the values propagate to the next stage of the pipeline ; going ulta is better
		//Pipeline between Execute and Writeback
		if(pipeline[0][5] == 1)                                        //If the decode stage had a valid instruction in the prev cycle 
		{
			pipeline[0][6] = 1;                     
			pipeline[1][6] = pipeline[1][5]; 
			pipeline[2][6] = pipeline[2][5];                           //READ THE REGISTER SPECIFICATION IF YOU ARE CONFUSED
			pipeline[3][6] = pipeline[3][5];
			pipeline[4][6] = pipeline[4][5];
			pipeline[5][6] = pipeline[5][5];
		}

		//Pipeline register between IQ and EXECUTE
		if(pipeline[0][4] == 1)                                        //If the decode stage had a valid instruction in the prev cycle 
		{
			pipeline[0][5] = 1;                     
			pipeline[1][5] = pipeline[1][4]; 
			pipeline[2][5] = pipeline[2][4];                           //READ THE REGISTER SPECIFICATION IF YOU ARE CONFUSED
			pipeline[3][5] = pipeline[3][4];
			pipeline[4][5] = pipeline[4][4];
			pipeline[5][5] = pipeline[5][4];
		}
		
		
		//Pipeline register between DISPATCH and ISSUE_QUEUE
		if(pipeline[0][3] == 1)                                        //If the decode stage had a valid instruction in the prev cycle 
		{
			pipeline[0][4] = 1;                     
			pipeline[1][4] = pipeline[1][3]; 
			pipeline[2][4] = pipeline[2][3];                           //READ THE REGISTER SPECIFICATION IF YOU ARE CONFUSED
			pipeline[3][4] = pipeline[3][3];
			pipeline[4][4] = pipeline[4][3];
			pipeline[5][4] = pipeline[5][3];
		}
		
		//Pipeline register between REGREAD and DISPATCH
		if(pipeline[0][2] == 1)                                        //If the decode stage had a valid instruction in the prev cycle 
		{
			pipeline[0][3] = 1;                     
			pipeline[1][3] = pipeline[1][2]; 
			pipeline[2][3] = pipeline[2][2];                           //READ THE REGISTER SPECIFICATION IF YOU ARE CONFUSED
			pipeline[3][3] = pipeline[3][2];
			pipeline[4][3] = pipeline[4][2];
			pipeline[5][3] = pipeline[5][2];
		}
		
		
		//Pipeline register between RENAME and REGREAD
		if(pipeline[0][1] == 1)                                        //If the decode stage had a valid instruction in the prev cycle 
		{
			pipeline[0][2] = 1;                     
			pipeline[1][2] = pipeline[1][1]; 
			pipeline[2][2] = pipeline[2][1];                           //READ THE REGISTER SPECIFICATION IF YOU ARE CONFUSED
			pipeline[3][2] = pipeline[3][1];
			pipeline[4][2] = pipeline[4][1];
			pipeline[5][2] = pipeline[5][1];
		}
		
		
		
		//Pipeline register between DECODE and RENAME
		if(pipeline[0][0] == 1)                                        //If the decode stage had a valid instruction in the prev cycle 
		{
			pipeline[0][1] = 1;                     
			pipeline[1][1] = pipeline[1][0]; 
			pipeline[2][1] = pipeline[2][0];                           //READ THE REGISTER SPECIFICATION IF YOU ARE CONFUSED
			pipeline[3][1] = pipeline[3][0];
			pipeline[4][1] = pipeline[4][0];
			pipeline[5][1] = pipeline[5][0];
		}
		
		
		//Pipeline register between FETCH and DECODE
		pipeline[0][0] = 1;                                          //means there is a valid instruction in the stage
		pipeline[1][0] = src1;                                      //These are the values in current cycle, but decode will need the values in previous cycle
		pipeline[2][0] = src2;
		pipeline[3][0] = dest;
		pipeline[4][0] = op_type;
		pipeline[5][0] = pc;
          		
		
		return 1;
	}
	
	return 0;
		
}