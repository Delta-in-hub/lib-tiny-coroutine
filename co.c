#include "co.h"

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static struct coroutine *current_coroutine;

static struct coroutine *coroutine_list[MAX_COROUTINE_NUM];

static int colist_top = 0;

void append_colist(struct coroutine *co)
{
    assert(co->state == CO_NEW);
    if (colist_top < MAX_COROUTINE_NUM)
    {
        coroutine_list[colist_top++] = co;
    }
    else
    {
        assert(0);
    }
}

void erase_colist(struct coroutine *co)
{
    if (!co)
        return;
    for (int i = 0; i < colist_top; i++)
    {
        if (coroutine_list[i] == co)
        {
            for (int j = i; j < colist_top - 1; j++)
            {
                coroutine_list[j] = coroutine_list[j + 1];
            }
            coroutine_list[colist_top - 1] = NULL;
            colist_top--;
            free(co);
            return;
        }
    }
}

struct coroutine *cocreate(void (*entry)(void *), void *arg)
{
    struct coroutine *co = (struct coroutine *)malloc(sizeof(struct coroutine));
    if (!co)
    {
        assert(co);
        return NULL;
    }
    co->entry = entry;
    co->arg = arg;
    co->state = CO_NEW;
    co->waiter = NULL;

    append_colist(co);

    return co;
}

__attribute__((constructor)) static inline void main_coroutine_init()
{
    srand(time(NULL));
    current_coroutine = cocreate(NULL, NULL);
    current_coroutine->state = CO_READY;
}

__attribute__((destructor)) static inline void main_coroutine_clean()
{
    current_coroutine->state = CO_FINISHED; // main coroutine is finished
    // no need to to this
    for (int i = 0; i < colist_top; i++)
    {
        assert(coroutine_list[i]);
        assert(coroutine_list[i]->state == CO_FINISHED);
        free(coroutine_list[i]);
    }
}

struct coroutine *get_next_coroutine()
{
    struct coroutine *colist[MAX_COROUTINE_NUM];
    int j = 0;
    for (int i = 0; i < colist_top; i++)
    {
        if (coroutine_list[i]->state == CO_READY || coroutine_list[i]->state == CO_NEW)
        {
            colist[j++] = coroutine_list[i];
        }
    }
    return j == 0 ? NULL : colist[rand() % j];
}

#if __x86_64__
#define STACK_SWITCH_CALL(rsp, rip, arg)                \
    asm volatile(                                       \
        "movq %0, %%rsp\n\t"                            \
        "movq %2, %%rdi\n\t"                            \
        "callq *%1\n\t"                                 \
        :                                               \
        : "r"((uintptr_t)(rsp)), "r"((rip)), "r"((arg)) \
        : "memory", "rdi");
#else
#define STACK_SWITCH_CALL(esp, eip, arg)                \
    asm volatile(                                       \
        "movl %0, %%esp\n\t"                            \
        "pushl %2\n\t"                                  \
        "calll *%1\n\t"                                 \
        :                                               \
        : "b"((uintptr_t)(esp)), "r"((eip)), "r"((arg)) \
        : "memory");
#endif

void coyield()
{
    struct coroutine *nextco = get_next_coroutine();
    assert(nextco);
    if (!nextco || nextco == current_coroutine)
        return;
    if (setjmp(current_coroutine->env) == 0)
    {
        if (nextco->state == CO_NEW)
        {

            // current_coroutine = nextco; //work fine
            nextco->state = CO_READY;
            current_coroutine = nextco;
            STACK_SWITCH_CALL(nextco->stack + FIXED_STACK_SIZE, nextco->entry, nextco->arg);

            current_coroutine->state = CO_FINISHED;

            if (current_coroutine->waiter)
                current_coroutine->waiter->state = CO_READY;

            coyield();

            assert(0); // Never run here.
        }
        else // == CO_READY
        {
            current_coroutine = nextco;
            longjmp(nextco->env, (int)(!0));
        }
    }
}

void cowait(struct coroutine *co)
{
    assert(co);
    if (co->state == CO_FINISHED)
    {
        erase_colist(co);
    }
    else
    {
        assert(co != current_coroutine);
        current_coroutine->state = CO_WAITTING;
        co->waiter = current_coroutine;
        coyield();
    }
}
