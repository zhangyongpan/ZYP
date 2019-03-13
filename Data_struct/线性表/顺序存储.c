#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define MAXSIZE 20
#define OK 1
#define ERROR 0
#define TRUE 1
#define FALSE 0
typedef int Status;

typedef int ElemType;
typedef struct 
{
    ElemType data[MAXSIZE];
    int length;    //线性表当前长度
}Sqlite;

/****************************************获得元素操作***********************************/
Status GetElem(Sqlite L,int i,ElemType *e)
{
    if(L.length == 0 || i < 1 || i > L.length)
        return ERROR;
        *e = L.data[i-1];
        return ok;
}

/*********************************************插入操作*********************************/
Status ListInsert(Sqlite *L,int i,ElemType e)
{
    int k;
    if(L ->length == MAXSIZE)
    return ERROR;
    if(i <1 || i > L->length+1)
        return ERROR;
    if(i <= L->length)
    {
        for(k = L->length-1;k >= i-1;k--)    //插入位置后数据依次后移一位
          L->data[k+1] = L->data[k];
    }
    L->data[i-1] = e;       //新数据插入
    L->length++;
    return OK;
}
/*************************************删除元素***************************/
Status ListDelete(Sqlite *L,int i,ElemType *e)
{
    int k;
    if(L->length == 0)
     return ERROR;
     if(i <1 || i >L->length)
        return ERROR;
    *e =L->data[i-1];
    if(i < L->length)
    {
        for(k =i;k >L->length;k++)
        L->data[K-1] = L->data[k];
    }
    L->length--;
    return Ok;
    
}