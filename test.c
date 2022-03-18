#include <stdio.h>
#include "co.h"

int cnt = 1;

void fun(void *arg)
{
    const char *s = arg;
    for (int i = 0; i < 5; i++)
    {
        printf("%c[%d](%d)  ", *s, cnt++, i + 1);
        coyield();
    }
}

#define N 1023
// #define N 10

struct coroutine *list[N];
char str[] = "abcdefghijklmnopqrstuvwxyz";
int main(int argc, char **argv)
{

    setbuf(stdout, NULL);
    printf("%s\n", *argv);
    printf("Start!\n\n");

    for (int i = 0; i < N; i++)
    {
        list[i] = cocreate(fun, &str[i % (sizeof(str) - 1)]);
    }

    for (int i = 0; i < N; i++)
    {
        cowait(list[i]);
    }

    printf("\n\nDone!\n\n");
    return 0;
}