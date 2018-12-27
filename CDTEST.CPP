#include <stdio.h>

class test {

public:
test(void);
test(int foo);
~test(void);
};

test::test(void)
{
	printf("constructor\n");
}

test::test(int foo)
{
	printf("constructor %d\n",foo);
}

test::~test(void)
{
	printf("destructor\n");
}

void main(void)
{
	printf("static\n");

	test mytest(1);

	printf("dynamic\n");

	test *mytest1 = new test;

}