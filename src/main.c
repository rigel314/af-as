/*
 * main.c
 *
 *  Created on: Jan 17, 2014
 *      Author: cody
 */

#include <stdio.h>

#include "preprocessor.h"

int main(int argc, char* argv[])
{
	FILE* fp;
	
	if(argc != 2)
	{
		printf("Invalid Arguments!\n");
		return 1;
	}
	
	fp = fopen(argv[1], "r");
	if(!fp)
	{
		printf("Invalid File!\n");
		return 2;
	}
	fclose(fp);
	
	printf("%s", preprocessFile(argv[1]));
	
	return 0;
}
