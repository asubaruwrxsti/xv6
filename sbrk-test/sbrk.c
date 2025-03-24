#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    void *old_brk, *new_brk;

    old_brk = sbrk(0);
    printf("Old program break: %p\n", old_brk);

    sbrk(1);

    new_brk = sbrk(0);
    printf("New program break: %p\n", new_brk);

    return 0;
}
