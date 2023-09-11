#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern const long _VS_MAX_DIMS;

typedef int (*vs_stack_fn)(void *, void *, float)  ;

typedef struct
{
  char *term;
  vs_stack_fn fn;
  void *context;
  void *stack;
  float value;
  void *next;
} fn_ref; //vs;_value

typedef struct {
    int count;
    fn_ref *last, *current;
} fn_stack;

fn_stack *fn_stack_create();
int fn_stack_push(fn_stack *stack, void *context, vs_stack_fn fn, float value);
int fn_stack_size(fn_stack *stack);
fn_ref *fn_stack_pop(fn_stack *stack);
int fn_stack_exec(fn_stack *stack);
