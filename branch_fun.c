#include "branch.h"

void BranchHistoryTable_init(
    BranchHistoryTable *BHT, 
    int index_range, 
    int num_bits, 
    int pred_initial
) {

    (*BHT).N = index_range;
    
    (*BHT).table = (int *) calloc((*BHT).N, sizeof(int));
    for (int i = 0; i < (*BHT).N; i++)
        (*BHT).table[i] = pred_initial;
}

void BranchHistoryTable_free(BranchHistoryTable *BHT) {    
    free((*BHT).table);
}