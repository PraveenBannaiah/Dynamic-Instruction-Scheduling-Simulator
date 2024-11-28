#ifndef SIM_PROC_H
#define SIM_PROC_H

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

/////////////Control signal/////////////
int decode_can_accept_new_bundle = 1;
int rename_can_accept_new_bundle = 1;
int rr_can_accept_new_bundle = 1;
int dispatch_can_accept_new_bundle = 1;
int execute_list_has_space = 1;
int ROB_can_accpet_new_bundle = 1;


///////////////////////////////////////

unsigned int ticker = 0;

typedef struct Pipeline{
	
	
	//Register between Fetch and decode stage
	int src1_fetch_decode, src2_fetch_decode,dest_fetch_decode,op_type_fetch_decode, valid;     //valid is used for the case when we have fetch less than width inst
	unsigned int PC_fetch_decode;
	unsigned int entry_clk_fetch; //Because the instruction will directly go into decode register                                                                          //Inistead of number of clock cycles 
	
	//Register between decode and rename stage
	int src1_og_decode_rename,src2_og_decode_rename,dest_og_decode_rename,src1_decode_rename, src2_decode_rename,dest_decode_rename,op_type_decode_rename;
	unsigned int PC_decode_rename;
	unsigned int no_clk_decode, entry_clk_fd;
	
	//Register between rename and register read stage
	int src1_og_rename_rr,src2_og_rename_rr,dest_og_rename_rr,src1_rename_rr,src2_rename_rr,dest_rename_rr,op_type_rename_rr;   //rr and rob both should have space for new bundle
	unsigned int PC_rename_rr;
	unsigned int no_clk_rename, no_clk_dr,entry_clk_fdr;
	
	int ROB_tag_for_this_inst;
	
	int temp_src1_rename_rr;                   //Because the data was going into rr and dispatch in the same clock cycles
	int temp_src2_rename_rr;
	int temp_dest_rename_rr;
	
	
	//Register between rr and dispatch
	int src1_rr_dispatch, src1_value, src2_rr_dispatch, src2_value,dest_rr_dispatch, op_type_rr_dispatch;
	int src1_og_rr_dispatch,src2_og_rr_dispatch,dest_og_rr_dispatch;
	unsigned int no_clk_rr, no_clk_rer,no_clk_drer,entry_clk_fdrer;
	unsigned int PC_rr_dispatch;
	int src1_ready, src2_ready, dest_ready;
	
	int ROB_tag_for_this_inst_rr;
	
	//Register between dispatch and issue is the ISSUE QUEUE
	//[][0] valid;[][1] age, [][2]dst tag, rs1 rdy, rs1 tag/value,rs2 rdy, rs2 tag/value , src1_og, src2_og, dest_og, op_type, no_cycles_in_iq
	unsigned int no_clk_dispatch, no_clk_rrd, no_clk_rerd,no_clk_drerd,entry_clk_fdrerd;
	// 12:no_clk_dispatch,13:no_clk_rrd,14:no_clk_rerd,15:no_clk_drerd,16:entry_clk_fdrerd,17: ROB_tag;
	int ROB_tag_for_this_inst_rr_d;
	
	
	//Register between Issue and Execute i.e the execute list
	//Execute list //0:src1,1:src2,2:dest,3:op_type,4:no_cyles_in_exe,5:completed?,6:src1_og,7:src2_og,8:src3_og,9:valid
	unsigned int no_clk_issue, no_clk_di, no_clk_rrdi, no_clk_rerdi,no_clk_drerdi,entry_clk_fdrerdi;
	int ROB_tag_for_this_inst_rr_d_i;
	
	/*int src1_exe1,src2_exe1,dest_exe1,op_type_exe1,no_cyles_in_ex_exe1,completed_exe1;   //We have five because a total of five instructions can be in flight in a given execute stage
	int src1_exe2,src2_exe2,dest_exe2,op_type_exe2,no_cyles_in_ex_exe2,completed_exe2;
	int src1_exe3,src2_exe3,dest_exe3,op_type_exe3,no_cyles_in_ex_exe3,completed_exe3;
	int src1_exe4,src2_exe4,dest_exe4,op_type_exe4,no_cyles_in_ex_exe4,completed_exe4;
	int src1_exe5,src2_exe5,dest_exe5,op_type_exe5,no_cyles_in_ex_exe5,completed_exe5;*/
	
	//unsigned int no_clk_exe, no_clk_ie, no_clk_di, no_clk_rrdi, no_clk_rerdi,no_clk_drerdi,entry_clk_fdrerdi;
	//int ROB_tag_for_this_inst_rr_d_i_ex;
	
	//Redister between the Execute stage and the writeback will also be dynamic as the size will again depend on WIDTH
	/*int src1_wb1,src2_wb1,dest_wb1,op_type_wb1,no_cyles_in_ex_wb1,completed_wb1;   //We have five because a total of five instructions can be in flight in a given execute stage
	int src1_wb2,src2_wb2,dest_wb2,op_type_wb2,no_cyles_in_ex_wb2,completed_wb2;
	int src1_wb3,src2_wb3,dest_wb3,op_type_wb3,no_cyles_in_ex_wb3,completed_wb3;
	int src1_wb4,src2_wb4,dest_wb4,op_type_wb4,no_cyles_in_ex_wb4,completed_wb4;
	int src1_wb5,src2_wb5,dest_wb5,op_type_wb5,no_cyles_in_ex_wb5,completed_wb5;*/
	
	//unsigned int no_clk_wb, no_clk_ew, no_clk_exe, no_clk_ie, no_clk_di, no_clk_rrdi, no_clk_rerdi,no_clk_drerdi,entry_clk_fdrerdi;
	
}pipeline;


//Reoder Buffer initialisation 
long long int** ROB = nullptr;
int ROB_head_pointer = 0;
int ROB_tail_pointer = 0;
int ROB_tail_phase = 0;
int ROB_size = 0;


//Issue Queue initialisation
int** IQ = nullptr;
int IQ_size = 0;
int IQ_entry_pointer = 0;
int youngest = 0;


//Rename Map Table
int RMT_valid_array[67];
int RMT_tag[67];

//Execute list
int **execute_list = nullptr;
	int execute_list_free_entry_pointer;


///////////////Initialising writeback buffer/////////////////////
int** WriteBack_buffer = nullptr;
int writeback_free_entry_pointer;
////////////////////////////////////////////////////////////////
	

//////////////////////////////Function Initialisations///////////////////////////////////////
void Fetch();
	
void Decode();

void Rename();

void RegRead();

void Dispatch();

void Issue();

void Execute();

void Writeback();

void Retire();

int Advance_Cycle(FILE*);

void Initialisation_function();
////////////////////////////////////////////////////////////////////////////////////////////


///////Count variables//////
long int INST_FETCH_CNT = 0;
long int INST_RETIRE_CNT = 0;
int WIDTH = 0;

#endif