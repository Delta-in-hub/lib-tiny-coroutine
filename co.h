#ifndef SIMPLECOROUTINE
#define SIMPLECOROUTINE

#define FIXED_STACK_SIZE 64 * 1024
#define MAX_COROUTINE_NUM 1024

#include <setjmp.h>
#include <stdint.h>

enum coroutine_state
{
    CO_NEW = 1,  // new coroutine create by costart()
    CO_READY,    // ready to run,maybe now running or not
    CO_WAITTING, // the coroutine call cowait()
    CO_FINISHED, // the coroutine finished the job
};

struct coroutine
{
    void (*entry)(void *);
    void *arg;

    struct coroutine *waiter; // the coroutine that is waiting for this one to finish
    enum coroutine_state state;

    uint8_t stack[FIXED_STACK_SIZE];
    jmp_buf env;
};

struct coroutine *cocreate(void (*entry)(void *), void *arg);
void coyield();
void cowait(struct coroutine *co);

#endif