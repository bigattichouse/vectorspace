#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "vs_core.h"
#include "vs_fn_stack.h"

fn_stack *fn_stack_create(){
    fn_stack *stack;
    stack = malloc(sizeof(fn_stack));
    stack->count=0;
    stack->current = NULL;
    stack->last = NULL;
    return (stack);
}

int fn_stack_push(fn_stack *stack,void *context, vs_stack_fn fn, float value){
    fn_ref *ref,*last;
    ref = malloc(sizeof(fn_ref));
    ref->fn = fn;
    ref->next = NULL;
    ref->value = value;
    ref->context = context;
    ref->stack = (void *) stack;
    if(stack->current==NULL){
        stack->current = ref;
        stack->last = ref;
    } else {
        last = stack->last;
        last->next = ref;
        stack->last = ref;
    }
    stack->count+=1;
    return stack->count;
}

int fn_stack_size(fn_stack *stack){
    return stack->count;
}

fn_ref *fn_stack_pop(fn_stack *stack){
    fn_ref *popped;
    if(stack->count==0) { return NULL; }
    popped = stack->current;
    stack->current = popped->next;
    stack->count-=1;
    if(stack->count==0) { stack->current = NULL; stack->last=NULL; }
    return popped;
}

int fn_stack_exec(fn_stack *stack){
    fn_ref *popped;
    popped = fn_stack_pop(stack);
    if(popped==NULL){
        return -1;
    }
    int out;
    out = popped->fn((void *)popped->context, (void *)popped->stack, popped->value);
    free(popped);
    return out;
}
