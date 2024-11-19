#ifndef SIM_PROC_H
#define SIM_PROC_H

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;


// Put additional data structures here as per your requirement

//Defining a pipeline class

/*typedef class Pipeline_architecture{
	int src_reg1,src_reg2,dest_reg;
	int operation type;
	unsigned long int PC;
	
	int width;
	
	public:
	
	Pipeline_architecture(unsigned int long width_of_pipeline) : width(width_of_pipeline);                //Constructor to initialise 
	
	void Fetch();
	
	void Decode();
	
	void Rename();
	
	void Register_read();
	
	void Dispatch();
	
	void Issue();
	
	void Execute();
	
	void Writeback();
	
	void Retire();
	
	void Advance_Cycle();
}pipeline;*/


//New idea
//Create a strcture to house the lines of superscalar proessor and create a 2d array with rows corresponsding to the number of pipeline stages and columsn corresponding to valeus


//The idea is to define an array of address pointers of Reorder_Buffer type and use the array index to implement the full and empty conditions
/*typedef class Reorder_Buffer{
	unsigned long int value, PC, tag;              //Initialise tag to a value that can never be mistaken for a register, like 100 and then keep +1(int)for next inst
	int dst,rdy,exc,mis, valid;
}ROB;

int ROB_head_pointer = 0;
int ROB_tail_pointer = 0;
int ROB_tail_phase = 0;



//NO idea how to pick ready instructions from the Issue queue, maybe age bits(same as parsing serially ig because we are entering in program order
typedef class Issue_Queue{
	int valid;
	int dst, rs1_rdy,rs2_rdy;
	unsigned long int rs1_tagORvalue,rs2_tagORvalue;
}IQ;
	


//Architectural Register File
unsigned int long ARF[67];                              //0-66  , Given in the specs

//Rename Map Table
int RMT_valid_array[67];
int RMT_rob_tag[67];*/

#endif
