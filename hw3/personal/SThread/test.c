#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

int main()
{
    int *p = malloc(sizeof(int));
    int *q = p;
    free(p);
    if(p == NULL)
        printf("p is NULL\n");
    else
    {
        printf("p is not NULL\n");
    }
    if(q == NULL)
        printf("q is NULL\n");
    else 
    {
        printf("q is not NULL\n");
    }

    /*if(&data == NULL)
        printf("data is NULL\n");
    else 
        printf("data is %d ;not NULL\n",data);*/
    return 0;
}
