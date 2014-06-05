#include <stdio.h>
int main(int argc, char *argv[])
{
	int i;
	if(argc != 3)
		printf("error\n");
	else
	{
		i = syscall(338, argv[1], argv[2]);
		printf("success\n");
	}
	return 0;
}
