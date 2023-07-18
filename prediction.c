#include "branch.h"

typedef BranchHistoryTable PredictionTable;

// FUNCTION PROTOTYPES //
void record_ground_truth(FILE *input_file);
void one_bit_prediction(int address_bits);
void two_bit_prediction(int address_bits);

//////////////////////////////////// main() ///////////////////////////////////
int main(int argc, char *argv[]) 
{ 
  int option;
  FILE *inputFile = NULL;  
  
  if (argc != 2) {
    fprintf(stderr, "Usage: .\\branch.exe <input trace file>\n");
    exit(-1);
  }
  
  inputFile = fopen(argv[1], "r");
  assert(inputFile != NULL);
      
  record_ground_truth(inputFile);

  do{
      printf("Which predictor do you want to use?\n");
      printf("1. One-bit predictor.\n");
      printf("2. Two-bit predictor.\n");
      scanf("%d", &option);

      switch (option) {
        case 1: one_bit_prediction(4); break;
        case 2: two_bit_prediction(4); break;
        default: printf("Please enter a valid option!\n");
      }
      printf("Do you want to continue? (1(yes) / 0(no))");
      scanf("%d", &option);
  }while(option == 1);
  
  

  return 0;
}

/////////////////////////////////////////// FUNCTIONS //////////////////////////////////////
void record_ground_truth(FILE* inputFile)
{
  // See the documentation to understand what these variables mean.
  int32_t microOpCount;
  uint64_t instructionAddress;
  int32_t sourceRegister1;
  int32_t sourceRegister2;
  int32_t destinationRegister;
  char conditionRegister;
  char TNnotBranch;
  char loadStore;
  int64_t immediate;
  uint64_t addressForMemoryOp;
  uint64_t fallthroughPC;
  uint64_t targetAddressdecisionBranch;
  char macroOperation[12];
  char microOperation[23];

  int64_t totalMicroops = 0;
  int64_t totalMacroops = 0;       
    
  FILE *gt_file, *stats_file;        

  // open the relevant files
  stats_file = fopen("branch.info", "wb");  

  while (true) {
    int result = fscanf(inputFile, 
                        "%" SCNi32
                        "%" SCNx64 
                        "%" SCNi32
                        "%" SCNi32
                        "%" SCNi32
                        " %c"
                        " %c"
                        " %c"
                        "%" SCNi64
                        "%" SCNx64
                        "%" SCNx64
                        "%" SCNx64
                        "%11s"
                        "%22s",
                        &microOpCount,
                        &instructionAddress,
                        &sourceRegister1,
                        &sourceRegister2,
                        &destinationRegister,
                        &conditionRegister,
                        &TNnotBranch,
                        &loadStore,
                        &immediate,
                        &addressForMemoryOp,
                        &fallthroughPC,
                        &targetAddressdecisionBranch,
                        macroOperation,
                        microOperation);
                        
    if (result == EOF) {
      break;
    }

    if (result != 14) {
      fprintf(stderr, "Error parsing trace at line %" PRIi64 "\n", totalMicroops);
      abort();
    }

    // For each micro-op
    totalMicroops++;

    // For each macro-op:
    if (microOpCount == 1) {
      totalMacroops++;
    }            
    
    if (TNnotBranch == 'T' || TNnotBranch == 'N') {
        // create a BranchInfo object
        BranchInfo B;
        // to record the address
        B.address = instructionAddress;
        // and the decision        
        B.decision = (TNnotBranch == 'T');
        // store it directly in the file
        fwrite(&B, sizeof(BranchInfo), 1, stats_file);          

        // can print the decision in the ground truth file
        // fprintf(gt_file, "%d ", (TNnotBranch == 'T'));        
    }    
  }
  fclose(inputFile);
  fclose(gt_file);  
  fclose(stats_file);        
}



void one_bit_prediction(int address_bits) {
  FILE *stats_file = fopen("branch.info", "rb");
  FILE *pred_file = fopen("1_prediction.txt", "w");

  BranchInfo B;  
  PredictionTable PT;

  unsigned long long table_size;
  int prediction_bits;
  int index;
  int ground_truth, prediction, pred_state, pred_msb;
  unsigned long long acc_ctr;
  unsigned long long no_of_branches;  
  
  // Initializations
  table_size = (int) pow(ADDRESS_TYPE, address_bits) - 1; 
  prediction_bits = 1; 
  acc_ctr = 0;
  no_of_branches = 0;

  BranchHistoryTable_init(&PT, table_size, prediction_bits, TAKEN);
          
  while(fread(&B, sizeof(BranchInfo), 1, stats_file) == 1) {   

    // get the ground truth decision        
    ground_truth = B.decision;
    
    // pick the appropriate index
    index = B.address % table_size;

    // get the prediction entry at that position
    pred_state = PT.table[index];

    // if prediction is accurate
    if (ground_truth == pred_state)
        acc_ctr++;        

    else
        // update the prediction table
        PT.table[index] = ground_truth;

    // increment branch counter (total number of branches required for computing accuracy)
    no_of_branches++;
  }
  
  // record the accuracy
  fprintf(pred_file, "Accuracy: %lf", (double) acc_ctr / (double) no_of_branches);  

  // record the final version of the prediction table
  fprintf(pred_file, "\n\nPrediction Table:\n");  
  for (int i = 0; i < PT.N; i++)
      fprintf(pred_file, "%d\n", PT.table[i]);

  fclose(stats_file);
  fclose(pred_file);
}



void two_bit_prediction(int address_bits) {

  FILE *stats_file = fopen("branch.info", "rb");
  FILE *pred_file = fopen("2_prediction.txt", "w");

  BranchInfo B;  
  PredictionTable PT;

  unsigned long long table_size;
  int prediction_bits;
  int index;
  int ground_truth, prediction, pred_state, pred_msb;
  unsigned long long acc_ctr;
  unsigned long long no_of_branches;
  
  // Initializations
  table_size = (int) pow(ADDRESS_TYPE, address_bits) - 1;
  prediction_bits = 2;
  acc_ctr = 0;
  no_of_branches = 0;

  BranchHistoryTable_init(&PT, table_size, prediction_bits, STRONGLY_TAKEN);
          
  while(fread(&B, sizeof(BranchInfo), 1, stats_file) == 1) {   

    // get the ground truth decision        
    ground_truth = B.decision;
    
    // pick the appropriate index
    index = B.address % table_size;

    // get the prediction entry at that position
    pred_state = PT.table[index];

    // we AND with '10' (==2 in decimal) to get the value at the most significant bit
    pred_msb = pred_state & 2;
    // note that pred_msb can either be '10' (==2) or '00'(==0)

    // if pred_msb == 2, then MSB is 1
    // else if pred_msb == 0, then MSB is 0
    // prediction = pred_msb == 2 ? TAKEN : NOT_TAKEN;    
    // fprintf(pred_file, "%d ", prediction);

    // if the prediction is accurate, then increment the accuracy counter
    if ((ground_truth << 1) == pred_msb)
      acc_ctr++;

    if (ground_truth == TAKEN) {
      if (pred_state != STRONGLY_TAKEN)
          pred_state++; 
    }
    else 
      if (pred_state != STRONGLY_NOT_TAKEN)
          pred_state--; 

    // update the prediction table
    PT.table[index] = pred_state;

    // increment branch counter (total number of branches required for computing accuracy)
    no_of_branches++;
  }
  
  // record the accuracy
  fprintf(pred_file, "Accuracy: %lf", (double) acc_ctr / (double) no_of_branches);  

  // record the final version of the prediction table
  fprintf(pred_file, "\n\nPrediction Table:\n");  
  for (int i = 0; i < PT.N; i++)
      fprintf(pred_file, "%d\n", PT.table[i]);

  fclose(stats_file);
  fclose(pred_file);
}
