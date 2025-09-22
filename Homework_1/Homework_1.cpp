#include <stdio.h>

void check_endian()
{
	unsigned int value = 1;
	unsigned char* byte_ptr = (unsigned char*)&value;

	if (*byte_ptr == 1)
	{
		printf("Little endian 시스템 입니다.\n");
	}
	else
	{
		printf("Big endian 시스템 입니다.\n");
	}
}

int main()
{	
	check_endian();
	return 0;
}