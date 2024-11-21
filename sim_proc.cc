#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sim_proc.h"
#include<iostream>

////////////////////////////////////Defining global pipeline array////////////////////////////////////
pipeline* pipeline_objects;
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
	
	
	
	///////////////////////Assigning memory to the ROB array////////////////////////
	ROB = new unsigned long int*[params.rob_size];
	for(int i=0;i<params.rob_size;i++)
	{	                          
	ROB[i] = new unsigned long int[7];       //[][0] valid_dest; [][1] value(used as rob tag); [][2] dst; [][3] rdy; [][4] exc; [][5] mis; [][6] pc
                                              //Use 10001, 10002 etc as rob tag and when accesing just do index%1000
	}
	ROB_size = params.rob_size;
	
	////////////////////////////////////////////////////////////////////////////////
	
	///////////////////////Assigning memory to the Issue Queue///////////////////////
	IQ = new int*[params.iq_size];
	for(int i=0;i<params.iq_size;i++)
		IQ[i] = new int[7];                               //[][0] valid;[][1] age, [][2]dst tag, rs1 rdy, rs1 tag/value,rs2 rdy, rs2 tag/value 
	IQ_size = params.iq_size;

	////////////////////////////////////////////////////////////////////////////////	
	
	
	//////////////////////Initialising RMT/////////////////////////////////////////
	for(int i=0;i<67;i++)
		RMT_valid_array[i] = 0;                                 //Setting all valid bits to zero
	
	/////////////////////////////////////////////////////////////////////////////////
	
	
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
		Fetch();
	}while(Advance_Cycle(FP));
	
	//////////////////////////////////////////////////////////////////////////////////
    return 0;
}

//////////////////////////////RETIRE//////////////////////////////
void Retire()
{
	int inst_retired_this_cycle = 0;
}
/////////////////////////////////////////////////////////////////

/////////////////////////////FETCH//////////////////////////////
void Fetch()
{
	////For now I don't see the need for this function
}
/////////////////////////////////////////////////////////////////


void Rename()
{
	if((ROB_head_pointer == ROB_tail_pointer)&&(ROB_tail_phase == 0))        //ROB is empty
		ROB_tail_phase = 1;
	
	
	if(((ROB_head_pointer + 3) == ROB_tail_pointer) && (ROB_tail_phase == 1))     //ROB is full
		ROB_can_accpet_new_bundle = 0;
		
		
	if(rr_can_accept_new_bundle && ROB_can_accpet_new_bundle)
	{
		rename_can_accept_new_bundle = 1;
		
		for(int i=0;i<WIDTH;i++)
		{
				
			if(pipeline_objects[i].dest_decode_rename == -1)                  //The current Instruction does not have a destination register
			{
				ROB[ROB_tail_pointer][0] = 0;                       
			}
			else
			{
				ROB[ROB_tail_pointer][0] = 1;
			}
			
			ROB[ROB_tail_pointer][1] = 1000 + ROB_tail_pointer + ROB_tail_phase;
			ROB[ROB_tail_pointer][2] = pipeline_objects[i].dest_decode_rename;
			ROB[ROB_tail_pointer][3] = 0;                                          //Instruction ready flags
			ROB[ROB_tail_pointer][4] = 0;                                          //exc
			ROB[ROB_tail_pointer][5] = 0;                                          //mis
			ROB[ROB_tail_pointer][6] = pipeline_objects[i].PC;
			
			ROB_tail_pointer += 1;
			ROB_tail_pointer = ROB_tail_pointer%ROB_size;                         //Because it is a circular FIFO
			
			///////////Renaming the destination register in RMT///////////////////
			if(pipeline_objects[i].dest_decode_rename != -1)
			{
				RMT_tag[pipeline_objects[i].dest_decode_rename] = 1000 + ROB_tail_pointer + ROB_tail_phase;
				RMT_valid_array[pipeline_objects[i].dest_decode_rename] = 1;
			}
			else{
				RMT_tag[pipeline_objects[i].dest_decode_rename] = pipeline_objects[i].dest_decode_rename;
				RMT_valid_array[pipeline_objects[i].dest_decode_rename] = 0;
			}
			
			
			///////////Renaming the source registers///////////////////////////////
			if((pipeline_objects[i].src1_decode_rename != -1)&&(RMT_valid_array[pipeline_objects[i].src1_decode_rename]))
				pipeline_objects[i].src1_decode_rename = RMT_tag[pipeline_objects[i].src1_decode_rename];
			
			if((pipeline_objects[i].src2_decode_rename != -1)&&(RMT_valid_array[pipeline_objects[i].src2_decode_rename]))
				pipeline_objects[i].src2_decode_rename = RMT_tag[pipeline_objects[i].src2_decode_rename];
					
			
		}
		
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

///////////////////////////Advance_Cycle/////////////////////////
int Advance_Cycle(FILE *FP)
{
	/////////////////////////Decode-Rename////////////////////////////////
	if(rename_can_accept_new_bundle)
	{
		for(int i=0;i<WIDTH;i++)
		{
			if(pipeline_objects[i].src1_fetch_decode == -2)
				break;
			pipeline_objects[i].src1_decode_rename = pipeline_objects[i].src1_fetch_decode;
			pipeline_objects[i].src2_decode_rename = pipeline_objects[i].src2_fetch_decode;
			pipeline_objects[i].dest_decode_rename = pipeline_objects[i].dest_fetch_decode;
			pipeline_objects[i].op_type_decode_rename = pipeline_objects[i].op_type_fetch_decode;
			
		}
		decode_can_accept_new_bundle = 1;
	}
	else
	{
		for(int i=0;i<WIDTH;i++)
			pipeline_objects[i].no_clk_decode += 1;
		decode_can_accept_new_bundle = 0;
	}
	
	
	/////////////////////////Fetch - Decode///////////////////////////////
	int op_type, dest, src1,src2;
	unsigned int pc;
	
	if(decode_can_accept_new_bundle){                                              //Think about the case when decode has to accept fewer than WIDTH instructions 
	for(int i =0; i<WIDTH;i++)
	{
	if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
		{
		pipeline_objects[i].src1_fetch_decode = src1;
		pipeline_objects[i].src2_fetch_decode = src2;
		pipeline_objects[i].dest_fetch_decode = dest;
		pipeline_objects[i].op_type_fetch_decode = op_type;
		pipeline_objects[i].PC = pc;
		}
	else                                                              //When we have fetch less than width functions
		{
		for(int j=i;j<WIDTH;j++)
		{
			pipeline_objects[j].src1_fetch_decode = -2;
			pipeline_objects[j].src2_fetch_decode = -2;
			pipeline_objects[j].dest_fetch_decode = -2;
			pipeline_objects[j].op_type_fetch_decode = -2;
			pipeline_objects[j].PC = -2;
		}
		break;
		}
	}
	}
	//////////////////////////////////////////////////////////////
	
	return 1;
}

/////////////////////////////////////////////////////////////////////

/////////////////////////////////Decode/////////////////////////////
void Decode()
{
	
}
////////////////////////////////////////////////////////////////////


void Writeback() {}

void Execute() {}

void Issue() {}

void Dispatch() {}

void RegRead() {}