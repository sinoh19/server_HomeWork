#include <stdio.h>

void check_endian()
{
	unsigned int value = 1;
	unsigned char* byte_ptr = (unsigned char*)&value;

	if (*byte_ptr == 1)
	{
		printf("Little endian �ý��� �Դϴ�.\n");
	}
	else
	{
		printf("Big endian �ý��� �Դϴ�.\n");
	}
}

int main()
{	
	check_endian();
	return 0;
}