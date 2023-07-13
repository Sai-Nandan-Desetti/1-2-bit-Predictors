#define __STDC_FORMAT_MACROS
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#define ADDRESS_TYPE 16

//////////// BranchInfo ///////////
typedef struct {    
  uint64_t address;
  int decision;
}BranchInfo;

////////// BranchHistoryTable /////////
typedef struct {
  int *table;
  int N;
}BranchHistoryTable;

void BranchHistoryTable_init(
  BranchHistoryTable *BHT,
  int index_range, 
  int num_bits,
  int pred_initial
);
void BranchHistoryTable_free(BranchHistoryTable *BHT);

///////// Auxiliary Constants //////////
enum {NOT_TAKEN=0, TAKEN=1};
enum {STRONGLY_TAKEN=3, WEAKLY_TAKEN=2, WEAKLY_NOT_TAKEN=1, STRONGLY_NOT_TAKEN=0};