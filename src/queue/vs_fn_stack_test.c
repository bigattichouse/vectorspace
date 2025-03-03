#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <mcheck.h>
#include "vs_core.h"
#include "vs_fn_stack.h"


int say_hello (void *context, void *stack, int store_value, float value){
    printf("Hello! %f\n",value);
    return 1;
}

int say_goodbye (void *context, void *stack, int store_value, float value){
    printf("Bye! %f\n",value);
    return 1;
}


int third_fn (void *context, void *stack, int store_value, float value){
    printf("Third Function Auto added and shows in the stack! (value: %f)\n",value);
    return 1;
}

int second_fn (void *context, void *stack, int store_value, float value){
    printf("Second Function Auto added and shows in the stack! (value: %f)\n",value);
    return 1;
}

int first_fn (void *context, void *stack, int store_value, float value){
    printf("Adding two more items to the stack\n");
    fn_stack_push((fn_stack *)stack,context, &second_fn,1.000, 4.0);
    fn_stack_push((fn_stack *)stack,context, &third_fn,1.000, 5.0);
    return 1;
}

int main(){
    vector *v;
    fn_stack *stack;
    int sz;
    stack = fn_stack_create();
    v = vs_createvector(newid(),2);
    printf("Push 1 to stack\n");
    sz = fn_stack_push(stack, v, &say_hello, 1.0, 1.1);
    printf("Stack Now has %d items.\n",sz);

    printf("Pop 1 from stack\n");
    fn_ref *ref;
    ref = fn_stack_pop(stack);
    sz = fn_stack_size(stack);
    printf("Stack Now has %d items.\n",sz);
    printf("Execute fn: \n");
    ref->fn(ref->context,ref->stack, ref->store_value, ref->value);

    printf("Push 3 to stack (hello, hello, goodbye)\n");
    sz = fn_stack_push(stack, v, &say_hello, 0, 1.1);
    printf("Stack Now has %d items.\n",sz);
    sz = fn_stack_push(stack, v, &say_hello, 0, 2.2);
    printf("Stack Now has %d items.\n",sz);
    sz = fn_stack_push(stack, v, &say_goodbye, 0, 3.3);
    printf("Stack Now has %d items.\n",sz);

    printf("Now pop/exec all of them.\n");
    int out = -1;
    out = fn_stack_exec(stack);
    printf("result: %d\n",out);
    out = fn_stack_exec(stack);
    printf("result: %d\n",out);
    out = fn_stack_exec(stack);
    printf("result: %d\n",out);
    printf("Now pop/exec more than possible.\n");

    out = fn_stack_exec(stack);
    printf("result: %d\n",out);


    printf("Now pushing one fn that is going to push two more functions.\n");
    sz = fn_stack_push(stack, v, &first_fn, 0, 3.0);
    sz = fn_stack_size(stack);
    printf("Stack Now has %d items.\n",sz);

    printf("Executing that function.\n");
        out = fn_stack_exec(stack);
        printf("result: %d\n",out);
    sz = fn_stack_size(stack);
    printf("Stack Now has %d items.\n",sz);

    out = fn_stack_exec(stack);
    out = fn_stack_exec(stack);

    return 0;
}
