#include <stdlib.h>
#include <stdio.h>
#include "ppmrw.h"

//method to skipwhitespace
void skipwhitespace(FILE* fs)
{
	//initial fgetc
	int ch = fgetc(fs);
	//loop if ch holds white space until it doesn't
	while(isspace(ch) != 0)
	{
		ch = fgetc(fs);
	}
	//once first char reached, return one space so the filepointer is before is
	ungetc(ch, fs);
}

//method to find comments and skip
void checkforcomment(FILE* fs)
{
	//initial fgetc
	int skippedcomments = 0;
	int ch = fgetc(fs);
	if(ch == '#')
	{
		while(skippedcomments == 0)
		{
			ch = fgetc(fs);
			

			if(ch == '\n')
			{
				skippedcomments = 1;
			}
		}
		printf("\n");
	}
	ungetc(ch, fs);
}
void printRawBits(size_t const size, void const * const ptr, FILE* fs)
{
	unsigned char *b = (unsigned char*) ptr;
	unsigned char byte;
	int i, j;

	for(i = size-1; i>=0; i--)
	{
		for (j=7; j >= 0 ; j--)
		{
			byte = (b[i] >> j) & 1;
			fputc(byte, fs);
		}
	}
}

//method for reading from ascii data
int readfromP3(FILE* fs, int width, int height, int maxColVal, Pixel* pixmap)
{
	int row, col;
	unsigned int asciiVal;
	for (row = 0; row < height; row += 1)
	{
		for (col = 0; col < width; col += 1)
		{
			//Take in first value red, check for correct formatting
			if(fscanf(fs, "%d", &asciiVal) == 0)
			{
				fprintf(stderr, "Incorrect format for data.");
				return 1;
			}

			//check for wrong value
			if(asciiVal > maxColVal)
			{
				fprintf(stderr, "Value in ascii data is not within the given bounds \n");
				return 1;
			}

			pixmap[width*row + col].r = asciiVal;

			//Take in second value, green, check for correct formatting
			if(fscanf(fs, "%d", &asciiVal) == 0)
			{
				fprintf(stderr, "Incorrect format for data.");
				return 1;
			}

			//check for wrong value
			if(asciiVal > maxColVal)
			{
				fprintf(stderr, "Value in ascii data is not within the given bounds \n");
				return 1;
			}

			pixmap[width*row + col].g = asciiVal;

			//Take in third value, blue, check for correct formatting
			if(fscanf(fs, "%d", &asciiVal) == 0)
			{
				fprintf(stderr, "Incorrect format for data.");
				return 1;
			}

			if(asciiVal > maxColVal)
			{
				fprintf(stderr, "Value in ascii data is not within the given bounds \n");
				return 1;
			}

			pixmap[width*row + col].b = asciiVal;

			// //any number of white space could be between values, so skipping
			skipwhitespace(fs);
		}
	}

	//close the file since we are done with reading
	fclose(fs);

	return 0;
}

//method for reading from binary data
int readfromP6(FILE* fs, int width, int height, int maxColVal, Pixel* pixmap)
{
	int row, col;
	int intVal;

	for (row = 0; row < height; row += 1)
	{
		for (col = 0; col < width; col += 1)
		{	
			//fread to read in 3 bytes(24 bits at a time)
			fread(&intVal, 1, 1, fs);
			pixmap[width*row + col].r = intVal;

			fread(&intVal, 1, 1, fs);
			pixmap[width*row + col].g = intVal ;

			fread(&intVal, 1, 1, fs);
			pixmap[width*row + col].b = intVal;

		}
	}

	fclose(fs);
}


void insertHeaderData(FILE* fs, int width, int height, int maxColVal, char* magicnum)
{

	char buffer[257];
	//place the beginning of the magic number
	fputc('P', fs);

	fputs(magicnum, fs);

	fputs(" \n", fs);

	fputs("#Converted by Thomas Back\n", fs);

	//include the width and height
	sprintf(buffer, "%d", width);

	fputs(buffer, fs);

	fputc(' ', fs);

	sprintf(buffer, "%d", height);

	fputs(buffer, fs);

	fputs(" \n", fs);

	sprintf(buffer, "%d", maxColVal);

	fputs(buffer, fs);

	fputs("\n", fs);

}

int p3conversion(FILE* fs, int width, int height, int maxColVal, char* output, char magicnum, char* outputnum, Pixel* pixmap)
{

	int row, col, value1, value2, value3;
	unsigned int byteOutVal;
	//allocate a buffer for turning integer header data back into ascii
	char buffer[256];


	//if magic number is 3 then we do from ASCII formatted data
	if(magicnum == '3')
	{
		printf("Performing read from p3");
		if(readfromP3(fs, width, height, maxColVal, pixmap) == 1)
		{
			fprintf(stderr, "P3 conversion failed");
			return 1;
		}
	}
	else
	{
		if(readfromP6(fs, width, height, maxColVal, pixmap) == 1)
		{
			fprintf(stderr, "P6 conversion failed");
			return 1;
		}
	}

	//open file to write to
	FILE* fo = fopen(output, "w");

	//function for writing the header data
	insertHeaderData(fo, width, height, maxColVal, outputnum);


	//header data complete; begin entering in the image data
	for(row = 0; row < height; row += 1)
	{
		for(col = 0; col < width; col += 1)
		{	
			//unpack bytes into value var
			value1 = pixmap[width*row + col].r;
			value2 = pixmap[width*row + col].g;
			value3 = pixmap[width*row + col].b;

			//now convert value to ascii and write to file
			sprintf(buffer, "%d", value1);

			fputs(buffer, fo);
			fputc('\n', fo);

			sprintf(buffer, "%d", value2);

			fputs(buffer, fo);
			fputc('\n', fo);

			sprintf(buffer, "%d", value3);

			fputs(buffer, fo);
			fputc('\n', fo);

		}

	}

	//finally close the file from writing
	fclose(fo);

}

int p6conversion(FILE* fs, int width, int height, int maxColVal, char* output, char magicnum, char* outputnum, Pixel* pixmap)
{
	int row, col;
	//if magic number is 3 then we do from ASCII formatted data
	if(magicnum == '3')
	{
		if(readfromP3(fs, width, height, maxColVal, pixmap) == 1)
		{
			fprintf(stderr, "P6 conversion failed");
			return 1;
		}
	}
	//else we perform read from raw bits
	else
	{
		if(readfromP6(fs, width, height, maxColVal, pixmap) == 1)
		{
			fprintf(stderr, "P6 Conversion failed");
			return 1;
		}
	}

	//open output file for writing
	FILE* fo = fopen(output, "wb");

	//Raw bytes to be written to file
	insertHeaderData(fo, width, height, maxColVal, outputnum);

	unsigned char buffer[1];

	for(row = 0; row < height; row += 1)
	{
		for(col = 0; col < width; col += 1)
		{	
			sprintf(buffer, "%c", pixmap[width*row + col].r);
			fwrite(buffer, sizeof(buffer),1, fo);

			sprintf(buffer, "%c", pixmap[width*row + col].g);
			fwrite(buffer, sizeof(buffer),1,fo);

			sprintf(buffer, "%c", pixmap[width*row + col].b);
			fwrite(buffer, sizeof(buffer),1,fo);
		}
	}

	fclose(fo);
}

int main(int argc, char* argv[])
{
	FILE* fp;
	int ch, magicnum;
	int width, height, maxColVal;
	Pixel* pixmap;


	//check for all 3 arguments
	if (argc != 4)
	{
		fprintf(stderr, "You have only included %d arguments. 4 is required to run.\n", argc);
		return 1;
	}

	//check for correct magic number for argument 2
	if (strcmp(argv[1], "3") == 1 || strcmp(argv[1], "6") == 1)
	{
		fprintf(stderr, "The argument: %s for the output format is not valid, please enter either 3 or 6.\n", argv[1]);
		return 1;
	}

	//begin checking for the correct file content
	fp = fopen(argv[2], "r+");

	//If file not found
	if(fp == NULL)
	{
		fprintf(stderr, "File %s could not be found or opened, please use valid file name. \n", argv[2]);
		return 1;
	}

	//Start scanning the file and check for first P followed by number
	if(fgetc(fp) != 'P')
	{
		fprintf(stderr, "Incompatible file contents, cannot read correctly, no white space should be present before magic number. \n");
		return 1;
	}

	//now scan for the magic number
	magicnum = fgetc(fp);

	//Check for valid ppm magic number type
	if(magicnum != '3' && magicnum != '6')
	{
		fprintf(stderr, "%c is an invalid magic number header for input file. We only accept 3 or 6. \n", magicnum);
		return 1;
	}

	//take in next character to check if it is a space
	ch = fgetc(fp);
	//must be space after magic number
	if (isspace(ch) == 0 )
	{
		fprintf( stderr, "There must be a space after the magic number, incorrect format.");
		return 1;
	} 

	//unget the space so we can start again with space
	ungetc(ch, fp);

	skipwhitespace(fp);

	checkforcomment(fp);

	skipwhitespace(fp);

	//now read in the dimensions width and height
	if(fscanf(fp, "%d %d", &width, &height) ==	0)
	{
		fprintf(stderr, "Could not read in width or height, make sure file contents are formatted correctly.\n");
		return 1;
	}

	if(height < 0 || width < 0)
	{
		fprintf(stderr, "Height and Width must not be negative\n");
		return 1;
	}

	ch = fgetc(fp);

	if (isspace(ch) == 0)
	{
		fprintf(stderr, "One space is required after the width and height \n");
		return 1;
	}

	ungetc(ch, fp);

	skipwhitespace(fp);
	//check for comments again since they could be anywhere
	checkforcomment(fp);

	skipwhitespace(fp);


	//read in the max color value
	if(fscanf(fp, "%d", &maxColVal) == 0)
	{
		fprintf(stderr, "Could not read in Max Color Value properly, make sure file contents are formatted correctly.\n");
		return 1;
	}

	if(maxColVal < 0)
	{
		fprintf(stderr, "Max Color Value cannot be negative \n");
		return 1;
	}

	//check for one space after the max color value again
	if(isspace(fgetc(fp)) == 0 )
	{
		fprintf(stderr, "One space required after the max color value, incorrect format \n");
		return 1;
	}

	//error checking for bad height and width values
	if( maxColVal < 0 || maxColVal > 256)
	{
		fprintf(stderr, "Max Color Values must be between 0-256 \n");
		return 1;
	}
	
	//allocate space based on the height and width of image
	pixmap = malloc(sizeof(Pixel) * width * height);

	//skip white space again so we are pointd at the first character of ascii data
	skipwhitespace(fp);

	//now that the values are read in we can begin the conversion process
	if(strcmp(argv[1], "3") == 0)
	{
		//run the conversion to P3
		if(p3conversion(fp, width, height, maxColVal, argv[3], magicnum, argv[1], pixmap) == 1)
		{
			return 1;
		}
	}
	else
	{
		if(p6conversion(fp, width, height, maxColVal, argv[3], magicnum, argv[1], pixmap) == 1)
		{
			return 1;
		}
	}

	printf("Successfully converted! \n");

	return 0;
}
