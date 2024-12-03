#ifndef SIM_PROC_H
#define SIM_PROC_H

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

/////////////Control signal/////////////
int DE_can_accept_new_bundle = 1;
int DE_contains_new_bundle = 0;
int DE_initial_entry = 1;


int RN_can_accept_new_bundle = 1;
int RN_contains_new_bundle = 0;
int RN_initial_entry = 1;
int NO_ROB_free_entries;



int RR_can_accept_new_bundle = 1;
int RR_contains_new_bundle = 0;
int RR_initial_entry = 1;


int DI_can_accept_new_bundle = 1;
int DI_contains_new_bundle = 0;
int DI_initial_entry = 1;

int execute_list_has_space = 1;


int EOF_reached = 0;

long int seq_no = 0;  
///////////////////////////////////////

unsigned int ticker = 0;

typedef struct Pipeline{
	
	
	//DE
	int src1_fetch_decode, src2_fetch_decode,dest_fetch_decode,op_type_fetch_decode, valid;     //valid is used for the case when we have fetch less than width inst
	long long int PC_fetch_decode;
	long int entry_clk_fetch;  
	int decode_cyles;
	
	//RN
	int src1_RN,src2_RN,dest_RN,op_type_RN;
	long long int PC_RN;
	long int no_clk_decode, entry_clk_FD;
	int rename_cyles;
	
	//RR
	int src1_RR,src2_RR,dest_RR,op_type_RR;   //rr and rob both should have space for new bundle
	int src1_RR_OG,src2_RR_OG,dest_RR_OG;                //Original destination is present in the ROB
	long long int PC_RR;
	int src1_RR_ready, src2_RR_ready;
    long int no_clk_RN, no_clk_DERN,entry_clk_FDERN;
	int RR_cyles;
	
	
	//DI
	int src1_DI,src2_DI,dest_DI,op_type_DI;
	int src1_DI_OG,src2_DI_OG,dest_DI_OG;
	long int no_clk_RR, no_clk_RNRR,no_clk_DERNRR,entry_clk_FDERNRR;
	long long int PC_DI;
	int src1_ready, src2_ready, dest_ready;
	int DI_cyles;
	//unsigned int no_clk_dispatch, no_clk_rrd, no_clk_rerd,no_clk_drerd,entry_clk_fdrerd;


	long int no_clk_DI;
	
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
int ROB_tags = 1001;


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

/////////////Wakeup signals////////////////////////
int *Wakeup = nullptr;
int wakeup_pointer;
	

//////////////////////////////Function Initialisations///////////////////////////////////////
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

void Initialisation_function();
////////////////////////////////////////////////////////////////////////////////////////////


///////Count variables//////
long int INST_FETCH_CNT = 0;
long int INST_RETIRE_CNT = 0;
int WIDTH = 0;

#endif