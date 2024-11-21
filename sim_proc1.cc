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


///////Count variables//////
long int INST_FETCH_CNT = 0;
long int INST_RETIRE_CNT = 0;
int WIDTH = 0;

//Starting for scalar processor
unsigned long int pipeline[6][9];                 //9 because we have a 9 stage pipelined
// pipeline[][0] -> Fetch; pipeline[][1] -> Decode ; Pipeline[][2] -> Rename
//Pipeline[0] -> inst there or not; Pipeline[1] ->src1 ; pipeline[2] ->src2; pipeline[3] ->dest ; pipeline[4] -> type ; pipeline[5] ->PC

	

//Creating a separate pipeline register for execute and writeback stage because they require more fields
int** pipeline_execute = nullptr;
int In_flight_inst = 0;


//Reoder Buffer initialisation 
unsigned long int** ROB = nullptr;
int ROB_head_pointer = 0;
int ROB_tail_pointer = 0;
int ROB_tail_phase = 0;
int ROB_size = 0;


//Issue Queue initialisation
int** IQ = nullptr;
int IQ_size = 0;
int IQ_entry_pointer = 0;


//Rename Map Table
int RMT_valid_array[67];
int RMT_tag[67];


//Pipeline register contents for scalar processor
int op_type, dest, src1,src2;
unsigned int pc;


int cycle_stop_signal = 0;

int main (int argc, char* argv[])
{
	///////////////////////////INITIALISATION////////////////////////////
	//Resetting the pipeline
	for(int i=0;i<9;i++)
		pipeline[0][i] = 0;                //Means there are not instructions in any stage of the pipeline
	
	
	for(int i=0;i<67;i++)
		RMT_valid_array[i] = 0;                                 //Setting all valid bits to zero
	
	
	
	/////////////////////////////////////////////////////////////////////
	
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



	WIDTH = params.width;
	
	///////////////////////Assigning memory to the ROB array////////////////////////
	ROB = new unsigned long int*[params.rob_size];
	for(int i=0;i<params.rob_size;i++)
	{	                          
	ROB[i] = new unsigned long int[7];       //[][0] valid_dest; [][1] value(used as rob tag); [][2] dst; [][3] rdy; [][4] exc; [][5] mis; [][6] pc
                                              //Use 10001, 10002 etc as rob tag and when accesing just do index%1000
	}
	ROB_size = params.rob_size;
	
	
	///////////////////////Assigning memory to the Issue Queue///////////////////////
	IQ = new int*[params.iq_size];
	for(int i=0;i<params.iq_size;i++)
		IQ[i] = new int[6];                               //[][0] valid; [][1]dst tag, rs1 rdy, rs1 tag/value,rs2 rdy, rs2 tag/value 
	IQ_size = params.iq_size;
	
	
	
	//////////////////Assigning memory to the Execute stage Register/////////////////
	pipeline_execute = new int*[params.width * 5];
	for(int i=0;i<(params.width * 5);i++)
		pipeline_execute = new int[7];                               //0: Completed, 1: dest, 2:src1, 3:src2, 4:op_type, 5:no_cyles_in_exe, 6:Valid_inst
	
	//Initialising 
	for(int i=0;i<(params.width * 5);i++)
	{
		pipeline_execute[i][0] = 0;
		pipeline_execute[i][1] = 0;
		pipeline_execute[i][2] = 0;
		pipeline_execute[i][3] = 0;
		pipeline_execute[i][4] = 0;
		pipeline_execute[i][5] = 0;
	}
	
	
	
	//Main loop
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
	
	if(ROB[ROB_head_pointer][3] == 1)                                   //Checking if the head of the ROB is ready to be retired
	{
		//Step 1, Update the ARF, not implementing here
		//step 2, check with the RMT
		if(RMT_tag[ROB[ROB_head_pointer][2]] == ROB[ROB_head_pointer][1])      //Checking if we have to update the RMT
		{
			RMT_valid_array[ROB[ROB_head_pointer][2]] = 0;                     //Invalidating that particular entry
		}
		
		ROB_head_pointer += 1;
		ROB_head_pointer = ROB_head_pointer%ROB_size;
		INST_RETIRE_CNT += 1;
	}
	
	
	std::cout<<"\n Printing ROB Contents";
	for(int i=0;i<ROB_size;i++)
		std::cout<<"\n Valid:"<<ROB[i][0]<<" robTag:"<<ROB[i][1]<<" dst:R"<<ROB[i][2]<<" rdy:"<<ROB[i][3]<<" exc:"<<ROB[i][4]<<" mis:"<<ROB[i][5]<<" pc:"<<ROB[i][6];
}

void Writeback()
{
	std::cout<<"\n In writeback stage";
}

void Execute()
{
	std::cout<<"\n In Execute stage";
	
	
	
	for(int i =0;i<WIDTH*5;i++)
	{
		if(pipeline_execute[i][6] == 1) 
			pipeline_execute[i][5] += 1;                                        //Updating the number of cycles
	}
	
	
	for(int i=0;i<WIDTH * 5;i++)
	{
		if(pipeline_execute[i][4] == 0)
		{
			if(pipeline_execute[i][5] == 1)
				pipeline[i][0] = 1;
		}
		
		else if(pipeline_execute[i][4] == 1)
		{
			if(pipeline_execute[i][5] == 2)
				pipeline[i][0] = 1;
		}
		else if(pipeline_execute[i][4] == 2)
		{
			if(pipeline_execute[i][5] == 5)
				pipeline[i][0] = 1;
		}
	}
	
	
	//Do something about the completed instructions 
	//Need to implement register bypass
	
	
}

void Issue()
{
	std::cout<<"\n In Issue stage";                       //if both the source operands are ready then issue it to the execute stage
	

	for(int j=0; j<IQ_size;j++){
	if((IQ[j][2] == 1)&&(IQ[j][4]))
	{
		
		//Updating the pipeline register
		pipeline[1][4] = IQ[j][3];
		pipeline[2][4] = IQ[j][5];
		pipeline[3][4] = IQ[j][1];
		
		
		
		for(int i=j;i<(IQ_size-1);i++)
		{
			IQ[i][0] = IQ[i+1][0];                         //Moving the issue queue up
			IQ[i][1] = IQ[i+1][1];
			IQ[i][2] = IQ[i+1][2];
			IQ[i][3] = IQ[i+1][3];
			IQ[i][4] = IQ[i+1][4];
			IQ[i][5] = IQ[i+1][5];
		}
		
		IQ[IQ_size-2][0] = IQ[IQ_size-1][0];                         //Moving the issue queue up for the last element
		IQ[IQ_size-2][1] = IQ[IQ_size-1][1];
		IQ[IQ_size-2][2] = IQ[IQ_size-1][2];
		IQ[IQ_size-2][3] = IQ[IQ_size-1][3];
		IQ[IQ_size-2][4] = IQ[IQ_size-1][4];
		IQ[IQ_size-2][5] = IQ[IQ_size-1][5];
		
		break;                                                //For now move only one instruction to the execute stage
		
	}
	}
}

void Dispatch()
{
	if(IQ_entry_pointer < IQ_size){
	std::cout<<"\n In Dispatch stage";                    //Not yet coding the control signal 
	IQ[IQ_entry_pointer][0] = 1;                         //Setting valid bit as 1 
	IQ[IQ_entry_pointer][1] = pipeline[3][3];            //Destination field of the Issue Queue
	IQ[IQ_entry_pointer][3] = pipeline[1][3];            //Storing the tag/value
	IQ[IQ_entry_pointer][5] = pipeline[2][3];            //storing the tag/value for source register 2
	
	if(pipeline[1][3] >= 2000)                           //If it >= 2000 means that the value is ready
	{
		IQ[IQ_entry_pointer][2] = 1;
	}
	
	if(pipeline[2][3] >= 2000)                           //If it >= 2000 means that the value is ready for soruce register 2
	{
		IQ[IQ_entry_pointer][4] = 1;
	}
	
	IQ_entry_pointer += 1;
	}
	
	
	/////Printing ISSUE QUEUE//////
	std::cout<<"\n Printing issue queue";
	for(int i=0;i<IQ_size;i++)
		if(IQ[i][0] == 1)
			std::cout<<"\n dst reg:R"<<IQ[i][1]<<" rs1 rdy:"<<IQ[i][2]<<" rs1 tag:"<<IQ[i][3]<<" rs2 rdy:"<<IQ[i][4]<<" rs2 tag:"<<IQ[i][5];
	
}

void RegRead()
{
	std::cout<<"\n In RegRead stage";
	
	//Here we need to check the values needed are ready for source register 1
	if(pipeline[1][2] >= 1000)                                          //Checking if there is a rob entry and hence a producer
	{
		for(int i=0;i<ROB_size;i++)
		{
			if(ROB[i][1] == pipeline[1][2])                             //Finding the ROB entry
				if(ROB[i][3] == 1)                                      //Checking if the destination register is ready
				{
					pipeline[1][2] = 2000;                              //If the value is 2000 then it means that the reg was ready and data is read 
					break;
				}
		}
	}
	else                                                                 //For immediate values and ARF register values
		pipeline[1][2] = 2000;
		
		
		
	//For source register 2
	if(pipeline[2][2] >= 1000)                                          //Checking if there is a rob entry and hence a producer
	{
		for(int i=0;i<ROB_size;i++)
		{
			if(ROB[i][1] == pipeline[2][2])                             //Finding the ROB entry
				if(ROB[i][3] == 1)                                      //Checking if the destination register is ready
				{
					pipeline[2][2] = 2000;                              //If the value is 2000 then it means that the reg was ready and data is read 
					break;
				}
		}
	}
	else                                                                 //For immediate values and ARF register values
		pipeline[2][2] = 2000;
}

void Rename()
{
	std::cout<<"\n In Rename stage";
	
	if(pipeline[0][1])                                                                    //Go into rename only if we have new values in decode stage
	{                                    
	//////////////////////STEP 1 of RENAME STAGE//////////////////////////////////////
	if((ROB_head_pointer == ROB_tail_pointer)&&(ROB_tail_phase == 1))                         //ROB Full case
		std::cout<<"\n ROB Full";
	else if((ROB_head_pointer == ROB_tail_pointer)&&(ROB_tail_phase == 0))                    //ROB empty case
	{
		std::cout<<"\n ROB empty";
		ROB_tail_phase = 1;
		
		if(pipeline[3][1] == -1)                   //Indicates no destination register
			ROB[ROB_tail_pointer][0] = 0;          //Indicates no destination register and need not update RMT_rob_tag
		else
			ROB[ROB_tail_pointer][0] = 1;
		
		ROB[ROB_tail_pointer][1] = 1000 + ROB_tail_pointer + ROB_tail_phase;               //Unique rob tag
		ROB[ROB_tail_pointer][2] = pipeline[3][1];                                         //destination register
		ROB[ROB_tail_pointer][3] = 0;                                         //destination ready flag
		ROB[ROB_tail_pointer][4] = 0;                                         //exc, no idea what this is
		ROB[ROB_tail_pointer][5] = 0;                                         //mispredict flag
		ROB[ROB_tail_pointer][6] = pipeline[5][1];
		
		ROB_tail_pointer += 1;
		ROB_tail_pointer = ROB_tail_pointer%ROB_size;                         //Because it is a circular FIFO
		
		
		//////////////Renaming the destination register in RMT///////////////
		if(pipeline[3][1] != -1){
		if(ROB[ROB_tail_pointer-1][0]  == 0)                                           //Meaning There is no ROB entry for this insts
			RMT_tag[pipeline[3][1]] = pipeline[3][1];                                      //Renaming it to the register name itself
		else{
			RMT_tag[pipeline[3][1]] = ROB[ROB_tail_pointer - 1][1];                         //ROB_tail_pointer - 1 because it was updated
			RMT_valid_array[pipeline[3][1]] = 1;                                           //Updating the valid bit for the given destination register
		}
		}
		
	}
	
	else{
		if(pipeline[3][1] == -1)                   //Indicates no destination register
			ROB[ROB_tail_pointer][0] = 0;          //Indicates no destination register and need not update RMT_rob_tag
		else
			ROB[ROB_tail_pointer][0] = 1;
		
		ROB[ROB_tail_pointer][1] = 1000 + ROB_tail_pointer + ROB_tail_phase;               //Unique rob tag
		ROB[ROB_tail_pointer][2] = pipeline[3][1];                                         //destination register
		ROB[ROB_tail_pointer][3] = 0;                                         //destination ready flag
		ROB[ROB_tail_pointer][4] = 0;                                         //exc, no idea what this is
		ROB[ROB_tail_pointer][5] = 0;                                         //mispredict flag
		ROB[ROB_tail_pointer][6] = pipeline[5][1];
		
		ROB_tail_pointer += 1;
		
		//////////////Renaming the destination register in RMT///////////////
		if(pipeline[3][1] != -1){                                                           //Only update the RMT if there is a valid destination register
		if(ROB[ROB_tail_pointer-1][0]  == 0)                                           //Meaning There is no ROB entry for this insts
			RMT_tag[pipeline[3][1]] = pipeline[3][1];                                      //Renaming it to the register name itself
		else{
			RMT_tag[pipeline[3][1]] = ROB[ROB_tail_pointer - 1][1];                         //ROB_tail_pointer - 1 because it was updated
			RMT_valid_array[pipeline[3][1]] = 1;                                           //Updating the valid bit for the given destination register
		}
		}
		
	}
	
	
	
	/////////////////////////////////////Step 2 of RENAMING THE SOURCE REGISTERS//////////////////////////////////////////
	
	if(pipeline[1][1] != -1)                                                              //Checking if there is a soruce register 1
	{
		if(RMT_valid_array[pipeline[1][1]] != 0)                                         //Checking if there is a recent version
			pipeline[1][1] = RMT_tag[pipeline[1][1]];
	}
	
	if(pipeline[2][1] != -1)                                                              //Checking if there is a soruce register 2
	{
		if(RMT_valid_array[pipeline[2][1]] != 0)                                         //Checking if there is a recent version
			pipeline[2][1] = RMT_tag[pipeline[2][1]];
	}
	
	
	
	std::cout<<"\n Printing RMT Contents";
	for(int i=0;i<67;i++)
	{
		if(RMT_valid_array[i] == 1)                                                         //Only printing entries with valid rob tag
		std::cout<<"\nValid:"<<RMT_valid_array[i]<<" Reg:R"<<RMT_tag[i]<<" Og reg:R"<<i;
	}
	
	std::cout<<"\n Printing ROB Contents";
	for(int i=0;i<ROB_size;i++)
		std::cout<<"\n Valid:"<<ROB[i][0]<<" robTag:"<<ROB[i][1]<<" dst:R"<<ROB[i][2]<<" rdy:"<<ROB[i][3]<<" exc:"<<ROB[i][4]<<" mis:"<<ROB[i][5]<<" pc:"<<ROB[i][6];
	
	
	}
	
	else{
		std::cout<<"\n Waiting for valid instruction";
	}
}

void Decode()
{
	std::cout<<"\n In decode stage";
	
	std::cout<<"\n source 1: R"<<pipeline[1][0]<<" source 2: R"<<pipeline[2][0]<<" destination: R"<<pipeline[3][0]<<" op type: "<<pipeline[4][0]<<" pc:"<<pipeline[5][0];
	
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
		
		INST_FETCH_CNT += 1;
	}
	else
		
		cycle_stop_signal = 1;
}
int Advance_Cycle()
{
	std::cout<<"\n Cycle finished";
	if(INST_RETIRE_CNT != INST_FETCH_CNT)
	{
		//In the next cycle the values propagate to the next stage of the pipeline ; going ulta is better
		//Pipeline between Execute and Writeback
		
			pipeline[0][6] = 1;                     
			pipeline[1][6] = pipeline_execute[1][5];                 //Just input the destination registers because only that needs to be written back and check for -1
			pipeline[2][6] = pipeline[2][5];                           //READ THE REGISTER SPECIFICATION IF YOU ARE CONFUSED
			pipeline[3][6] = pipeline[3][5];
			pipeline[4][6] = pipeline[4][5];
			pipeline[5][6] = pipeline[5][5];

		//Pipeline register between IQ and EXECUTE
		if(pipeline[0][6] == 1)                                        //Because we need to go ulta 
		{
			if(IQ_entry_pointer <= IQ_size)
				pipeline[0][5] = 1; 
			else
				pipeline[0][5] = 0;
			pipeline[1][5] = pipeline[1][4]; 
			pipeline[2][5] = pipeline[2][4];                           //READ THE REGISTER SPECIFICATION IF YOU ARE CONFUSED
			pipeline[3][5] = pipeline[3][4];
			pipeline[4][5] = pipeline[4][4];
			pipeline[5][5] = pipeline[5][4];
		
			//Assigning to the execute pipeline registers
			if(In_flight_inst <= WIDTH * 5){
			pipeline_execute[In_flight_inst][0] = 0;                                //Inst not completed
			pipeline_execute[In_flight_inst][1] = pipeline[3][5];
			pipeline_execute[In_flight_inst][2] = pipeline[1][5];
			pipeline_execute[In_flight_inst][3] = pipeline[2][5];
			pipeline_execute[In_flight_inst][4] = pipeline[4][5];
			pipeline_execute[In_flight_inst][6] = 1;
			In_flight_inst += 1;
			}
			else{
				for(int i =0;i<WIDTH*5;i++)
				{
					if(pipeline_execute[i][0] == 1)
					{
						pipeline_execute[i][0] = 0;                                //Inst not completed
						pipeline_execute[i][1] = pipeline[3][5];
						pipeline_execute[i][2] = pipeline[1][5];
						pipeline_execute[i][3] = pipeline[2][5];
						pipeline_execute[i][4] = pipeline[4][5];
						pipeline_execute[i][6] = 1;
						break;
					}
				}			
			}
		}
		
		
		//Pipeline register between DISPATCH and ISSUE_QUEUE
		if(pipeline[0][5] == 1)                                        //Check if issue queue is empty       //ISSUE QUEUE is the reg here actually
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
		if(dest == -1)
			dest = 666;                                                 //we only have 67 registers so I can use this to tackl eunsigned int error cause by -1
		if(src1 == -1)
			src1 = 666;
		if(src2 == -1)
			src2 == 666;
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