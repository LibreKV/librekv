#include <stdio.h>
#include <math.h>

//函数声明 
void tenTtwo(int x);

void main()
{
    int a;
    printf("请输入一个十进制数:");
    scanf("%d",&a);
    printf("二进制数为");
    tenTtwo(a);
}


//函数实现 
void tenTtwo(uint64_t x)
{

    uint64_t j=0;
    uint64_t d[1000];  //暂时分配1000块空间
    uint64_t num=0;    //用于计数 判断用户所需要多少长度的二进制数
    //短除法 将余数放置于数组中
    do
    {
       d[j]=(x%2);//余数
       x=(x/2); 
       j++;
       num++;
    }while(x!=1);
    
    d[num]=1;  //最后一个数为1 
    //逆向输出数组中的数据拼接成二进制数
    for(j=num;j>=0;j--) 
	printf("%d",d[j]);
	printf("\n");
}
