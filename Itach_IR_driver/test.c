#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



char *code = "sendir,1:1,1,38000,1,69,341,170,21,64";

int main(int argc, char **argv)
{
	char buf[500];
	int a = 5;
	memset(buf, 0, 500);
	strcpy(buf, code);

	printf("%c\n", a +48);
	buf[strlen("sendir,1:")] = a+48;
	printf("%s\n", buf);
	return 0;
}

