#ifndef STACK_H
#define STACK_H

typedef struct {
    int right;
    int left;
} SortRange;

typedef struct StackNode {
    SortRange range;
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *bottom;
    StackNode *top;
    int size;
} RangeStack;

int stack_init(RangeStack *stack);
int stack_push(RangeStack *stack, SortRange range);
void stack_pop(RangeStack *stack, SortRange *range);
int stack_is_empty(const RangeStack *stack);
int stack_size(const RangeStack *stack);
void stack_destroy(RangeStack *stack);

#endif
