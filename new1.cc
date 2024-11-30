#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sim_proc.h"
#include<iostream>


////////////////////////////////////Defining global pipeline array////////////////////////////////////
pipeline* pipeline_objects;
/////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////Temporary signals//////////////////////////////////////////////
int temp_control_signal = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////


unsigned seq_no = 0;                              



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
		
		
	}
}

void Fetch(FILE *FP)
{
	int op_type, dest, src1,src2;
	long long int pc;
	
	
	if((DE_can_accept_new_bundle)&&(EOF_reached == 0))
	{
		DE_contains_new_bundle = 1;
		for(int i =0; i<WIDTH;i++)
		{
		if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
			{
			INST_FETCH_CNT += 1;
			printf("\n %lx %d %d %d %d\n", pc, op_type, dest, src1, src2);
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
	else
		DE_contains_new_bundle = 0;
}


void Decode()
{
	if((DE_contains_new_bundle)&&(RN_can_accept_new_bundle))
	{
		RN_contains_new_bundle = 1;
		for(int i=0;i<WIDTH;i++)
		{
			pipeline_objects[i].src1_RN = pipeline_objects[i].src1_fetch_decode;
			pipeline_objects[i].src2_RN = pipeline_objects[i].src2_fetch_decode;
			pipeline_objects[i].dest_RN = pipeline_objects[i].dest_fetch_decode;
			pipeline_objects[i].op_type_RN = pipeline_objects[i].op_type_fetch_decode;
			pipeline_objects[i].PC_RN = pipeline_objects[i].PC_fetch_decode;
			pipeline_objects[i].entry_clk_FD = pipeline_objects[i].entry_clk_fetch;
		}
		
	}
	else
	{
		RN_contains_new_bundle = 0;
		for(int i=0;i<WIDTH;i++)
		{
			pipeline_objects[i].no_clk_decode += 1;
		}
	}
}


void Rename()
{
	if(RN_contains_new_bundle)
	{
		if((RR_can_accept_new_bundle == 0)||(NO_ROB_free_entries<WIDTH))
		{
			RR_contains_new_bundle = 0;
			
			RN_can_accept_new_bundle = 0;
			
			for(int i = 0;i<WIDTH;i++)
				pipeline_objects[i].no_clk_RN += 1;
			return;
		}
		else
		{
			RR_contains_new_bundle = 1;
			////////////////////////Assiging a rob entry for the instruction//////////////////////////
			////ROB[tail][0] = Does it have a destionation register?
			////ROB[tail][1] = ROB_tag
			////ROB[tail][2] = Destination
			////ROB[tail][3] = INST_ready?
			////ROB[tail][4] = PC
			for(int i=0;i<WIDTH;i++)
			{
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
				
				RMT_tag[pipeline_objects[i].dest_RN] = ROB[ROB_tail_pointer][1];
				
				
				
			/////////////////////Passing other signals to RR///////////////////////////
				pipeline_objects[i].op_type_RR = pipeline_objects[i].op_type_RN;
				pipeline_objects[i].PC_RR = pipeline_objects[i].PC_RN;
				pipeline_objects[i].no_clk_DERN = pipeline_objects[i].no_clk_decode;
				pipeline_objects[i].entry_clk_FDERN = pipeline_objects[i].entry_clk_FD;
				pipeline_objects[i].src1_RR_OG = pipeline_objects[i].src1_RN;
				pipeline_objects[i].src2_RR_OG = pipeline_objects[i].src2_RN;
				pipeline_objects[i].dest_RR_OG = pipeline_objects[i].dest_RN;
				
				
				
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
		
		RR_contains_new_bundle = 0;
		for(int i = 0;i<WIDTH;i++)
			pipeline_objects[i].no_clk_RN += 1;
	}
	
	std::cout<<"\nPrinting ROB";
	for(int i = 0;i<ROB_size;i++)
		if(ROB[i] != 0)                                                        //If equal to zero then it there is not valid entry
			std::cout<<"\n dest?:"<<ROB[i][0]<<" ROB_tag:"<<ROB[i][1]<<" dest:"<<ROB[i][2]<<" RDY?:"<<ROB[i][3]<<" PC:"<<ROB[i][4];
	
}




void RegRead()
{
	if((RR_contains_new_bundle)&&(DI_can_accept_new_bundle))
	{
		DI_contains_new_bundle = 1;
		RR_can_accept_new_bundle = 1;
		
		for(int i =0 ;i<WIDTH; i++)
		{
			
		/////////////////////////Asserting readiness of source 1/////////////////////////
			if(pipeline_objects[i].src1_RR == -1)
				pipeline_objects[i].src1_ready = 1;
			else if(pipeline_objects[i].src1_RR >= 1000)
			{
				int j = ROB_tail_pointer;
				while(1)
				{
					if((ROB[j][1] == pipeline_objects[i].src1_RR)&&(ROB[j][3] == 1)&&(ROB[j][0] == 1))
						pipeline_objects[i].src1_ready = 1;
					else
						pipeline_objects[i].src1_ready = 0;
						
				}
				
				if(j == ROB_head_pointer)
					break;
				
				if(j == 0)
					j = ROB_size - 1;                                      //To deal with circular FIFO
				else
					j = j - 1;
			}
			else
				pipeline_objects[i].src1_ready = 1;
			
			
			
		/////////////////////////Asserting readiness of source 2/////////////////////////
			if(pipeline_objects[i].src2_RR == -1)
				pipeline_objects[i].src2_ready = 1;
			else if(pipeline_objects[i].src2_RR >= 1000)
			{
				int j = ROB_tail_pointer;
				while(1)
				{
					if((ROB[j][1] == pipeline_objects[i].src2_RR)&&(ROB[j][3] == 1)&&(ROB[j][0] == 1))
						pipeline_objects[i].src2_ready = 1;
					else
						pipeline_objects[i].src2_ready = 0;
						
				}
				
				if(j == ROB_head_pointer)
					break;
				
				if(j == 0)
					j = ROB_size - 1;                                      //To deal with circular FIFO
				else
					j = j - 1;
			}
			else
				pipeline_objects[i].src2_ready = 1;
			
			
		///////////////////////Assiging other signals//////////////////////////////////////////////
			pipeline_objects[i].PC_DI = pipeline_objects[i].PC_RR;
			pipeline_objects[i].no_clk_RNRR = pipeline_objects[i].no_clk_RN;
			pipeline_objects[i].no_clk_DERNRR = pipeline_objects[i].no_clk_DERN;
			pipeline_objects[i].entry_clk_FDERNRR = pipeline_objects[i].entry_clk_FDERN;
			pipeline_objects[i].src1_DI_OG = pipeline_objects[i].src1_RR_OG;
			pipeline_objects[i].src2_DI_OG = pipeline_objects[i].src2_RR_OG;
			pipeline_objects[i].dest_DI_OG = pipeline_objects[i].dest_RR_OG;
			pipeline_objects[i].src1_DI = pipeline_objects[i].src1_RR;
			pipeline_objects[i].src2_DI = pipeline_objects[i].src2_RR;
			pipeline_objects[i].dest_DI = pipeline_objects[i].dest_RR;
			pipeline_objects[i].op_type_DI = pipeline_objects[i].op_type_RR;
			
			
		}
		
	}
	else
	{
		DI_contains_new_bundle = 0;
		RR_can_accept_new_bundle = 0;
		
		for(int i = 0;i<WIDTH;i++)
			pipeline_objects[i].no_clk_RR += 1;
	}
}


void Dispatch()
{
	if((DI_contains_new_bundle)&&((IQ_size - IQ_entry_pointer) >= WIDTH))
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
		//IQ[entry][9] = NO_clk_RR
		//IQ[entry][10] = NO_clk_RR
		//IQ[entry][11] = NO_CLK_RN
		//IQ[entry][12] = NO_CLK_DE
		//IQ[entry][13] = ENTRY_FE
		//IQ[entry][14] = AGE
		//IQ{entry][15] = op_type
		//IQ{entry][16] = dest_OG
		for(int i = 0 ;i<WIDTH;i++)
		{
			IQ[IQ_entry_pointer][0] = 1;
			IQ[IQ_entry_pointer][1] = pipeline_objects[i].dest_DI;
			if(pipeline_objects[i].src1_ready == 1)
				IQ[IQ_entry_pointer][2] = 1;
			else
				IQ[IQ_entry_pointer][2] = 0;
			IQ[IQ_entry_pointer][3] = pipeline_objects[i].src1_DI;
			if(pipeline_objects[i].src2_ready == 1)
				IQ[IQ_entry_pointer][4] = 1;
			else
				IQ[IQ_entry_pointer][4] = 0;
			IQ[IQ_entry_pointer][5] = pipeline_objects[i].src2_DI;
			IQ[IQ_entry_pointer][6] = pipeline_objects[i].src1_DI_OG;
			IQ[IQ_entry_pointer][7] = pipeline_objects[i].src2_DI_OG;
			///////Update the number of cycles in iq in the issue stage
			IQ[IQ_entry_pointer][9] = pipeline_objects[i].no_clk_DI;
			IQ[IQ_entry_pointer][10] = pipeline_objects[i].no_clk_RR;
			IQ[IQ_entry_pointer][11] = pipeline_objects[i].no_clk_RNRR;
			IQ[IQ_entry_pointer][12] = pipeline_objects[i].no_clk_DERNRR;
			IQ[IQ_entry_pointer][13] = pipeline_objects[i].entry_clk_FDERNRR;
			IQ[IQ_entry_pointer][14] = youngest;
			IQ[IQ_entry_pointer][15] = pipeline_objects[i].op_type_DI;
			IQ[IQ_entry_pointer][15] = pipeline_objects[i].dest_RR_OG;
			
				
			////////////////Updating the pointers and counters//////////////////////
			IQ_entry_pointer += 1;
			youngest += 1;	
		}
	}
	else
	{
		DI_can_accept_new_bundle = 0;
		for(int i = 0;i<WIDTH;i++)
			pipeline_objects[i].no_clk_DI += 1;
	}
}


void Issue()
{
	int oldest =0;
	int number_of_issued_inst = 0;
	
	////////Updating cyles in issue queue///////
	for(int i=0;i<IQ_size;i++)
	{
		if(IQ[i][0] == 1)
			IQ[i][8] += 1;
	}
	///////////////////////////////////////////
	
	for(int i=0;i<IQ_size;i++)
	{
		if((number_of_issued_inst == WIDTH)||(execute_list_free_entry_pointer == (WIDTH*5 - 1)))
			break;
		else
		{
			for(int j=0;j<IQ_size;j++)
			{
				oldest = j;
				
				if((IQ[j][0] == 1) && (IQ[j][14] == oldest) && (IQ[j][2] == 1) && (IQ[j][4] == 1))
				{
					if((number_of_issued_inst == WIDTH)||(execute_list_free_entry_pointer == (WIDTH*5 - 1)))
						return;
					
					//IQ[entry][0] = Valid instructions
					//IQ[entry][1] = Dest Tag, regardless of if it exists
					//IQ[entry][2] = RS1 RDY
					//IQ[entry][3] = RS1 TAG
					//IQ[entry][4] = RS2 RDY
					//IQ[entry][5] = RS2 TAG
					//IQ[entry][6] = RS1 OG 
					//IQ[entry][7] = RS2 OG
					//IQ[entry][8] = NO_clock_IQ
					//IQ[entry][9] = NO_clk_RR
					//IQ[entry][10] = NO_clk_RR
					//IQ[entry][11] = NO_CLK_RN
					//IQ[entry][12] = NO_CLK_DE
					//IQ[entry][13] = ENTRY_FE
					//IQ[entry][14] = AGE
					//IQ{entry][15] = op_type
					//IQ{entry][16] = dest_OG
					
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
					
					
					//////////////Reducing age of instructions older than j//////////////
					for(int k=(j+1);k<(IQ_size);k++)                                                        
					IQ[k][1] = IQ[k][1] - 1;
				
				
					////////////Deleting the Issue Queue row/////////////////////
					for(int k =j;k<(IQ_size-1);k++)                                                         //Moving up the issue queue
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
					
					IQ[IQ_size-1][0] = 0;                                                                   //Invalidating last entry
					
					
					////////////////Updating the pointers and the counters//////////////////////
					execute_list_free_entry_pointer += 1;
					number_of_issued_inst += 1;
					IQ_entry_pointer = IQ_entry_pointer -1;
					youngest -= 1;
				}
			}
		}
	}
}



void Execute(){}
void Writeback(){}
void Retire(){}



int Advance_Cycle()
{
	ticker = ticker + 1;          //Updating the global clock
	//////////////////////////////////////////////////////////////
	
	if(temp_control_signal == 20)
		return 0;
	else
	{
		std::cout<<"\n inst count:"<<INST_FETCH_CNT<<"  retire count:"<<INST_RETIRE_CNT;
		std::cout<<"\n ROB_head:"<<ROB_head_pointer<<" ROB_tail:"<<ROB_tail_pointer;
		temp_control_signal += 1;
		return 1;
	}
}