#include <stdlib.h>

#include "stack.h"

int stack_size(const RangeStack *stack) {
    return stack->size;
}

int stack_is_empty(const RangeStack *stack) {
    return stack->top == stack->bottom;
}

int stack_init(RangeStack *stack) {
    stack->top = malloc(sizeof *stack->top);
    if (stack->top == NULL) {
        stack->bottom = NULL;
        stack->size = 0;
        return 0;
    }
    stack->bottom = stack->top;
    stack->top->next = NULL;
    stack->size = 0;
    return 1;
}

/* A push stores its range in the former top node and adds a new top marker. */
int stack_push(RangeStack *stack, SortRange range) {
    StackNode *node = malloc(sizeof *node);
    if (node == NULL) {
        return 0;
    }
    stack->top->range = range;
    node->next = stack->top;
    stack->top = node;
    ++stack->size;
    return 1;
}

void stack_pop(RangeStack *stack, SortRange *range) {
    if (stack_is_empty(stack)) {
        return;
    }
    StackNode *node = stack->top;
    stack->top = node->next;
    free(node);
    --stack->size;
    *range = stack->top->range;
}

void stack_destroy(RangeStack *stack) {
    while (stack->top != NULL) {
        StackNode *next = stack->top->next;
        free(stack->top);
        stack->top = next;
    }
    stack->bottom = NULL;
    stack->size = 0;
}
