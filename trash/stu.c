#include<stdio.h>
typedef unsigned int uint;

struct student
{
    uint id;
    char *sex;
    char *name;
};
void displayStudent(struct student *);
void disStuArray(int,struct student *);

int main()
{
    //优势在这儿，如果这个结构体有几十个成员，而我有只需赋值中间某几个，
    //当然了可以先给结构分配内存，在用stu.name = "" 或 p->name=""
    //的方式进行赋值，不过这好像麻烦了一点点。

    //所以 那什么 叫指定初始化来着。这就叫指定初始化，
    struct student stus[]={
            [0] = {21,"g","stu4"},
            [3] = {
                .id = 22,
                .name = "stu5",
                .sex = "b"
            }
    };
    disStuArray(4,stus);
}

void displayStudent(struct student *s)
{
    printf("学号:%-5d 性别:%-10s  姓名:%-10s  \r\n",s->id,s->sex,s->name);
}

void disStuArray(int n,struct student *p)
{
    while(n--)
        displayStudent(p++);
}
