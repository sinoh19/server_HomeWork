#include <stdio.h>

int main()
{
	unsigned int value = 1;
	unsigned char* byte_ptr = (unsigned char*)&value;

	if (*byte_ptr == 1)
	{
		printf("Little endian\n");
	}
	else
	{
		printf("Big endian\n");
	}

	return 0;

}