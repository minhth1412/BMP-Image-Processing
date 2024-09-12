#include "Header.h"

int main(int argc, char* argv[])
{
	if (argc < 4 || argc > 5)														//Check if the commands is higher or lower than permission
	{
		cout << "Syntax Error! The command has to have 4 or 5 data only, include:\n1. Name of the .exe file to run\n";
		cout << "2. The request with file(-conv or -zoom)\n3. The location to load BMP file\n4. The destination to save BMP file\n5. And the ratio S if the request is -zoom!\n";
		return 0;
	}
	img image;
	readBMPfile(argv[2], image);													//Read the image
	RGB* data = ReadPixelArray(image);												//Read the pixel data from the 'image'
	if (strcmp(argv[1], "-conv") == 0 && argc == 4)									//Check if the command is file-converting
	{
		cout << "Read the command successfully, please wait for the convertion!\n";
		ConvertTo8BitFile(argv[3], data, image);									//Function to convert BMP file into 8 bit
	}
	else if (strcmp(argv[1], "-zoom") == 0 && argc == 5)							//Check if the command is file-zooming out							
	{
		cout << "Read the command successfully, please wait for the zooming out!\n";
		unsigned long S = atoi(argv[4]);											//Transform the ratio from string to int by using atoi order
		ReSizetoSmallerSize(argv[3], image, S, data);								//Function to zoom out the image
	}
	else
		cout << "Error occurs with the command, please try next time!\n";			//This is written when the command is less or more the number of permission
	delete[] image.BitmapData;
	image.BitmapData = NULL;

	/*//I really don't understand this!
	//In theory, these 4 lines below have to be written to delete the dynamic array which is allocated before
	//But when I drop these lines out of my code, it runs smoothly without error of heap memories!
	unsigned long temp = image.header.DataOffset - 14 - sizeof(DIB_Header);
	if (temp != 0)
	{
		delete[] image.dibReserved;													
		image.dibReserved = NULL;
	}*/
	delete[] data;									//delete the dynamic array of pixel data
	data = NULL;
	return 0;
}