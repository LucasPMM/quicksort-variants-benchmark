#ifndef INPUT_H
#define INPUT_H

typedef enum { INPUT_ASCENDING, INPUT_DESCENDING, INPUT_RANDOM } InputOrder;

int **create_input_arrays(InputOrder order, int length, int count);
int **copy_input_arrays(int *const *arrays, int length, int count);
void free_input_arrays(int **arrays, int count);

#endif
