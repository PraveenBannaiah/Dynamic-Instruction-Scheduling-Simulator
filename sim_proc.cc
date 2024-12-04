#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sim_proc.h"
#include<iostream>
#include<iomanip>
#include<cmath>


////////////////////////////////////Defining global pipeline array////////////////////////////////////
pipeline* pipeline_objects;
/////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////Temporary signals//////////////////////////////////////////////
int temp_control_signal = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////                            



int main (int argc, char* argv[])
{	
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
    /*printf("rob_size:%lu "
            "iq_size:%lu "
            "width:%lu "
            "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);*/
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
	
	WIDTH = params.width;
	
	////////////////////////////////////////////////////////////////////////////////
	
	
	///////////////////////Creating an array of objects to mimic different lines of superscalar architecture/////////////////
	
	pipeline_objects = new pipeline[WIDTH];
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////Initialising the variables/////////////
	Initialisation_function();
	/////////////////////////////////////////////////////////////////////
	
	
	
	///////////////////////Assigning memory to the ROB array////////////////////////
	ROB = new long long int*[params.rob_size];
	for(int i=0;i<params.rob_size;i++)
	{	                          
	ROB[i] = new long long int[20];       
	}
	ROB_size = params.rob_size;
	NO_ROB_free_entries = ROB_size;     //For checking rob full conditions
	////////////////////////////////////////////////////////////////////////////////
	
	
	
	
	///////////////////////Assigning memory to the Issue Queue///////////////////////
	IQ = new int*[params.iq_size];
	for(int i=0;i<params.iq_size;i++)
		IQ[i] = new int[20];                              
	IQ_size = params.iq_size;                               

	////////////////////////////////////////////////////////////////////////////////	
	
	
	
	//////////////////////Initialising RMT/////////////////////////////////////////
	for(int i=0;i<67;i++)
		RMT_valid_array[i] = 0;                                 //Setting all valid bits to zero
	
	/////////////////////////////////////////////////////////////////////////////////
	
	
	
	/////////////////////Assigning memory to the writeback buffer//////////////////////
	
	WriteBack_buffer = new int*[WIDTH*8];                                       //Because WIDTH*5 is the maximum number of instructions that can finish in a given cycle
	for(int j=0;j<(WIDTH*8);j++)
		WriteBack_buffer[j] = new int[20];                                      //All instructions spend one cycle in Writeback buffer thats why there is no field for it
	writeback_free_entry_pointer = 0;
	
	/////////////////////////////////////////////////////////////////////////
	
	
	///////////////////Assigning memory to the execute list////////////////////////
	execute_list_free_entry_pointer = 0;
	execute_list = new int*[WIDTH*5];                     //Becuase we can have 5 instructions at a time in the execute list
	for(int j=0;j<(WIDTH*5);j++)
		execute_list[j] = new int[20];                   
														 
	///////////////////////////////////////////////////////////////////////////////


	///////////////////Assiging memory to wakeup buffer////////////////////
	Wakeup = new int[WIDTH*5];
	wakeup_pointer = 0;
	
	
	///////////////////////Main Loop////////////////////////////////////////////////
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
	
	//////////////////////////////////////////////////////////////////////////////////
	std::cout<<"# === Simulator Command =========";
	std::cout<<"\n# ./sim "<< params.rob_size <<" "<< params.iq_size<<" "<<params.width<<" "<< trace_file;
	std::cout<<"\n# === Processor Configuration ===";
	std::cout<<"\n# ROB_SIZE = "<<params.rob_size;
	std::cout<<"\n# IQ_SIZE  = "<<params.iq_size;
	std::cout<<"\n# WIDTH    = "<<params.width;
	std::cout<<"\n# === Simulation Results ========";
	std::cout<<"\n# Dynamic Instruction Count    = "<<seq_no;
	std::cout<<"\n# Cycles                       = "<<ticker;
	
	float ipc = float(seq_no) / ticker;
	ipc = std::round(ipc * 100.0) / 100.0;

	std::cout <<"\n# Instructions Per Cycle (IPC) = "<< std::fixed << std::setprecision(2) << ipc << std::endl;
	
	
	
    return 0;
}


void Initialisation_function()
{
	for(int i=0;i<WIDTH;i++)
	{
		pipeline_objects[i].no_clk_decode = 1;
		pipeline_objects[i].no_clk_RN = 1; 
		pipeline_objects[i].no_clk_RR = 1;
		pipeline_objects[i].no_clk_DI = 1;
		pipeline_objects[i].decode_cyles = 1;
		pipeline_objects[i].rename_cyles = 1;
		pipeline_objects[i].RR_cyles = 1;
		pipeline_objects[i].DI_cyles = 1;
		
		
	}
}

void Fetch(FILE *FP)
{
	int op_type, dest, src1,src2;
	long long int pc;
	
	//std::cout<<"\nFETCH";
	
	if((DE_can_accept_new_bundle)&&(EOF_reached == 0))
	{
		DE_contains_new_bundle = 1;
		for(int i =0; i<WIDTH;i++)
		{
		if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
			{
			INST_FETCH_CNT += 1;
			//printf("\n %lx %d %d %d %d\n", pc, op_type, dest, src1, src2);
			pipeline_objects[i].src1_fetch_decode = src1;
			pipeline_objects[i].src2_fetch_decode = src2;
			pipeline_objects[i].dest_fetch_decode = dest;
			pipeline_objects[i].op_type_fetch_decode = op_type;
			pipeline_objects[i].PC_fetch_decode = pc;
			
			pipeline_objects[i].entry_clk_fetch = ticker;
			
			}
		else                                                              //When we have fetch less than width functions
			{
				EOF_reached = 1;
			for(int j=i;j<WIDTH;j++)
			{
				pipeline_objects[j].src1_fetch_decode = -2;
				pipeline_objects[j].src2_fetch_decode = -2;
				pipeline_objects[j].dest_fetch_decode = -2;
				pipeline_objects[j].op_type_fetch_decode = -2;
				pipeline_objects[j].PC_fetch_decode = -2;
			}
			break;
			}
		}
		
	}

}


void Decode()
{
	//std::cout<<"\nDECODE";
	
	if(DE_initial_entry)                                             //TO make sure the data flows for the first time
	{
		DE_can_accept_new_bundle = 1;
		DE_initial_entry = 0;
		return;
	}
	
	
	if(RN_can_accept_new_bundle)
	{
		DE_can_accept_new_bundle = 1;
		
		
		for(int i=0;i<WIDTH;i++)
		{
			pipeline_objects[i].src1_RN = pipeline_objects[i].src1_fetch_decode;
			pipeline_objects[i].src2_RN = pipeline_objects[i].src2_fetch_decode;
			pipeline_objects[i].dest_RN = pipeline_objects[i].dest_fetch_decode;
			pipeline_objects[i].op_type_RN = pipeline_objects[i].op_type_fetch_decode;
			pipeline_objects[i].PC_RN = pipeline_objects[i].PC_fetch_decode;
			pipeline_objects[i].entry_clk_FD = pipeline_objects[i].entry_clk_fetch;
			pipeline_objects[i].no_clk_decode = pipeline_objects[i].decode_cyles;
			
			pipeline_objects[i].decode_cyles = 1;
		}
		
	}
	else
	{
		DE_can_accept_new_bundle = 0;
		RN_contains_new_bundle = 0;
		for(int i=0;i<WIDTH;i++)
		{
			pipeline_objects[i].decode_cyles += 1;
			//std::cout<<"\n Instruction stalling in decode:"<<pipeline_objects[i].dest_fetch_decode<<" for:"<<pipeline_objects[i].no_clk_decode;
		}
		//std::cout<<"\n Stalling in DECODE:"<<pipeline_objects[0].no_clk_decode;
	}
	
}


void Rename()
{
	
	//std::cout<<"\nRENAME";
	
	if((RN_initial_entry)&&(DE_initial_entry == 0))                                             //TO make sure the data flows for the first time
		{
			RN_can_accept_new_bundle = 1;
			RN_initial_entry = 0;
			return;
		}
	else if(RN_initial_entry)
		return;
	
	
	
	if((RR_can_accept_new_bundle)||(RR_is_actually_free))
	{
			
		if(NO_ROB_free_entries<WIDTH)
		{
			RN_can_accept_new_bundle = 0;
			
			//std::cout<<"\n Number of free rob entries:"<<NO_ROB_free_entries;
			//std::cout<<" RR_can_accept_new_bundle:"<<RR_can_accept_new_bundle;
			for(int i = 0;i<WIDTH;i++)
			{
				pipeline_objects[i].rename_cyles += 1;
				//std::cout<<"\n Instruction stalling in rename:"<<pipeline_objects[i].dest_RN<<" for:"<<pipeline_objects[i].rename_cyles;
				pipeline_objects[i].RR_cyles = 1;
			}
			
			RR_is_actually_free = 1;
			//return;
		}
		else
		{
			RR_is_actually_free = 0;
			RN_can_accept_new_bundle = 1;
			////////////////////////Assiging a rob entry for the instruction//////////////////////////
			////ROB[tail][0] = Does it have a destionation register?
			////ROB[tail][1] = ROB_tag
			////ROB[tail][2] = Destination
			////ROB[tail][3] = INST_ready?
			////ROB[tail][4] = PC
			for(int i=0;i<WIDTH;i++)
			{
				if((pipeline_objects[i].PC_RN == -2)||(pipeline_objects[i].src1_RN == -2)||(pipeline_objects[i].src2_RN == -2)||(pipeline_objects[i].dest_RN == -2))
					continue;                                                       //Implies not a valid instruction
				
				
				if(pipeline_objects[i].dest_RN == -1)
					ROB[ROB_tail_pointer][0] = -1;
				else
					ROB[ROB_tail_pointer][0] = 1;
				
				ROB[ROB_tail_pointer][1] = ROB_tags;
				
				ROB[ROB_tail_pointer][2] = pipeline_objects[i].dest_RN;
				
				ROB[ROB_tail_pointer][3] = 0;
				
				ROB[ROB_tail_pointer][4] = pipeline_objects[i].PC_RN;	
			
			
	
			//////////////////////Renaming the source register/////////////////////////
		
				if((pipeline_objects[i].src1_RN >= 0)&&(RMT_valid_array[pipeline_objects[i].src1_RN] == 1))
					pipeline_objects[i].src1_RR = RMT_tag[pipeline_objects[i].src1_RN];
				else
					pipeline_objects[i].src1_RR = pipeline_objects[i].src1_RN;
				
				if((pipeline_objects[i].src2_RN >= 0)&&(RMT_valid_array[pipeline_objects[i].src2_RN] == 1))
					pipeline_objects[i].src2_RR = RMT_tag[pipeline_objects[i].src2_RN];
				else
					pipeline_objects[i].src2_RR = pipeline_objects[i].src2_RN;
			
			
			////////////////////Renaming the destination register///////////////////////
			
				pipeline_objects[i].dest_RR = ROB[ROB_tail_pointer][1];
				
				
			////////////////////Updating the RMT/////////////////////////////////////////
			
				if(pipeline_objects[i].dest_RN >= 0)
				{					
					RMT_tag[pipeline_objects[i].dest_RN] = ROB[ROB_tail_pointer][1];
					RMT_valid_array[pipeline_objects[i].dest_RN] = 1;
				}
				else
				{
					RMT_tag[pipeline_objects[i].dest_RN] = pipeline_objects[i].dest_RN;
					RMT_valid_array[pipeline_objects[i].dest_RN] = 0;
				}
				
				
				
			/////////////////////Passing other signals to RR///////////////////////////
				pipeline_objects[i].op_type_RR = pipeline_objects[i].op_type_RN;
				pipeline_objects[i].PC_RR = pipeline_objects[i].PC_RN;
				
				pipeline_objects[i].no_clk_DERN = pipeline_objects[i].no_clk_decode;
				pipeline_objects[i].entry_clk_FDERN = pipeline_objects[i].entry_clk_FD;
				pipeline_objects[i].no_clk_RN = pipeline_objects[i].rename_cyles;
				
				pipeline_objects[i].src1_RR_OG = pipeline_objects[i].src1_RN;
				pipeline_objects[i].src2_RR_OG = pipeline_objects[i].src2_RN;
				pipeline_objects[i].dest_RR_OG = pipeline_objects[i].dest_RN;
				pipeline_objects[i].src1_RR_ready = 0;
				pipeline_objects[i].src2_RR_ready = 0;
				
				//if(ROB[ROB_tail_pointer][1] == 2475)
					//std::cout<<"\n RENAME CYCLES RENAME CYCLES RENAME CYCLES:"<<pipeline_objects[i].no_clk_RN;
			//////////////////////Resetting Timers///////////////////////////////////
				pipeline_objects[i].rename_cyles = 1;
				
				
			////////////////////Updating pointers and counters////////////////////////
			ROB_tail_pointer = (ROB_tail_pointer + 1)%ROB_size;
			NO_ROB_free_entries -= 1;
			ROB_tags += 1;
			
			}
			
			
		}
	}
	else
	{
		RN_can_accept_new_bundle = 0;
		//std::cout<<"\n Stalling in rename due to REGREAD";
		for(int i = 0;i<WIDTH;i++)
		{
			pipeline_objects[i].rename_cyles += 1;
			//std::cout<<"\n Instruction stalling in rename:"<<pipeline_objects[i].dest_RN<<" for:"<<pipeline_objects[i].rename_cyles;
		}
	}
	
	/*
	std::cout<<"\nPrinting ROB";
	for(int i = 0;i<ROB_size;i++)
		if(ROB[i] != 0)                                                        //If equal to zero then it there is not valid entry
			std::cout<<"\n dest?:"<<ROB[i][0]<<" ROB_tag:"<<ROB[i][1]<<" dest:"<<ROB[i][2]<<" RDY?:"<<ROB[i][3]<<" PC:"<<ROB[i][4];
	*/	
			
	
	
}




void RegRead()
{
	//std::cout<<"\nREGREAD";
	
	if((RR_initial_entry)&&(RN_initial_entry == 0))                                             //TO make sure the data flows for the first time
	{
		RR_can_accept_new_bundle = 1;
		RR_initial_entry = 0;
		return;
	}
	else if(RR_initial_entry)
		return;
	
	
	if((RR_is_actually_free)&&(DI_moved_along))
	{
		DI_is_actually_free = 1;
		//std::cout<<"\n Case 2";
		return;
	}
	else
		DI_is_actually_free = 0;
	
	
	if((DI_can_accept_new_bundle)||(DI_is_actually_free))
	{	
		RR_can_accept_new_bundle = 1;
		DI_moved_along = 0;
		//std::cout<<"\n REGREAD WENT IN1";
		
		
		
		for(int i =0 ;i<WIDTH; i++)
		{
		/////////////////////////Asserting readiness of source 1/////////////////////////
			if(pipeline_objects[i].src1_RR == -1)
				pipeline_objects[i].src1_ready = 1;
			else if(pipeline_objects[i].src1_RR >= 1000)
			{
				for(int k=0;k<ROB_size;k++)
				{
					if((ROB[k][1] == pipeline_objects[i].src1_RR)&&(ROB[k][3] == 1)&&(ROB[k][0] == 1))
					{	
						pipeline_objects[i].src1_ready = 1;
						break;
					}
					else
						pipeline_objects[i].src1_ready = 0;
				}
				
				if(pipeline_objects[i].src1_RR_ready == 1)
					pipeline_objects[i].src1_ready = 1;
			}
			else
				pipeline_objects[i].src1_ready = 1;
			
			
			
		/////////////////////////Asserting readiness of source 2/////////////////////////
			if(pipeline_objects[i].src2_RR == -1)
				pipeline_objects[i].src2_ready = 1;
			else if(pipeline_objects[i].src2_RR >= 1000)
			{
				for(int k=0;k<ROB_size;k++)
				{
					if((ROB[k][1] == pipeline_objects[i].src2_RR)&&(ROB[k][3] == 1)&&(ROB[k][0] == 1))
					{
						pipeline_objects[i].src2_ready = 1;
						break;
					}
					else
						pipeline_objects[i].src2_ready = 0;
					
				}
				if(pipeline_objects[i].src2_RR_ready == 1)
					pipeline_objects[i].src2_ready = 1;
				
			}
			else
				pipeline_objects[i].src2_ready = 1;
			
			
		///////////////////////Assiging other signals//////////////////////////////////////////////
			pipeline_objects[i].PC_DI = pipeline_objects[i].PC_RR;
			
			pipeline_objects[i].no_clk_RNRR = pipeline_objects[i].no_clk_RN;
			pipeline_objects[i].no_clk_DERNRR = pipeline_objects[i].no_clk_DERN;
			pipeline_objects[i].entry_clk_FDERNRR = pipeline_objects[i].entry_clk_FDERN;
			pipeline_objects[i].no_clk_RR = pipeline_objects[i].RR_cyles;
			
			pipeline_objects[i].src1_DI_OG = pipeline_objects[i].src1_RR_OG;
			pipeline_objects[i].src2_DI_OG = pipeline_objects[i].src2_RR_OG;
			pipeline_objects[i].dest_DI_OG = pipeline_objects[i].dest_RR_OG;
			pipeline_objects[i].src1_DI = pipeline_objects[i].src1_RR;
			pipeline_objects[i].src2_DI = pipeline_objects[i].src2_RR;
			pipeline_objects[i].dest_DI = pipeline_objects[i].dest_RR;
			pipeline_objects[i].op_type_DI = pipeline_objects[i].op_type_RR;
			
			
		////////////////Resetting Timiners//////////////////////////////////
			pipeline_objects[i].RR_cyles = 1;
			
		}
		
	}
	else
	{
		RR_can_accept_new_bundle = 0;
		
		if(RR_is_actually_free)
		{
			//std::cout<<"\n Case ! REGREAD";
			return;
		}
		
		
		for(int i = 0;i<WIDTH;i++)
		{
			pipeline_objects[i].RR_cyles += 1;
			//std::cout<<"\n Instruction stalling in RR:"<<pipeline_objects[i].dest_RR<<" for:"<<pipeline_objects[i].RR_cyles;
		}
		
	}
	
}


void Dispatch()
{
	//std::cout<<"\nDISPATCH";
	
	if((DI_initial_entry)&&(RR_initial_entry == 0))                                             //TO make sure the data flows for the first time
	{
		DI_can_accept_new_bundle = 1;
		DI_initial_entry = 0;
		return;
	}
	else if(DI_initial_entry)
		return;
	
	
		
	if((IQ_size - IQ_entry_pointer) >= WIDTH)
	{
		DI_can_accept_new_bundle = 1;
		
		//IQ[entry][0] = Valid instructions
		//IQ[entry][1] = Dest Tag, regardless of if it exists
		//IQ[entry][2] = RS1 RDY
		//IQ[entry][3] = RS1 TAG
		//IQ[entry][4] = RS2 RDY
		//IQ[entry][5] = RS2 TAG
		//IQ[entry][6] = RS1 OG 
		//IQ[entry][7] = RS2 OG
		//IQ[entry][8] = NO_clock_IQ
		//IQ[entry][9] = NO_clk_DI
		//IQ[entry][10] = NO_clk_RR
		//IQ[entry][11] = NO_CLK_RN
		//IQ[entry][12] = NO_CLK_DE
		//IQ[entry][13] = ENTRY_FE
		//IQ[entry][14] = AGE
		//IQ{entry][15] = op_type
		//IQ{entry][16] = dest_OG
		
		 
		for(int i = 0 ;i<WIDTH;i++)
		{
			for(int j=0;j<450;j++)
			{
				if(recently_issued[j] == pipeline_objects[i].dest_DI)
					goto label;
			}
			
			DI_moved_along = 1;
			
			
			IQ[IQ_entry_pointer][0] = 1;
			IQ[IQ_entry_pointer][1] = pipeline_objects[i].dest_DI;
			
			if(pipeline_objects[i].src1_ready == 1)
				IQ[IQ_entry_pointer][2] = 1;
			else
				IQ[IQ_entry_pointer][2] = 0;
			
			IQ[IQ_entry_pointer][3] = pipeline_objects[i].src1_DI;
			
			if(pipeline_objects[i].src2_ready == 1)
			{
				IQ[IQ_entry_pointer][4] = 1;
			}
			else
				IQ[IQ_entry_pointer][4] = 0;
			
			IQ[IQ_entry_pointer][5] = pipeline_objects[i].src2_DI;
			IQ[IQ_entry_pointer][6] = pipeline_objects[i].src1_DI_OG;
			IQ[IQ_entry_pointer][7] = pipeline_objects[i].src2_DI_OG;
			
			IQ[IQ_entry_pointer][9] = pipeline_objects[i].no_clk_DI;
			IQ[IQ_entry_pointer][10] = pipeline_objects[i].no_clk_RR;
			IQ[IQ_entry_pointer][11] = pipeline_objects[i].no_clk_RNRR;
			IQ[IQ_entry_pointer][12] = pipeline_objects[i].no_clk_DERNRR;
			IQ[IQ_entry_pointer][13] = pipeline_objects[i].entry_clk_FDERNRR;
			
			IQ[IQ_entry_pointer][14] = youngest;
			IQ[IQ_entry_pointer][15] = pipeline_objects[i].op_type_DI;
			IQ[IQ_entry_pointer][16] = pipeline_objects[i].dest_RR_OG;
			IQ[IQ_entry_pointer][8] = 0;
			
			//std::cout<<"\n Instruction going into inssue quuqu:"<<pipeline_objects[i].dest_DI<<" with DI cycles for:"<<pipeline_objects[i].no_clk_DI<< "RR Cycles:"<<pipeline_objects[i].no_clk_RR;
			
			
			
			///////////////Putting data into the recently issued buffer////////////////////
			recently_issued[recently_issued_free_entry] = pipeline_objects[i].dest_DI;
			recently_issued_free_entry = (recently_issued_free_entry + 1)%450;
			
				
			////////////////Resetting Timers///////////////////////////////////
			pipeline_objects[i].no_clk_DI = 1;
			
			
			
			////////////////Updating the pointers and counters//////////////////////
			IQ_entry_pointer += 1;
			youngest += 1;	
			
			label: continue;
		}
	}
	else
	{
		//std::cout<<"\n Issue queue full";
		//std::cout<<"\n IQ_size - IQ_entry_pointer:"<<IQ_size - IQ_entry_pointer;
		DI_can_accept_new_bundle = 0;
		
		if(DI_is_actually_free == 1)
		{
			DI_can_accept_new_bundle = 1;
			return;
		}
		
		for(int i = 0;i<WIDTH;i++)
		{
			pipeline_objects[i].no_clk_DI += 1;
			//std::cout<<"\n Instruction stalling in DI:"<<pipeline_objects[i].dest_DI<<" for:"<<pipeline_objects[i].no_clk_DI;
		}
	}
		
		
	/*
	std::cout<<"\nPrinting issue queue";
	for(int i=0;i<IQ_size;i++)
	{
		std::cout<<"\n Valid:"<<IQ[i][0]<<" dest:"<<IQ[i][1]<<" src1RDY:"<<IQ[i][2]<<" src1:"<<IQ[i][3]<<" src2RDY:"<<IQ[i][4]<<" src2:"<<IQ[i][5]<< " Cycles:"<<IQ[i][8]<<" age:"<<IQ[i][14];
	}
	*/
	
	
	

}


void Issue()
{
	//std::cout<<"\nISSUE";
	
	
	int oldest =0;
	int number_of_issued_inst = 0;
	
	////////Updating cyles in issue queue///////
	for(int i=0;i<IQ_entry_pointer;i++)
	{
		if(IQ[i][0] == 1)
			IQ[i][8] += 1;
	}
	///////////////////////////////////////////
	
	/*
	std::cout<<"\nPrinting issue queue in ISSUE stage";
	for(int i=0;i<IQ_size;i++)
	{
		std::cout<<"\n Valid:"<<IQ[i][0]<<" dest:"<<IQ[i][0]<<" src1RDY:"<<IQ[i][2]<<" src1:"<<IQ[i][3]<<" src2RDY:"<<IQ[i][4]<<" src2:"<<IQ[i][5]<<" cycles:"<<IQ[i][8]<<" TAG:"<<IQ[i][1];
	}
	*/
	
	
	
	
	
	for(int i=0;i<IQ_size;i++)
	{
		if((number_of_issued_inst == WIDTH)||(execute_list_free_entry_pointer == (WIDTH*5)))
			break;
		else
		{
			for(int j=0;j<IQ_size;j++)
			{
				oldest = j;
				
				if((IQ[j][0] == 1) && (IQ[j][14] == oldest) && (IQ[j][2] == 1) && (IQ[j][4] == 1))
				{
					if((number_of_issued_inst == WIDTH)||(execute_list_free_entry_pointer == (WIDTH*5)))
					{
						//std::cout<<"\n Not going into ISSUE update stage";
						//std::cout<<"\n execute list pointer:"<<execute_list_free_entry_pointer<<" number inst issued:"<<number_of_issued_inst;
						return;
					}
					//IQ[entry][0] = Valid instructions
					//IQ[entry][1] = Dest Tag, regardless of if it exists
					//IQ[entry][2] = RS1 RDY
					//IQ[entry][3] = RS1 TAG
					//IQ[entry][4] = RS2 RDY
					//IQ[entry][5] = RS2 TAG
					//IQ[entry][6] = RS1 OG 
					//IQ[entry][7] = RS2 OG
					//IQ[entry][8] = NO_clock_IQ
					//IQ[entry][9] = NO_clk_DI
					//IQ[entry][10] = NO_clk_RR
					//IQ[entry][11] = NO_CLK_RN
					//IQ[entry][12] = NO_CLK_DE
					//IQ[entry][13] = ENTRY_FE
					//IQ[entry][14] = AGE
					//IQ{entry][15] = op_type
					//IQ{entry][16] = dest_OG
					//EX{entry][16] = Latency
					
					execute_list[execute_list_free_entry_pointer][0] = IQ[j][1];														
					execute_list[execute_list_free_entry_pointer][1] = IQ[j][2];
					execute_list[execute_list_free_entry_pointer][2] = IQ[j][3];
					execute_list[execute_list_free_entry_pointer][3] = IQ[j][4];
					execute_list[execute_list_free_entry_pointer][4] = IQ[j][5];
					execute_list[execute_list_free_entry_pointer][5] = IQ[j][6];
					execute_list[execute_list_free_entry_pointer][6] = IQ[j][7];
					execute_list[execute_list_free_entry_pointer][7] = IQ[j][8];
					execute_list[execute_list_free_entry_pointer][8] = IQ[j][9];
					execute_list[execute_list_free_entry_pointer][9] = IQ[j][10];
					execute_list[execute_list_free_entry_pointer][10] = IQ[j][11];
					execute_list[execute_list_free_entry_pointer][11] = IQ[j][12];
					execute_list[execute_list_free_entry_pointer][12] = IQ[j][13];
					execute_list[execute_list_free_entry_pointer][13] = IQ[j][14];
					execute_list[execute_list_free_entry_pointer][14] = IQ[j][15];
					execute_list[execute_list_free_entry_pointer][15] = IQ[j][16];
					execute_list[execute_list_free_entry_pointer][16] = 0;
					
					//////////////Reducing age of instructions older than j//////////////
					for(int k=(j+1);k<(IQ_entry_pointer);k++)                                                        
						IQ[k][14] = IQ[k][14] - 1;
				
				
					////////////Deleting the Issue Queue row/////////////////////
					for(int k =j;k<(IQ_entry_pointer-1);k++)                                                         //Moving up the issue queue
					{
						
						IQ[k][0] = IQ[k+1][0];
						IQ[k][1] = IQ[k+1][1];
						IQ[k][2] = IQ[k+1][2];
						IQ[k][3] = IQ[k+1][3];
						IQ[k][4] = IQ[k+1][4];
						IQ[k][5] = IQ[k+1][5];
						IQ[k][6] = IQ[k+1][6];
						IQ[k][7] = IQ[k+1][7];
						IQ[k][8] = IQ[k+1][8];
						IQ[k][9] = IQ[k+1][9];
						IQ[k][10] = IQ[k+1][10];
						IQ[k][11] = IQ[k+1][11];
						IQ[k][12] = IQ[k+1][12];
						IQ[k][13] = IQ[k+1][13];
						IQ[k][14] = IQ[k+1][14];
						IQ[k][15] = IQ[k+1][15];
						IQ[k][16] = IQ[k+1][16];
					}
					
					IQ[IQ_entry_pointer - 1][0] = 0;                                                                   //Invalidating last entry
					
					
					////////////////Updating the pointers and the counters//////////////////////
					execute_list_free_entry_pointer += 1;
					number_of_issued_inst += 1;
					IQ_entry_pointer = IQ_entry_pointer -1;
					youngest -= 1;
					
					break;
				}
			}
		}
	}
	
	/*
	std::cout<<"\nPrinting issue queue after ISSUE stage";
	for(int i=0;i<IQ_size;i++)
	{
		std::cout<<"\n Valid:"<<IQ[i][0]<<" dest:"<<IQ[i][0]<<" src1RDY:"<<IQ[i][2]<<" src1:"<<IQ[i][3]<<" src2RDY:"<<IQ[i][4]<<" src2:"<<IQ[i][5]<<" cycles:"<<IQ[i][8]<<" TAG:"<<IQ[i][1];
	}
	*/
	
	
	
}



void Execute()
{
	/*
	std::cout<<"\nEXECUTE";
	
	std::cout<<"\n Printing execute list";
	for(int i=0;i<execute_list_free_entry_pointer;i++)
	{
		std::cout<<"\n tag:"<<execute_list[i][0]<<" cycles:"<<execute_list[i][16]<<" ISSUE CYCLES:"<<execute_list[i][7];
	}
	*/
	
	
	
	

	for(int i=0;i<execute_list_free_entry_pointer;i++)
	{
		//////////////////////////////Find instructions that are ending this cycle/////////////////////////////
		if((execute_list[i][14]==0)&&(execute_list[i][16]==0) || ((execute_list[i][14]==1)&&(execute_list[i][16]==1)) || ((execute_list[i][14]==2)&&(execute_list[i][16]==4)))
		{
			//std::cout<<"|n READY INST:"<<execute_list[i][0];
			
			////////////////////////Wakeup procedures/////////////////////////
			//////////IN IQ/////////
			for(int j=0;j<IQ_size;j++)
			{
				if(IQ[j][0] == 1)
				{
					if(IQ[j][3] == execute_list[i][0])
						IQ[j][2] = 1; 
					if(IQ[j][5] == execute_list[i][0])
						IQ[j][4] = 1; 
				}
			}
			
			/////////IN DI Bundle///
			for(int j=0;j<WIDTH;j++)
			{
				//std::cout<<"\n DI:"<<pipeline_objects[j].dest_DI<<" j:"<<j;
				if(pipeline_objects[j].src1_DI == execute_list[i][0])
				{
					pipeline_objects[j].src1_ready = 1;
				}
				if(pipeline_objects[j].src2_DI == execute_list[i][0])
					pipeline_objects[j].src2_ready = 1;
			}
			
			/////////IN RR Bundle///
			for(int j=0;j<WIDTH;j++)
			{
				//std::cout<<"\n  RR:"<<pipeline_objects[j].dest_RR<<" j:"<<j;;
				if(pipeline_objects[j].src1_RR == execute_list[i][0])                  //Becase the renamed values are not yet available so comparing with the og dest
					pipeline_objects[j].src1_RR_ready = 1;
				 
				
				if(pipeline_objects[j].src2_RR == execute_list[i][0])
					pipeline_objects[j].src2_RR_ready = 1;
				
			}
			
			
			///////////////////Adding the instruction to the writeback buffer///////////
			for(int j=0;j<17;j++)
				WriteBack_buffer[writeback_free_entry_pointer][j] = execute_list[i][j];
			
			WriteBack_buffer[writeback_free_entry_pointer][16] += 1;
			writeback_free_entry_pointer += 1;
			WriteBack_buffer[writeback_free_entry_pointer][16] += 1;
			
			
			///////////////////Removing the instruction from the execute list////////////////////////
			
			for(int j=i;j<execute_list_free_entry_pointer - 1;j++)
			{
				execute_list[j][0] = execute_list[j+1][0];
				execute_list[j][1] = execute_list[j+1][1];
				execute_list[j][2] = execute_list[j+1][2];
				execute_list[j][3] = execute_list[j+1][3];
				execute_list[j][4] = execute_list[j+1][4];
				execute_list[j][5] = execute_list[j+1][5];
				execute_list[j][6] = execute_list[j+1][6];
				execute_list[j][7] = execute_list[j+1][7];
				execute_list[j][8] = execute_list[j+1][8];
				execute_list[j][9] = execute_list[j+1][9];
				execute_list[j][10] = execute_list[j+1][10];
				execute_list[j][11] = execute_list[j+1][11];
				execute_list[j][12] = execute_list[j+1][12];
				execute_list[j][13] = execute_list[j+1][13];
				execute_list[j][14] = execute_list[j+1][14];
				execute_list[j][15] = execute_list[j+1][15];
				execute_list[j][16] = execute_list[j+1][16];
			}
			execute_list_free_entry_pointer -= 1;
			
			i = i -1;                                                    //Because we are reducing execute list pointer
		}
	}
	
	
	
	////////////////Updating the cycles//////////////////////
	for(int i =0;i<execute_list_free_entry_pointer;i++)
		execute_list[i][16] += 1;
	
	
}



void Writeback()
{
	/*
	std::cout<<"\nWRITEBACK";
	
	
	std::cout<<"\n Printing writeback buffer";
	for(int i=0;i<writeback_free_entry_pointer;i++)
	{
		std::cout<<"\n tag:"<<WriteBack_buffer[i][0]<<" cycles:"<<WriteBack_buffer[i][16]<<" ISSUE queue cycles:"<<WriteBack_buffer[i][7];
	}
	*/
	
	
	
	for(int i =0;i<writeback_free_entry_pointer;i++)
	{
		for(int j=0;j<ROB_size;j++)
		{
			if(WriteBack_buffer[i][0] == ROB[j][1])
			{
				ROB[j][3] = 1;
				
				////////////Also passing other information to print////////////
				ROB[j][5] = WriteBack_buffer[i][5];          //RS1 original
				ROB[j][6] = WriteBack_buffer[i][6];          //RS2 origianl
				ROB[j][7] = WriteBack_buffer[i][15];		 //Dest original
				ROB[j][8] = WriteBack_buffer[i][14];         //op type
				ROB[j][9] = WriteBack_buffer[i][7];          //CLK_IQ
				ROB[j][10] = WriteBack_buffer[i][8];         //CLK_DI
				ROB[j][11] = WriteBack_buffer[i][9];         //CLK_RR
				ROB[j][12] = WriteBack_buffer[i][10];        //CLK_RN
				ROB[j][13] = WriteBack_buffer[i][11];        //CLK_DE
				ROB[j][14] = WriteBack_buffer[i][12];        //ENTRY FE
				ROB[j][15] = WriteBack_buffer[i][16];        //CLK_EX
				ROB[j][16] = 0;
				
			}
		}
	}
	
	//////Clearing the writeback buffer///////////
	writeback_free_entry_pointer = 0;
	
}



void Retire()
{
	//std::cout<<"\nRETIRE";
	
	
	int inst_retired_this_cycle = 0;
	
	//////////updating cycles spent in retire stage////////////
	//ROB[][16] represent the cycles 
	for(int i = 0 ; i<ROB_size; i++)
	{
		if(ROB[i][3] == 1)
			ROB[i][16] += 1;
	}
	
	//////////Retiring///////////
	for(int i = 0;i<WIDTH;i++)                                                           //WE can retire upto WIDTH instructions in a given cycles
	{		
		if(INST_RETIRE_CNT == 10000)
			break;
		
		if((ROB[ROB_head_pointer][3] == 1)&&(ROB[ROB_head_pointer][0] != 0))
		{
			//////////////////Updating the RMT/////////////////////////
			if(RMT_tag[ROB[ROB_head_pointer][2]] == ROB[ROB_head_pointer][1])
			{
				if(ROB[ROB_head_pointer][0] == 1)                                         //Checking if the inst actually has a destination register
					RMT_valid_array[ROB[ROB_head_pointer][2]] = 0;
			}
			
			//std::cout<<"\n Retiring inst tag:"<<ROB[ROB_head_pointer][1]<<" ISSUE queue cycles:"<<WriteBack_buffer[i][9];
			
			/////////////////Printing outputs/////////////////////////
			int j = ROB_head_pointer;                                            //Used to mitigate writing long lines of code
			
			std::cout<<" "<<seq_no<<" fu{"<<ROB[ROB_head_pointer][8]<<"}";
			std::cout<<" src{"<<ROB[ROB_head_pointer][5]<<","<<ROB[ROB_head_pointer][6]<<"} dst{"<<ROB[ROB_head_pointer][2]<<"}";
			std::cout<<" FE{"<<ROB[ROB_head_pointer][14]<<",1}";
			std::cout<<" DE{"<<ROB[ROB_head_pointer][14] + 1<<","<<ROB[ROB_head_pointer][13]<<"}";
			std::cout<<" RN{"<<ROB[ROB_head_pointer][14] + 1 + ROB[j][13]<<","<<ROB[j][12]<<"}";
			std::cout<<" RR{"<<ROB[ROB_head_pointer][14] + 1 + ROB[j][13] + ROB[j][12]<<","<<ROB[j][11]<<"}";
			std::cout<<" DI{"<<ROB[ROB_head_pointer][14] + 1 + ROB[j][13] + ROB[j][12] + ROB[j][11]<<","<<ROB[j][10]<<"}";
			std::cout<<" IS{"<<ROB[ROB_head_pointer][14] + 1 + ROB[j][13] + ROB[j][12] + ROB[j][11] + ROB[j][10]<<","<<ROB[j][9]<<"}";
			std::cout<<" EX{"<<ROB[ROB_head_pointer][14] + 1 + ROB[j][13] + ROB[j][12] + ROB[j][11] + ROB[j][10] + ROB[j][9]<<","<< ROB[j][15]<<"}";
			std::cout<<" WB{"<<ROB[ROB_head_pointer][14] + 1 + ROB[j][13] + ROB[j][12] + ROB[j][11] + ROB[j][10] + ROB[j][9] + ROB[j][15]<<",1}";
			std::cout<<" RT{"<<ROB[ROB_head_pointer][14] + 1 + ROB[j][13] + ROB[j][12] + ROB[j][11] + ROB[j][10] + ROB[j][9] + ROB[j][15] + 1<<","<<ROB[j][16]<<"} \n";
			
			       
			
			
			/////////////////Updating variables///////////////////////
			NO_ROB_free_entries += 1;
			inst_retired_this_cycle += 1;
			ROB_head_pointer += 1;
			ROB_head_pointer = ROB_head_pointer%ROB_size;
			INST_RETIRE_CNT += 1;
			seq_no += 1;
		}
	}
	
	/*
	std::cout<<"\nPrinting ROB in retire stage";
	for(int i = 0;i<ROB_size;i++)
		if(ROB[i] != 0)                                                        //If equal to zero then it there is not valid entry
			std::cout<<"\n dest?:"<<ROB[i][0]<<" ROB_tag:"<<ROB[i][1]<<" dest:"<<ROB[i][2]<<" RDY?:"<<ROB[i][3]<<" PC:"<<ROB[i][4]<<" ISSUE CYCLES:"<<ROB[i][9];
			
	*/
	
}



int Advance_Cycle()
{
	ticker = ticker + 1;          //Updating the global clock
	//////////////////////////////////////////////////////////////
	
	
	if((INST_FETCH_CNT == INST_RETIRE_CNT))
	{
		//std::cout<<"\n inst count:"<<INST_FETCH_CNT<<"  retire count:"<<INST_RETIRE_CNT;
		//std::cout<<"\n ROB_head:"<<ROB_head_pointer<<" ROB_tail:"<<ROB_tail_pointer;
		return 0;
	}
	else
	{
		//std::cout<<"\n inst count:"<<INST_FETCH_CNT<<"  retire count:"<<INST_RETIRE_CNT;
		//std::cout<<"\n ROB_head:"<<ROB_head_pointer<<" ROB_tail:"<<ROB_tail_pointer;
		return 1;
	}
	
	
	
	
	
	/*
	std::cout<<"\n TICKER TICKER TICKER:"<<ticker;
	
	if(temp_control_signal ==2465)
		return 0;
	else
	{
		//std::cout<<"\n inst count:"<<INST_FETCH_CNT<<"  retire count:"<<INST_RETIRE_CNT;
		std::cout<<"\n ROB_head:"<<ROB_head_pointer<<" ROB_tail:"<<ROB_tail_pointer;
		temp_control_signal += 1;
		return 1;
	}
	*/

	
	
	
}