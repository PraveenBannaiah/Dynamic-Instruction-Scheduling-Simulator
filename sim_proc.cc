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


unsigned seq_no = 0;                              //Because I am too lazy to actually count it and the inst in ROB retired based on program order

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
	ROB[i] = new long long int[20];       //[][0] valid_dest; [][1] value(used as rob tag); [][2] dst; [][3] rdy; [][4] exc; [][5] mis; [][6] pc, [][7]src_og_1, [][8]src2_og,[][9]op_type,[][10]:valid inst;
                                              //Use 10001, 10002 etc as rob tag and when accesing just do index%1000
	}
	ROB_size = params.rob_size;
	ROB_can_accpet_new_bundle = ROB_size;     //For checking rob full conditions
	////////////////////////////////////////////////////////////////////////////////
	
	///////////////////////Assigning memory to the Issue Queue///////////////////////
	IQ = new int*[params.iq_size];
	for(int i=0;i<params.iq_size;i++)
		IQ[i] = new int[20];                               //[][0] valid;[][1] age, [][2]dst tag, rs1 rdy, rs1 tag/value,rs2 rdy, rs2 tag/value , src1_og, src2_og, dest_og, op_type, no_cycles_in_iq
	IQ_size = params.iq_size;                               // 12:no_clk_dispatch,13:no_clk_rrd,14:no_clk_rerd,15:no_clk_drerd,16:entry_clk_fdrerd 17:rob_tag;

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
		execute_list[j] = new int[20];                   //0:src1,1:src2,2:dest,3:op_type,4:no_cyles_in_exe,5:completed?,6:src1_og,7:src2_og,8:src3_og,9:valid
														 //10: no_clk_iq,11:no_clk_dispatch,12:no_clk_rrd,13:no_clk_rerd,14:no_clk_drerd,15:entry_clk_fdrerd,16:rob_tag
														 
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
		Fetch();
	}while(Advance_Cycle(FP));
	
	//////////////////////////////////////////////////////////////////////////////////
    return 0;
}



/////////////////////////////FETCH//////////////////////////////
void Fetch()
{
	////For now I don't see the need for this function
	std::cout<<"\n In fetch stage";
}
/////////////////////////////////////////////////////////////////


void Rename()
{
	std::cout<<"\n In rename stage";
	
	
	if((ROB_head_pointer == ROB_tail_pointer)&&(ROB_tail_phase == 0))        //ROB is empty
		ROB_tail_phase = 1;
	
	
	//if(((ROB_head_pointer + 3) == ROB_tail_pointer) && (ROB_tail_phase == 1))     //ROB is full
	//	ROB_can_accpet_new_bundle = 0;
		
		
	if(rr_can_accept_new_bundle && (ROB_can_accpet_new_bundle>=WIDTH))
	{
		rename_can_accept_new_bundle = 1;
		
		for(int i=0;i<WIDTH;i++)
		{
			//std::cout<<"\n test"<<pipeline_objects[i].src1_decode_rename<<" " <<pipeline_objects[i].src2_decode_rename;
			if((pipeline_objects[i].src1_decode_rename == -2) || (pipeline_objects[i].src2_decode_rename == -2))
			{
				//printf("\nEmpty pipeline");
				break;                                                                                      //For initial case when there are no instructions in the pipeline
			}
			
			
			///////////Renaming the source registers///////////////////////////////                              //we are renaming first becuase of test case r3 == r3 + r1;
			if((pipeline_objects[i].src1_decode_rename != -1)&&(RMT_valid_array[pipeline_objects[i].src1_decode_rename]))
			{
				
				pipeline_objects[i].temp_src1_rename_rr = RMT_tag[pipeline_objects[i].src1_decode_rename];
				
			}
			else
			{
				pipeline_objects[i].temp_src1_rename_rr = pipeline_objects[i].src1_decode_rename;
			}
			
			if((pipeline_objects[i].src2_decode_rename != -1)&&(RMT_valid_array[pipeline_objects[i].src2_decode_rename]))
				pipeline_objects[i].temp_src2_rename_rr = RMT_tag[pipeline_objects[i].src2_decode_rename];
			else
				pipeline_objects[i].temp_src2_rename_rr = pipeline_objects[i].src2_decode_rename;
			/////////////////////////////////////////////////////////////////////////
				
			if(pipeline_objects[i].dest_decode_rename == -1)                  //The current Instruction does not have a destination register
			{
				ROB[ROB_tail_pointer][0] = 0;                       
			}
			else
			{
				ROB[ROB_tail_pointer][0] = 1;
			}
			
			ROB_can_accpet_new_bundle--;                                     //Reflects the number of free entries in the ROb
			
			
			ROB[ROB_tail_pointer][1] = ROB_tag_start;
			pipeline_objects[i].ROB_tag_for_this_inst = ROB[ROB_tail_pointer][1];
			
			///////////Renaming the destination register in RMT///////////////////                   //This block is here because in earlier logic the tail_pointer is updated before assigning 
			if(pipeline_objects[i].dest_decode_rename != -1)
			{
				RMT_tag[pipeline_objects[i].dest_decode_rename] = ROB_tag_start;
				RMT_valid_array[pipeline_objects[i].dest_decode_rename] = 1;
				pipeline_objects[i].temp_dest_rename_rr = ROB_tag_start;
			}
			else{                                                                                             //For branch instruction
				RMT_tag[pipeline_objects[i].dest_decode_rename] = pipeline_objects[i].dest_decode_rename;
				RMT_valid_array[pipeline_objects[i].dest_decode_rename] = 0;
				pipeline_objects[i].temp_dest_rename_rr = pipeline_objects[i].dest_decode_rename;
			}
			
			ROB_tag_start += 1;
			
			///////////////////////////////////////////////////////////////////////
			if(pipeline_objects[i].dest_decode_rename == -1)
				ROB[ROB_tail_pointer][2] = 444;                                         //Can't use -1 because declared as unsigned 
			else
				ROB[ROB_tail_pointer][2] = pipeline_objects[i].dest_decode_rename;
			ROB[ROB_tail_pointer][3] = 0;                                          //Instruction ready flags
			ROB[ROB_tail_pointer][4] = 0;                                          //exc
			ROB[ROB_tail_pointer][5] = 0;                                          //mis
			ROB[ROB_tail_pointer][6] = pipeline_objects[i].PC_decode_rename;
			ROB[ROB_tail_pointer][10] = 1;                                         //THis is valid instruction, needed when the pipeline is empty
			ROB[ROB_tail_pointer][7] = pipeline_objects[i].src1_og_decode_rename;
			ROB[ROB_tail_pointer][8] = pipeline_objects[i].src2_og_decode_rename;
			ROB[ROB_tail_pointer][9] = pipeline_objects[i].op_type_decode_rename;
			
			ROB_tail_pointer += 1;
			ROB_tail_pointer = ROB_tail_pointer%ROB_size;                         //Because it is a circular FIFO
			
		}
		
	}
	
	else
		rename_can_accept_new_bundle = 0;
	
	
	/*std::cout<<"\n Printing RMT Contents";
	for(int i=0;i<67;i++)
	{
		if(RMT_valid_array[i] == 1)                                                         //Only printing entries with valid rob tag
		std::cout<<"\nValid:"<<RMT_valid_array[i]<<" Reg:R"<<RMT_tag[i]<<" Og reg:R"<<i;
	}
	
	std::cout<<"\n Printing ROB Contents";
	for(int i=0;i<ROB_size;i++)
		std::cout<<"\n Valid:"<<ROB[i][0]<<" robTag:"<<ROB[i][1]<<" dst:R"<<ROB[i][2]<<" rdy:"<<ROB[i][3]<<" exc:"<<ROB[i][4]<<" mis:"<<ROB[i][5]<<" pc:"<<ROB[i][6];
	*/
}

///////////////////////////Advance_Cycle/////////////////////////
int Advance_Cycle(FILE *FP)
{
	std::cout<<"\n Advance cycle";
	
	
	/////////////////////////RegRead-Dispatch////////////////////////////
	if(dispatch_can_accept_new_bundle)
	{
		//printf("\n Dispatch can accept a new bundle");
		for(int i=0;i<WIDTH;i++)
		{
			pipeline_objects[i].src1_rr_dispatch = pipeline_objects[i].src1_rename_rr;
			pipeline_objects[i].src2_rr_dispatch = pipeline_objects[i].src2_rename_rr;
			pipeline_objects[i].dest_rr_dispatch = pipeline_objects[i].dest_rename_rr;
			pipeline_objects[i].src1_og_rr_dispatch = pipeline_objects[i].src1_og_rename_rr;
			pipeline_objects[i].src2_og_rr_dispatch = pipeline_objects[i].src2_og_rename_rr;
			pipeline_objects[i].dest_og_rr_dispatch = pipeline_objects[i].dest_og_rename_rr;
			
			pipeline_objects[i].op_type_rr_dispatch = pipeline_objects[i].op_type_rename_rr;
			
			pipeline_objects[i].PC_rr_dispatch = pipeline_objects[i].PC_rename_rr;
			
			std::cout<<"\n DISPATCH lineNo:"<<i<<" op_type:"<<pipeline_objects[i].op_type_rr_dispatch<< " src1:"<< pipeline_objects[i].src1_rr_dispatch;
			std::cout<<" src2:"<<pipeline_objects[i].src2_rr_dispatch<<" dest:"<<pipeline_objects[i].dest_rr_dispatch;
			
			////////////////////TIMING//////////////////////////////
			pipeline_objects[i].no_clk_rer = pipeline_objects[i].no_clk_rename;
			pipeline_objects[i].no_clk_drer = pipeline_objects[i].no_clk_dr;
			pipeline_objects[i].entry_clk_fdrer = pipeline_objects[i].entry_clk_fdr;
			
		}
	}
	else
		//printf("\n Dispatch can not accept a new bundle");
	
	/////////////////////////Rename-RegRead///////////////////////////////
	std::cout<<"\nrr_can_accept_new_bundle: "<<rr_can_accept_new_bundle;
	if(rr_can_accept_new_bundle)
	{
		//printf("\n RR can accept a new bundle");
		
		for(int i=0;i<WIDTH;i++)
		{
			pipeline_objects[i].src1_og_rename_rr = pipeline_objects[i].src1_og_decode_rename;
			pipeline_objects[i].src2_og_rename_rr = pipeline_objects[i].src2_og_decode_rename;
			pipeline_objects[i].dest_og_rename_rr = pipeline_objects[i].dest_og_decode_rename;
			
			pipeline_objects[i].dest_rename_rr = pipeline_objects[i].temp_dest_rename_rr;
			pipeline_objects[i].src1_rename_rr = pipeline_objects[i].temp_src1_rename_rr;
			pipeline_objects[i].src2_rename_rr = pipeline_objects[i].temp_src2_rename_rr;
			
			
			//pipeline_objects[i].dest_rename_rr = pipeline_objects[i].dest_decode_rename;
			pipeline_objects[i].op_type_rename_rr = pipeline_objects[i].op_type_decode_rename;
			
			pipeline_objects[i].PC_rename_rr = pipeline_objects[i].PC_decode_rename;
			
			std::cout<<"\n REGREAD lineNo:"<<i<<" op_type:"<<pipeline_objects[i].op_type_rename_rr<< "src1:"<<pipeline_objects[i].src1_rename_rr;
			std::cout<<"\n src2:"<<pipeline_objects[i].src2_rename_rr<<" dest:"<<pipeline_objects[i].dest_rename_rr;
			
			
			
			/////////////////TIMING////////////////////
			pipeline_objects[i].no_clk_dr = pipeline_objects[i].no_clk_decode;
			pipeline_objects[i].entry_clk_fdr = pipeline_objects[i].entry_clk_fd;
			pipeline_objects[i].no_clk_rename = 1;
		}
	}
	else
	{
		//printf("\n RR can not accept a new bundle");
		
		for(int i=0;i<WIDTH;i++)
		{
			pipeline_objects[i].no_clk_rename += 1;
		}
	}
	
	/////////////////////////////////////////////////////////////////////////
	
	
		
	/////////////////////////Decode-Rename////////////////////////////////
	if(rename_can_accept_new_bundle)
	{
		//printf("\n Rename can accept a new bundle");
		
		for(int i=0;i<WIDTH;i++)
		{
			if(pipeline_objects[i].src1_fetch_decode == -2)
				continue;
			pipeline_objects[i].src1_decode_rename = pipeline_objects[i].src1_fetch_decode;
			pipeline_objects[i].src2_decode_rename = pipeline_objects[i].src2_fetch_decode;
			pipeline_objects[i].dest_decode_rename = pipeline_objects[i].dest_fetch_decode;
			pipeline_objects[i].op_type_decode_rename = pipeline_objects[i].op_type_fetch_decode;
			
			pipeline_objects[i].src1_og_decode_rename = pipeline_objects[i].src1_fetch_decode;                    //Storing the original reg names
			pipeline_objects[i].src2_og_decode_rename = pipeline_objects[i].src2_fetch_decode;
			pipeline_objects[i].dest_og_decode_rename = pipeline_objects[i].dest_fetch_decode;
			
			pipeline_objects[i].PC_decode_rename = pipeline_objects[i].PC_fetch_decode;
			
			//////////////TIMING///////////////
			pipeline_objects[i].entry_clk_fd = pipeline_objects[i].entry_clk_fetch;
			pipeline_objects[i].no_clk_decode = 1;
			
			
		}
		decode_can_accept_new_bundle = 1;
	}
	else
	{
		//printf("\n Rename can not accept a new bundle");
		
		for(int i=0;i<WIDTH;i++)
			pipeline_objects[i].no_clk_decode += 1;
		
		decode_can_accept_new_bundle = 0;
	}
	
	
	/////////////////////////Fetch - Decode///////////////////////////////
	int op_type, dest, src1,src2;
	unsigned int pc;
	
	if(decode_can_accept_new_bundle){                                              //Think about the case when decode has to accept fewer than WIDTH instructions

		//printf("\n Instruction fetched in a given cycle\n");
	
	if(EOF_reached)
		{
			for(int i = 0;i<WIDTH;i++)
			{
			pipeline_objects[i].src1_fetch_decode = -2;
			pipeline_objects[i].src2_fetch_decode = -2;
			pipeline_objects[i].dest_fetch_decode = -2;
			pipeline_objects[i].op_type_fetch_decode = -2;
			pipeline_objects[i].PC_fetch_decode = -2;
			}
		}	
	
	else{
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
	}
	
	ticker = ticker + 1;          //Updating the global clock
	//////////////////////////////////////////////////////////////
	
	if(temp_control_signal == 1155)
		return 0;
	else
	{
		std::cout<<"\n inst count:"<<INST_FETCH_CNT<<"  retire count:"<<INST_RETIRE_CNT;
		std::cout<<"\n ROB_head:"<<ROB_head_pointer<<" ROB_tail:"<<ROB_tail_pointer;
		temp_control_signal += 1;
		return 1;
	}
	
	if(INST_FETCH_CNT == INST_RETIRE_CNT)
	{
		std::cout<<"\n inst count:"<<INST_FETCH_CNT<<"  retire count:"<<INST_RETIRE_CNT;
		return 0;
	}
	else
	{
		std::cout<<"\n inst count:"<<INST_FETCH_CNT<<"  retire count:"<<INST_RETIRE_CNT;
		return 1;
	}
	

	
}

/////////////////////////////////////////////////////////////////////

/////////////////////////////////Decode/////////////////////////////
void Decode()
{
	//std::cout<<"\n In decode stage";
}
////////////////////////////////////////////////////////////////////

//////////////////////////////RETIRE//////////////////////////////
void Retire()
{
	std::cout<<"\n\n\n In retire stage";
	
	int inst_retired_this_cycle = 0;
	//std::cout<<"\n Printing ROB Contents";
	for(int i=0;i<ROB_size;i++)
	{
		std::cout<<"\n Valid:"<<ROB[i][0]<<" robTag:"<<ROB[i][1]<<" dst:R"<<ROB[i][2]<<" rdy:"<<ROB[i][3]<<" exc:"<<ROB[i][4]<<" mis:"<<ROB[i][5]<<" pc:"<<ROB[i][6];
		std::cout<<" other info:";
		for(int j=7;j<17;j++)
			std::cout<<" "<<ROB[i][j];
	}
	
	std::cout<<"\n ROB_head_pointer:"<<ROB_head_pointer;
	
	//////////updating cycles spent in retire stage////////////
	//ROB[][18] represent the cycles 
	for(int i = 0 ; i<ROB_size; i++)
	{
		if(ROB[i][3] == 1)
			ROB[i][18] += 1;
	}
	
	for(int i = ROB_head_pointer ; i<ROB_size; i++)
	{
		//if(ROB[i][10] == 0)                                                       //Not yet assiging valid bits
		//	continue;

		if(inst_retired_this_cycle == WIDTH)
			break;
		
		//std::cout<<"\n ROB[ROB_head_pointer][3]:"<<ROB[ROB_head_pointer][3];
		if(ROB[ROB_head_pointer][3] == 1)
		{
			if(ROB[ROB_head_pointer][2] == 444)                                    //Checking for branch instructions
			{}
			
			else if(RMT_tag[ROB[ROB_head_pointer][2]] == ROB[ROB_head_pointer][1])      //Checking if we have to update the RMT
			{
				RMT_valid_array[ROB[ROB_head_pointer][2]] = 0;                     //Invalidating that particular entry
			}
			
			ROB_can_accpet_new_bundle++;                                           //Reflects the number of free entries in the ROB
			
			int temp_cycles;
			if(ROB[ROB_head_pointer][9] == 0)
				temp_cycles = 1;
			else if(ROB[ROB_head_pointer][9] == 1)
				temp_cycles = 2;
			else
				temp_cycles = 5;
			
			
			int temp_dest;
			if(ROB[ROB_head_pointer][2] == 444)
				temp_dest = -1;
			else
				temp_dest = ROB[ROB_head_pointer][2];
			//[][0] valid_dest; [][1] value(used as rob tag); [][2] dst; [][3] rdy; [][4] exc; [][5] mis; [][6] pc, [][7]src_og_1, [][8]src2_og,[][9]op_type,[][10]:valid inst;
			// 11:no_clk_iq,12:no_clk_dispatch,13:no_clk_rrd,14:no_clk_rerd,15:no_clk_drerd,16:entry_clk_fdrerd 17:rob_tag;
			std::cout<<"\n"<<seq_no<<"  fu{"<<ROB[ROB_head_pointer][9]<<"}"<<"  src{"<<ROB[ROB_head_pointer][7]<<","<<ROB[ROB_head_pointer][8]<<"}";
			std::cout<<" dst{"<<temp_dest<<"}"<<" FE{"<<ROB[ROB_head_pointer][16]<<",1}"<<" DE{"<<ROB[ROB_head_pointer][16]+1<<","<<ROB[ROB_head_pointer][15]<<"}";
			std::cout<<" RN{"<<ROB[ROB_head_pointer][15] + ROB[ROB_head_pointer][16] +1<<","<<ROB[ROB_head_pointer][14]<<"} RR{"<<ROB[ROB_head_pointer][15] + ROB[ROB_head_pointer][16] +1 + ROB[ROB_head_pointer][14] <<","<<ROB[ROB_head_pointer][13]<<"}";
			std::cout<<" DI{"<<ROB[ROB_head_pointer][15] + ROB[ROB_head_pointer][16] +1 + ROB[ROB_head_pointer][14] + ROB[ROB_head_pointer][13]<<","<<ROB[ROB_head_pointer][12]<<"} IS{"<<ROB[ROB_head_pointer][15] + ROB[ROB_head_pointer][16] +1 + ROB[ROB_head_pointer][14] + ROB[ROB_head_pointer][13] + ROB[ROB_head_pointer][12]<<","<<ROB[ROB_head_pointer][11]<<"}";
			std::cout<<" EX{"<<ROB[ROB_head_pointer][15] + ROB[ROB_head_pointer][16] +1 + ROB[ROB_head_pointer][14] + ROB[ROB_head_pointer][13] + ROB[ROB_head_pointer][12] + ROB[ROB_head_pointer][11]<<","<<temp_cycles<<"}";
			std::cout<<" WB{"<<ROB[ROB_head_pointer][15] + ROB[ROB_head_pointer][16] +1 + ROB[ROB_head_pointer][14] + ROB[ROB_head_pointer][13] + ROB[ROB_head_pointer][12] + ROB[ROB_head_pointer][11] + temp_cycles<<",1}";
			std::cout<<" RT{"<<ROB[ROB_head_pointer][15] + ROB[ROB_head_pointer][16] +1 + ROB[ROB_head_pointer][14] + ROB[ROB_head_pointer][13] + ROB[ROB_head_pointer][12] + ROB[ROB_head_pointer][11] + temp_cycles+1<<","<<ROB[ROB_head_pointer][18]<<"}";
			
			ROB_head_pointer += 1;
			ROB_head_pointer = ROB_head_pointer%ROB_size;
			INST_RETIRE_CNT += 1;
			inst_retired_this_cycle += 1;
			seq_no += 1;
			
			/*for(int j=0;j<writeback_free_entry_pointer;j++)                       //Idk why i wrote this but then also it shout be be before updating the head pointer
			{
				if(WriteBack_buffer[j][2] == ROB[ROB_head_pointer][2])
				{
					std::cout<<"\n"<<"RETIRED  "<<"fu{"<<"}"<<"  src{";
				}
				
			}*/
		}
	}
}
/////////////////////////////////////////////////////////////////

void Writeback() {
	
	//std::cout<<"\n In Writeback stage";
	
	////I don't think we need to hold the values in write back buffer, after setting the ready bit in ROB we can let it over write ig
	for(int i =0 ;i<=writeback_free_entry_pointer;i++)
	{
		for(int j=0;j<ROB_size;j++)
		{
			
			//std::cout<<"\n WritebackBuffer: "<<WriteBack_buffer[i][16]<<"  ROB:"<<ROB[j][1]<<"\n";
			if((WriteBack_buffer[i][16] == ROB[j][1])&&(ROB[j][10] == 1))
			{
				ROB[j][3] = 1;                                                    //Setting ready bit in ROB
				
				//Writing the data into the ROB to print it out
				// 11:no_cycles_iq,12:no_clk_dispatch,13:no_clk_rrd,14:no_clk_rerd,15:no_clk_drerd,16:entry_clk_fdrerd 17:rob_tag;
				ROB[j][11] = WriteBack_buffer[i][10];
				ROB[j][12] = WriteBack_buffer[i][11];
				ROB[j][13] = WriteBack_buffer[i][12];
				ROB[j][14] = WriteBack_buffer[i][13];
				ROB[j][15] = WriteBack_buffer[i][14];
				ROB[j][16] = WriteBack_buffer[i][15];
				
			}
				
		}
	}
	
	writeback_free_entry_pointer = 0;  //Since we don't have a limit on the number of instructions written back in each cycle we get an empty writback buffer each cycle

}

void Execute() {
	
	//std::cout<<"\n In Execute stage";
	wakeup_pointer = 0;
	
	///////////////////////Checking for completeness//////////////////
	for(int i =0;i<execute_list_free_entry_pointer;i++)
	{
		if(execute_list[i][3] == 0)                               //1 cycle instruction 
		{
			if(execute_list[i][4] == 1)
				execute_list[i][5] = 1;                           //set the complete flag high
			else
				execute_list[i][5] = 0;
			
		}
		else if(execute_list[i][3] == 1)                           //2 cycle instruction 
		{
			if(execute_list[i][4] == 2)
				execute_list[i][5] = 1;
			else
				execute_list[i][5] = 0;			
			
		}
		else if(execute_list[i][3] == 2)                           //5 cycle instruction
		{
			if(execute_list[i][4] == 5)
				execute_list[i][5] = 1; 
			else
				execute_list[i][5] = 0;
			
		}
	}
	
	
	//////////////////////////Forward Bypassing////////////////////////////////////
	for(int i =0;i<execute_list_free_entry_pointer;i++)
	{
		if(((execute_list[i][3] == 0)&&(execute_list[i][4] == 0))||((execute_list[i][3] == 1)&&(execute_list[i][4] == 1))||((execute_list[i][3] == 2)&&(execute_list[i][4] == 4)))                              
		{
			for(int j=0;j<IQ_size;j++)                       //Forward Bypassing for Issue Queue
			{
				//execute
				//0:src1,1:src2,2:dest,3:op_type,4:no_cyles_in_exe,5:completed?,6:src1_og,7:src2_og,8:src3_og,9:valid
				//10: no_clk_iq,no_clk_in_rr,no_clk_in_rename_rr,no_clk_in_decode_rr,no_clk_in_fetch_rr;
				
				//Issue
				//[][0] valid;[][1] age, [][2]dst tag, rs1 rdy, rs1 tag/value,rs2 rdy, rs2 tag/value ,
				//std::cout<<"\n MISSING4 i:"<<i<<" ex 0:"<<execute_list[i][0]<<" ex 1:"<<execute_list[i][1]<<" j:"<<j<<" IQ[4]:"<<IQ[j][4]<<" IQ[6]:"<<IQ[j][6];
				if(execute_list[i][2] == IQ[j][4])
				{
					IQ[j][3] = 1;
				}
				
				if(execute_list[i][2] == IQ[j][6])
				{
					IQ[j][5] = 1;
				}
				/*if(execute_list[i][1] == IQ[j][4])
				{
					IQ[j][3] = 1;
				}
				
				if(execute_list[i][1] == IQ[j][6])
				{
					IQ[j][5] = 1;
				}
				*/
			}
			
			
			Wakeup[wakeup_pointer] = execute_list[i][16];
			wakeup_pointer += 1;
			
			////////Forward biasing for regread//////
			for(int j=0;j<WIDTH;j++)
			{
				if((execute_list[i][16] == pipeline_objects[j].src1_rename_rr) || (execute_list[i][16] == pipeline_objects[j].src1_rename_rr))
					pipeline_objects[j].src1_ready_re = 1;
				
				if((execute_list[i][16] == pipeline_objects[j].src2_rename_rr) || (execute_list[i][16] == pipeline_objects[j].src2_rename_rr))
					pipeline_objects[j].src2_ready_re = 1;
			}
			
			
			/////Forward bypassing to dispatch////////
			for(int j=0;j<WIDTH;j++)
			{
				if((execute_list[i][16] == pipeline_objects[j].src1_rr_dispatch) || (execute_list[i][16] == pipeline_objects[j].src1_rr_dispatch))
					pipeline_objects[j].src1_ready = 1;
				
				if((execute_list[i][16] == pipeline_objects[j].src2_rr_dispatch) || (execute_list[i][16] == pipeline_objects[j].src2_rr_dispatch))
					pipeline_objects[j].src2_ready = 1;
			}
			
			/////////////////////////////////////////
		}
	}
	/////////////////////////////////////////////////////////////////
	
	
	///////////////////////for writeback/////////////////////////////
	for(int i =0;i<execute_list_free_entry_pointer;i++)                  //All completed instructions are put into the write back buffer
	{
		if(execute_list[i][5] == 1)                                      //Filtering complete instructions
		{
			//0:src1,1:src2,2:dest,3:op_type,4:no_cyles_in_exe,5:completed?,6:src1_og,7:src2_og,8:src3_og,9:valid
				//10: no_clk_iq,11:no_clk_dispatch,12:no_clk_rrd,13:no_clk_rerd,14:no_clk_drerd,15:entry_clk_fdrerd,16:rob_tag
			/*std::cout<<"\n xecute list pointer:"<<execute_list_free_entry_pointer;
			std::cout<<"\n writeback_pointer:"<<writeback_free_entry_pointer;
			std::cout<<"\nadfgad";*/
			
			std::cout<<"\n WRITING:"<<execute_list[i][16];
			WriteBack_buffer[writeback_free_entry_pointer][0] = execute_list[i][0];
			WriteBack_buffer[writeback_free_entry_pointer][1] = execute_list[i][1];
			WriteBack_buffer[writeback_free_entry_pointer][2] = execute_list[i][2];
			WriteBack_buffer[writeback_free_entry_pointer][3] = execute_list[i][3];
			WriteBack_buffer[writeback_free_entry_pointer][4] = execute_list[i][4];
			WriteBack_buffer[writeback_free_entry_pointer][5] = execute_list[i][5];
			WriteBack_buffer[writeback_free_entry_pointer][6] = execute_list[i][6];
			WriteBack_buffer[writeback_free_entry_pointer][7] = execute_list[i][7];
			WriteBack_buffer[writeback_free_entry_pointer][8] = execute_list[i][8];
			WriteBack_buffer[writeback_free_entry_pointer][9] = execute_list[i][9];
			WriteBack_buffer[writeback_free_entry_pointer][10] = execute_list[i][10];
			WriteBack_buffer[writeback_free_entry_pointer][11] = execute_list[i][11];
			WriteBack_buffer[writeback_free_entry_pointer][12] = execute_list[i][12];
			WriteBack_buffer[writeback_free_entry_pointer][13] = execute_list[i][13];
			WriteBack_buffer[writeback_free_entry_pointer][14] = execute_list[i][14];
			WriteBack_buffer[writeback_free_entry_pointer][15] = execute_list[i][15];
			WriteBack_buffer[writeback_free_entry_pointer][16] = execute_list[i][16];
			
			writeback_free_entry_pointer += 1;
		}
	}


	
	///////////////////////////////////////////////////////////////////////
	

	////////////////////////////Clearing the execute list//////////////////////
	int i = 0;
	while(i<execute_list_free_entry_pointer)
	{
		if(execute_list[i][5] == 1)
		{
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
			
			/*execute_list[execute_list_free_entry_pointer - 2][0] = execute_list[execute_list_free_entry_pointer - 1][0];
			execute_list[execute_list_free_entry_pointer - 2][1] = execute_list[execute_list_free_entry_pointer - 1][1];
			execute_list[execute_list_free_entry_pointer - 2][2] = execute_list[execute_list_free_entry_pointer - 1][2];
			execute_list[execute_list_free_entry_pointer - 2][3] = execute_list[execute_list_free_entry_pointer - 1][3];
			execute_list[execute_list_free_entry_pointer - 2][4] = execute_list[execute_list_free_entry_pointer - 1][4];
			execute_list[execute_list_free_entry_pointer - 2][5] = execute_list[execute_list_free_entry_pointer - 1][5];
			execute_list[execute_list_free_entry_pointer - 2][6] = execute_list[execute_list_free_entry_pointer - 1][6];
			execute_list[execute_list_free_entry_pointer - 2][7] = execute_list[execute_list_free_entry_pointer - 1][7];
			execute_list[execute_list_free_entry_pointer - 2][8] = execute_list[execute_list_free_entry_pointer - 1][8];
			execute_list[execute_list_free_entry_pointer - 2][9] = execute_list[execute_list_free_entry_pointer - 1][9];
			execute_list[execute_list_free_entry_pointer - 2][10] = execute_list[execute_list_free_entry_pointer - 1][10];
			execute_list[execute_list_free_entry_pointer - 2][11] = execute_list[execute_list_free_entry_pointer - 1][11];
			execute_list[execute_list_free_entry_pointer - 2][12] = execute_list[execute_list_free_entry_pointer - 1][12];
			execute_list[execute_list_free_entry_pointer - 2][13] = execute_list[execute_list_free_entry_pointer - 1][13];
			execute_list[execute_list_free_entry_pointer - 2][14] = execute_list[execute_list_free_entry_pointer - 1][14];
			execute_list[execute_list_free_entry_pointer - 2][15] = execute_list[execute_list_free_entry_pointer - 1][15];
			execute_list[execute_list_free_entry_pointer - 2][16] = execute_list[execute_list_free_entry_pointer - 1][16];*/
			
			execute_list_free_entry_pointer -= 1;
			
			                    
		}
		else
			i++;
	}
	
	
	/////////////////////////////////Updating the cycles//////////////////////////////////////
	for(int i =0;i<execute_list_free_entry_pointer;i++)
		execute_list[i][4] += 1;
	
	/////////////////////////////////////////////////////////////////////////////////////////
	
	
	
	//std::cout<<"\n Printing the execute list in execute stage\n";
	//0:src1,1:src2,2:dest,3:op_type,4:no_cyles_in_exe,5:completed?,
	//for(int i=0;i<execute_list_free_entry_pointer;i++)
	//	std::cout<<"\n src1:"<<execute_list[i][0]<<" src2:"<<execute_list[i][1]<<" dest:"<<execute_list[i][2]<<" op_type:"<<execute_list[i][3]<<" no_cycles:"<<execute_list[i][4]<<" completed?:"<<execute_list[i][5];
	//	std::cout<<" ex16:"<<execute_list[i][16];
	
}
//////////////////////////////////////////////////////////////////////////////////////////////
void Issue() {
	
	
	//std::cout<<"\n In Issue stage";
	
	
	
	int oldest =0;
	int number_of_issued_inst = 0;
	
	////////counting cyles in issue queue///////
	for(int i=0;i<IQ_size;i++)
	{
		if(IQ[i][0] == 1)
			IQ[i][11] += 1;
		
	}
	///////////////////////////////////////////
	
	
	execute_list_has_space = 1;                                                                //Initally assume we have space
	
	for(int k=0;k<IQ_size;k++){                                                                //Trying to isse upto WIDTH instruction
		if(number_of_issued_inst > WIDTH)
			break;
		if(execute_list_free_entry_pointer == (WIDTH*5))
		{
			execute_list_has_space = 0;
			break;
		}
		
		for(int j=0;j<IQ_size;j++)
		{
			oldest = j;
			if((IQ[j][0] == 1)&&(IQ[j][1] == oldest) && (IQ[j][3]) && (IQ[j][5]))                  //THen issue it to the execute list
			{
				//0:src1,1:src2,2:dest,3:op_type,4:no_cyles_in_exe,5:completed?,6:src1_og,7:src2_og,8:src3_og,9:valid
				//10: no_clk_iq,11:no_clk_dispatch,12:no_clk_rrd,13:no_clk_rerd,14:no_clk_drerd,15:entry_clk_fdrerd,16:rob_tag
				
				// 12:no_clk_dispatch,13:no_clk_rrd,14:no_clk_rerd,15:no_clk_drerd,16:entry_clk_fdrerd,17: ROB_tag; //ISSUE_QUEUE
				
				
				if(execute_list_free_entry_pointer == (WIDTH*5))
				{
					execute_list_has_space = 0;
					break;
				}

														 
				execute_list[execute_list_free_entry_pointer][0] = IQ[j][4];                       //renamed source 1
				execute_list[execute_list_free_entry_pointer][1] = IQ[j][6];                       //renamed source 2
				execute_list[execute_list_free_entry_pointer][2] = IQ[j][2];                       //renamed destination 
				execute_list[execute_list_free_entry_pointer][6] = IQ[j][7];                       //og source 1
				execute_list[execute_list_free_entry_pointer][7] = IQ[j][8];                       //og source 2
				execute_list[execute_list_free_entry_pointer][8] = IQ[j][9];                       //dest og
				execute_list[execute_list_free_entry_pointer][3] = IQ[j][10];                      //op_type
				execute_list[execute_list_free_entry_pointer][9] = 1;                              //valid instruction 
				
				//////////////////////TIMING/////////////////////////////////
				execute_list[execute_list_free_entry_pointer][10] = IQ[j][11];
				execute_list[execute_list_free_entry_pointer][11] = IQ[j][12];
				execute_list[execute_list_free_entry_pointer][12] = IQ[j][13];
				execute_list[execute_list_free_entry_pointer][13] = IQ[j][14];
				execute_list[execute_list_free_entry_pointer][14] = IQ[j][15];
				execute_list[execute_list_free_entry_pointer][15] = IQ[j][16];
				execute_list[execute_list_free_entry_pointer][16] = IQ[j][17];
				
				
			
				execute_list_free_entry_pointer += 1;
				
				
				number_of_issued_inst += 1;
				IQ_entry_pointer = IQ_entry_pointer -1;                                         //Reducing the pointer and age for new isnt
				
				
				youngest -= 1;
				for(int k=(j+1);k<(IQ_size);k++)                                                             //reducin the age of the instructions older than issued inst
					IQ[k][1] = IQ[k][1] - 1;
					
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
					IQ[k][17] = IQ[k+1][17];
				}
				/*IQ[IQ_size-2][0] = IQ[IQ_size-1][0];                         //Moving the issue queue up for the last element
				IQ[IQ_size-2][1] = IQ[IQ_size-1][1];
				IQ[IQ_size-2][2] = IQ[IQ_size-1][2];
				IQ[IQ_size-2][3] = IQ[IQ_size-1][3];
				IQ[IQ_size-2][4] = IQ[IQ_size-1][4];
				IQ[IQ_size-2][5] = IQ[IQ_size-1][5];
				IQ[IQ_size-2][6] = IQ[IQ_size-1][6];
				IQ[IQ_size-2][7] = IQ[IQ_size-1][7];
				IQ[IQ_size-2][8] = IQ[IQ_size-1][8];
				IQ[IQ_size-2][9] = IQ[IQ_size-1][9];
				IQ[IQ_size-2][10] = IQ[IQ_size-1][10];
				IQ[IQ_size-2][11] = IQ[IQ_size-1][11];
				IQ[IQ_size-2][12] = IQ[IQ_size-1][12];
				IQ[IQ_size-2][13] = IQ[IQ_size-1][13];
				IQ[IQ_size-2][14] = IQ[IQ_size-1][14];
				IQ[IQ_size-2][15] = IQ[IQ_size-1][15];
				IQ[IQ_size-2][16] = IQ[IQ_size-1][16];
				IQ[IQ_size-2][17] = IQ[IQ_size-1][17];*/
				
				
				break;                                                        //Becase I want to start new iteration
			}
		}
	}
	
	std::cout<<"\n Printing the execute list\n";
	//0:src1,1:src2,2:dest,3:op_type,4:no_cyles_in_exe,5:completed?,
	for(int i=0;i<execute_list_free_entry_pointer;i++)
		std::cout<<"\n src1:"<<execute_list[i][0]<<" src2:"<<execute_list[i][1]<<" dest:"<<execute_list[i][2]<<" op_type:"<<execute_list[i][3]<<" no_cycles:"<<execute_list[i][4]<<" completed?:"<<execute_list[i][5];
	
	
}

////////////////////////////////////////////////////////////////////////////////////
void Dispatch() {
	
	//std::cout<<"\n In Dispatch stage";
	
	
	
	for(int i=0;i<WIDTH;i++){
		
		std::cout<<"\n"<<i<<" "<<"src1 R:"<<pipeline_objects[i].src1_rr_dispatch<<" src2 :R"<<pipeline_objects[i].src2_rr_dispatch<<" dest:R"<<pipeline_objects[i].dest_rr_dispatch;
		
		if((pipeline_objects[i].src1_rr_dispatch == -2)||(pipeline_objects[i].src2_rr_dispatch == -2)||(pipeline_objects[i].dest_rr_dispatch == -2))
			continue;
		
		else if((IQ_size - IQ_entry_pointer + 1) >= WIDTH)                                //Checking if we have enough space for the whole bundle 
		{
			dispatch_can_accept_new_bundle = 1;
			
			//std::cout<<"\n Writing into issue queue";
			
			IQ[IQ_entry_pointer][0] = 1;                                           //Setting valid bit as 1
			IQ[IQ_entry_pointer][1] = youngest;                                    //setting the age bit
			youngest += 1;
			IQ[IQ_entry_pointer][2] = pipeline_objects[i].dest_rr_dispatch;
			
			
			if(pipeline_objects[i].src1_ready)
				IQ[IQ_entry_pointer][3] = 1;
			else
				IQ[IQ_entry_pointer][3] = 0;
			
			
			IQ[IQ_entry_pointer][4] = pipeline_objects[i].src1_rr_dispatch;
			
			if(pipeline_objects[i].src2_ready)
				IQ[IQ_entry_pointer][5] = 1;
			else
				IQ[IQ_entry_pointer][5] = 0;
			
			IQ[IQ_entry_pointer][6] = pipeline_objects[i].src2_rr_dispatch;
			
			
			for(int k =0;k<wakeup_pointer;k++)
			{
				if(Wakeup[k] = pipeline_objects[i].src1_rr_dispatch)
					IQ[IQ_entry_pointer][3] = 1;
				if(Wakeup[k] = pipeline_objects[i].src2_rr_dispatch)
					IQ[IQ_entry_pointer][5] = 1;
			}
			
			
			IQ[IQ_entry_pointer][7] = pipeline_objects[i].src1_og_rr_dispatch;                       //Storing value
			IQ[IQ_entry_pointer][8] = pipeline_objects[i].src2_og_rr_dispatch;
			IQ[IQ_entry_pointer][9] = pipeline_objects[i].dest_og_rr_dispatch;
			IQ[IQ_entry_pointer][10] = pipeline_objects[i].op_type_rr_dispatch;
			IQ[IQ_entry_pointer][17] = pipeline_objects[i].ROB_tag_for_this_inst_rr;
			
			
			
			////////////////////////TIMING//////////////////////////////
			//pipeline_objects[i].no_clk_dispatch = 1;                                                 //Instead initialise it to one
			IQ[IQ_entry_pointer][12] = pipeline_objects[i].no_clk_dispatch;
			IQ[IQ_entry_pointer][13] = pipeline_objects[i].no_clk_rr;
			IQ[IQ_entry_pointer][14] = pipeline_objects[i].no_clk_rer;
			IQ[IQ_entry_pointer][15] = pipeline_objects[i].no_clk_drer;
			IQ[IQ_entry_pointer][16] = pipeline_objects[i].entry_clk_fdrer;
			
			
			
			
			IQ_entry_pointer += 1;
		}
		else
		{
			dispatch_can_accept_new_bundle = 0;
			
			
			//for(int j=0;j<WIDTH;j++)
				pipeline_objects[i].no_clk_dispatch += 1;
		}
	}
	
	
	////Printing ISSUE QUEUE//////
	std::cout<<"\n Printing issue queue";
	//[][0] valid;[][1] age, [][2]dst tag, rs1 rdy, rs1 tag/value,rs2 rdy, rs2 tag/value , src1_og, src2_og, dest_og, op_type, no_cycles_in_iq
	for(int i=0;i<IQ_size;i++)
		if(IQ[i][0] == 1){
			std::cout<<"\n valid:"<<IQ[i][0]<<" age:"<<IQ[i][1]<<" dst tag:"<<IQ[i][2]<<" rs1 rdy:"<<IQ[i][3]<<" rs1 tag:"<<IQ[i][4]<<" rs2 rdy:"<<IQ[i][5]<<" rs2 tag:"<<IQ[i][6];
			std::cout<<" src_og:R"<<IQ[i][7]<<" src2_og:R"<<IQ[i][8]<<" dest_og:R"<<IQ[i][9]<<" op_type:"<<IQ[i][10]<<" no_cycles:"<<IQ[i][11];
			std::cout<<"  fetch_entry:"<<pipeline_objects[i].entry_clk_fdrer<<" decode:"<<pipeline_objects[i].no_clk_drer<<" rename:"<<pipeline_objects[i].no_clk_rer;
			std::cout<<" rr:"<<pipeline_objects[i].no_clk_rr;
		}
}

void RegRead() {
	
	
	//std::cout<<"\n In RegRead stage";
	for(int i=0;i<WIDTH;i++)
	{
	std::cout<<"\n src1:"<<pipeline_objects[i].src1_rename_rr<<" src1_og:"<<pipeline_objects[i].src1_og_rename_rr;
	std::cout<<" src2:"<<pipeline_objects[i].src2_rename_rr<<" src2_og:"<<pipeline_objects[i].src2_og_rename_rr;
	std::cout<<" dest:"<<pipeline_objects[i].dest_rename_rr;
	}
	
	
	if(dispatch_can_accept_new_bundle){
		rr_can_accept_new_bundle = 1;
	for(int i=0;i<WIDTH;i++)
	{
		////////////////Moving the rob tag along///////////////////
		pipeline_objects[i].ROB_tag_for_this_inst_rr = pipeline_objects[i].ROB_tag_for_this_inst;
		
		//////////////////////////////////////////////////////////
		
		
		///////////////////////////Source 1/////////////////////////
		if(pipeline_objects[i].src1_og_rename_rr == -1)
			pipeline_objects[i].src1_ready = 1;
		else if(pipeline_objects[i].src1_rename_rr >= 1000)
		{
			int j = ROB_tail_pointer;
			while(j != ROB_head_pointer)                                 //We have to go in reverse to find the most recent version
			{
				//std::cout<<"\n j:"<<j<<" ROB_tail:"<<ROB_tail_pointer<<" ROB_head:"<<ROB_head_pointer;
				if(ROB[j][1] == pipeline_objects[i].src1_rename_rr)           //Comparing  with the renamed values also
				{
					if(ROB[j][0] == 1)     
					{
						if(ROB[j][3] == 1)                                       //Implies that the register is ready
						{
							pipeline_objects[i].src1_ready = 1;
							break;
						}
						else{
							pipeline_objects[i].src1_ready = 0;
						}
					}
				}
				
				if(j == 0)
					j = ROB_size - 1;                                      //To deal with circular FIFO
				else
					j = j - 1;
			}
			
			for(int k =0;k<wakeup_pointer;k++)
			{
				if(Wakeup[k] = pipeline_objects[i].src1_rename_rr)
					pipeline_objects[i].src1_ready = 1;
			}
		}
		else                                                                      //Means the values are ready in the ARF
			pipeline_objects[i].src1_ready = 1;  


		//////////////////////////////Source 2///////////////////////////
		if(pipeline_objects[i].src2_og_rename_rr == -1)
			pipeline_objects[i].src2_ready = 1;
		else if(pipeline_objects[i].src2_rename_rr >= 1000)
		{
			int j = ROB_tail_pointer;
			while(j != ROB_head_pointer)                                  //We have to go in reverse to find the most recent version
			{
				//std::cout<<"\n Inside RR loop 2";
				if(ROB[j][1] == pipeline_objects[i].src2_rename_rr)           //Comparing  with the renamed values also
				{
					if(ROB[j][0] == 1)     
					{
						if(ROB[j][3] == 1)                                       //Implies that the register is ready
						{
							pipeline_objects[i].src2_ready = 1;
							break;
						}
						else{
							pipeline_objects[i].src2_ready = 0;
						}
					}
				}
				
				if(j == 0)
					j = ROB_size - 1;                                      //To deal with circular FIFO
				else
					j = j - 1;
			}
			for(int k =0;k<wakeup_pointer;k++)
			{
				if(Wakeup[k] = pipeline_objects[i].src2_rename_rr)
						pipeline_objects[i].src2_ready = 1;
			}
		}
		else                                                                      //Means the values are ready in the ARF
			pipeline_objects[i].src2_ready = 1; 
			
			
		
			
			
		///////////////////TIMING////////////////////
		pipeline_objects[i].no_clk_dispatch = 1;
		pipeline_objects[i].no_clk_rrd = pipeline_objects[i].no_clk_rr;
		pipeline_objects[i].no_clk_rerd = pipeline_objects[i].no_clk_rer;
		pipeline_objects[i].no_clk_drerd = pipeline_objects[i].no_clk_drer;
		pipeline_objects[i].entry_clk_fdrerd = pipeline_objects[i].entry_clk_fdrer;
	}
	
	}
	else
	{
		rr_can_accept_new_bundle = 0;
		for(int i=0;i<WIDTH;i++)
			pipeline_objects[i].no_clk_rr += 1;
	}
}


void Initialisation_function()
{
	for(int i=0;i<WIDTH;i++){
	/////Fetch-decode////////
	pipeline_objects[i].src1_fetch_decode = -2;
	pipeline_objects[i].src2_fetch_decode = -2;
	pipeline_objects[i].dest_fetch_decode = -2;
	pipeline_objects[i].op_type_fetch_decode = -2;
	pipeline_objects[i].valid = 0;
	
	///////decode-rename/////////
	pipeline_objects[i].src1_og_decode_rename = -2;
	pipeline_objects[i].src2_og_decode_rename = -2;
	pipeline_objects[i].dest_og_decode_rename = -2;
	pipeline_objects[i].src1_decode_rename = -2;
	pipeline_objects[i].src2_decode_rename = -2;
	pipeline_objects[i].dest_decode_rename = -2;
	pipeline_objects[i].op_type_decode_rename = -2;
	
	
	///////rename-rr////////////
	pipeline_objects[i].src1_og_rename_rr = -2;
	pipeline_objects[i].src2_og_rename_rr = -2;
	pipeline_objects[i].dest_og_rename_rr = -2;
	pipeline_objects[i].src1_rename_rr = -2;
	pipeline_objects[i].src2_rename_rr = -2;
	pipeline_objects[i].dest_rename_rr = -2;
	pipeline_objects[i].op_type_rename_rr = -2;
	pipeline_objects[i].temp_dest_rename_rr = -2;
	pipeline_objects[i].temp_src1_rename_rr = -2;
	pipeline_objects[i].temp_src2_rename_rr = -2;
	
	
	
	///////rr-dispatch/////////
	pipeline_objects[i].src1_rr_dispatch = -2;
	pipeline_objects[i].src1_value = -2;
	pipeline_objects[i].src2_rr_dispatch = -2;
   	pipeline_objects[i].src2_value = -2;
	pipeline_objects[i].dest_rr_dispatch = -2;
   	pipeline_objects[i].op_type_rr_dispatch = -2;
	pipeline_objects[i].src1_og_rr_dispatch = -2;
	pipeline_objects[i].src2_og_rr_dispatch = -2;
	pipeline_objects[i].dest_og_rr_dispatch = -2;
	pipeline_objects[i].src1_ready = -2;
   	pipeline_objects[i].src2_ready = -2;
   	pipeline_objects[i].dest_ready = -2;
	
	
	
	///////////Timing///////////
	pipeline_objects[i].no_clk_rr = 1;
	
	
	
	
	}
}