#include "co.h"

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static struct coroutine *current_coroutine; // current running coroutine

static struct coroutine *coroutine_list[MAX_COROUTINE_NUM]; // all coroutine

static int colist_top = 0; // index of the top of the coroutine list

/**
 * @brief append a new-created coroutine to the coroutine list
 * @param coroutine coroutine to be appended
 */
void append_colist(struct coroutine *co)
{
    assert(co->state == CO_NEW);
    if (colist_top < MAX_COROUTINE_NUM)
        coroutine_list[colist_top++] = co;
    else
        assert(0);
}

/**
 * @brief remove a coroutine from the coroutine list and free its memory.
 * @param coroutine coroutine to be removed
 */
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

/**
 * @brief create a new coroutine , it not running but be ready for it.
 * @param entry function pointer of the coroutine,void (*entry)(void *)
 * @param arg argument of the coroutine
 */
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

/**
 * @brief main function is also a coroutine, it is the first coroutine to be created.
 */
__attribute__((constructor)) static inline void main_coroutine_init()
{
    srand(time(NULL)); // random get next coroutine
    current_coroutine = cocreate(NULL, NULL);
    current_coroutine->state = CO_READY;
}

/**
 * @brief free all coroutine in the coroutine list.
 * Acutally, it is not necessary to free all coroutine,
 */
__attribute__((destructor)) static inline void main_coroutine_clean()
{
    current_coroutine->state = CO_FINISHED; // main coroutine is finished
    // no need to to this
    for (int i = 0; i < colist_top; i++)
    {
        assert(coroutine_list[i]);
        assert(coroutine_list[i]->state == CO_FINISHED || coroutine_list[i]->state == CO_NEW); // all coroutine should be finished or not started.
        free(coroutine_list[i]);
    }
}

/**
 * @brief randomly get next coroutine to be run,maybe return current coroutine.
 * @return next coroutine to be run
 */
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

// Only support x86 but not Windows.
#if defined(__linux__) || defined(__APPLE__)
#if __x86_64__

/**
 * @brief change stack pointer and jump to the coroutine entry function.
 * @param rsp new stack bottom pointer
 * @param rip new coroutine entry function pointer
 * @param arg argument of the function
 * @details change r/esp,set pararm,and call function.In x64 rdi is the first argument and x86 push it to stack.
 * @note e/rsp is changed ,thus local variable is not accessible any more after calling return ,otherwise crash.
 */

#define CO_JUMP(rsp, rip, arg)                          \
    asm volatile(                                       \
        "movq %0, %%rsp\n\t"                            \
        "movq %2, %%rdi\n\t"                            \
        "callq *%1\n\t"                                 \
        :                                               \
        : "r"((uintptr_t)(rsp)), "r"((rip)), "r"((arg)) \
        : "memory", "rdi");
#elif defined(__i386__)
#define CO_JUMP(esp, eip, arg)                          \
    asm volatile(                                       \
        "movl %0, %%esp\n\t"                            \
        "pushl %2\n\t"                                  \
        "calll *%1\n\t"                                 \
        :                                               \
        : "b"((uintptr_t)(esp)), "r"((eip)), "r"((arg)) \
        : "memory");
#else
#error "Not support ARM etc because of calling convention."
#endif

#elif defined(_WIN32)
#error "Not support Windows because of using ATT syntax inlien assembly and x86-64 calling convention."

#else
#warning "Not Tested."

#endif

/**
 * @brief current coroutine is finished,do some work,and switch another coroutine,it will be sheduled again.
 * @warning  MUST BE NOINLINE FUNCTION.Explained in coyield().
 */
__attribute__((noinline)) static void coroutine_finished()
{
    current_coroutine->state = CO_FINISHED;
    if (current_coroutine->waiter)
        current_coroutine->waiter->state = CO_READY;
    coyield();
}

/**
 * @brief curent coroutine yield and switch an avaiable coroutine.
 */
void coyield()
{
    struct coroutine *nextco = get_next_coroutine();
    assert(nextco);                             // NULL means no coroutine could run which isn't normal.
    if (!nextco || nextco == current_coroutine) // maybe still current coroutine just return.
        return;
    if (setjmp(current_coroutine->env) == 0) // save current environment
    {
        if (nextco->state == CO_NEW)
        {
            // next coroutine is never runned,we need to checkout its entry function and stack.
            // that coroutine has not setjmp() yet,so we can't longjmp() to it.
            // use inline assembly to switch stack(%r/esp)and call entry function(%%r/eip).
            current_coroutine = nextco;
            nextco->state = CO_READY;
            CO_JUMP(nextco->stack + FIXED_STACK_SIZE, nextco->entry, nextco->arg);

            // in asm,`call` push r/eip of here to stack.
            // so if a coroutine is finished ,it will run here.

            // You need do some work but can't in here.
            // beacause r/esp is changed,so local variable is not accessible any more after calling return ,otherwise crash.
            // current_coroutine is global variable but it may be accessed by register like ( 12(%esp) )which may be changed either , because global variable was accessed before and complier don't know you change r/esp.

            // so you MUST do those in another function.
            // then complier will re-get gval like ( gloabvariable(%rip) or something)
            coroutine_finished();

            // r/esp maybe equal to nextco->stack + FIXED_STACK_SIZE - 4 in x86
            // but it affect nothing because this coroutine is not re-runned again.
            // the memory will be free in cowait or the end of main.

            assert(0); // Never run here.
        }
        // else // == CO_READY //both are ok.
        else if (nextco->state == CO_READY)
        {
            // next co runned before ,so it call setjmp() before and we can just longjmp() to it.
            current_coroutine = nextco;
            longjmp(nextco->env, (int)(!0) /*all number except 0 is ok.*/);
        }
        else
        {
            // Never run here.
            assert(0);
        }
    }
}

void cowait(struct coroutine *co)
{
    assert(co);
    if (co->state == CO_FINISHED)
    {
        // waitting coroutine is finished,free its memory.
        erase_colist(co);
        // current_coroutine is no need to be supended ,let us roll.
    }
    else
    {
        assert(co != current_coroutine); // Waitting for yourself finished is dealock.

        // When a coroutine's state is CO_WAITTING,it will not be shedlued.
        current_coroutine->state = CO_WAITTING;

        // till this's waitting coroutine finished and set this coroutine's state to CO_READY in coroutine_finished().
        co->waiter = current_coroutine;

        coyield();
    }
}
