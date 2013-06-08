/* config.c: This code is used for the reading of config files. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

char* ReadConfig(char* filename)
{
    FILE* file;
    char* str;
    
    file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Input file %s cannot be opened.!\n", filename);
        exit(1);
    }
    
	str = malloc(GetFileSize(file));
	fclose(file);
    
	return str;
}

char** ParseConfig(char* string) 
{
	int numlines;
	int i = 0;
	char* delims = " ";
	char* result = NULL;
	char** arr;
	
	numlines = NumLines(string);
	arr = malloc(numlines);
	
	result = strtok(string, delims);
	while (result != NULL) {
		/* add to array */
		arr[i] = result;
		
		/* Refer to http://www.elook.org/programming/c/strtok.html */
		result = strtok(NULL, delims);
		
		++i;
	}
	return arr;
}

long GetFileSize(FILE* file) 
{
    char tmpChar;
    unsigned long FileSize = 0;
    do
    {
        tmpChar = getc(file);
    } while (++FileSize, tmpChar != EOF);
    
    return FileSize;
}

long NumLines(char* str) 
{
	unsigned long NumLines = 1;
	unsigned long i;
	for (i = 0; str[i] != '\0'; ++i)
	{
		if (str[i] == '\n')
		{
			++NumLines;
		}
	}
	return NumLines;
}
