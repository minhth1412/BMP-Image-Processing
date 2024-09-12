#pragma once
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>

using namespace std;

struct RGB			// This 4 lines below are in ordered: blue -> green -> red -> reserved(if exists)!
{
	unsigned char Blue;					// This can be presented if we just read the 8-bit BMP file!
	unsigned char Green;
	unsigned char Red;
	unsigned char Reserved;				//This occurs when we read the 32-bit BMP file, and in another cases, it equals to zero!
	RGB() {
		Blue = Red = Green = Reserved = 0;
	}
};

struct BMP_Header	/*4 header of BMP file, 14 bytes in total*/
{
	char Signature[2];					//It specifies the file type, with the bmp file is BM
	unsigned long  FileSize = 0;			//Size of the bitmap file
	unsigned long  Reserved = 0;			//No need to do with this, and I still don't know what these things deal with the BMP file
	unsigned long  DataOffset = 0;			//Contain the first pixel array's saving point
	BMP_Header() {
		FileSize = Reserved = DataOffset = 0;
		Signature[0] = Signature[1] = ' ';
	}
};

struct DIB_Header		/*The rest is belong to the DIB, 40 bytes in total in this structure(can be more bytes in the latest version, especially the V5HEADER type (it's 124 in sum))*/
{
	unsigned long   Size = 0;				//The size of this DIB header
	unsigned long   Width = 0;				//Bmp width (pixels)
	unsigned long   Height = 0;				//Bmp height(pixels)
	unsigned short  Planes = 0;				//The number of colour planes which are used(1 in general)
	unsigned short  BitPerPixel = 0;		//The number of bit per pixel(typical values are 1, 4, 8, 16, 24 & 32
	unsigned long   Compression = 0;		//The compression method being used
	unsigned long   ImageSize = 0;			//This is the size of raw bmp data
	unsigned long   XpixelsPerMeter = 0;	//The horizontal resolution of the image
	unsigned long   YpixelsPerMeter = 0;	//The vertical resolution of the image
	unsigned long   ColorsUsed = 0;			//The number of colours being used in the color palette, or 0
	unsigned long   ImportantColor = 0;		//The number of important colours being used, or 0
};

struct img {
	BMP_Header header;		//Save the header of BMP file
	DIB_Header dibHeader;	//Save the DIB header of BMP file
	char* dibReserved;		// Save the rest of dibHeader (if it exists!)
	//There is a category in BMP file structure, called Color Table(or color pallette)
	//This appears when the BMP file need to write is 1, 4 or 8 bit!
	//And with another files, Color Table isn't exist
	//With the 32bit bmp file, there is something occur in between the first 40 bytes of DIB Header and the Pixel Data
	//And with some others latest type of BMP, the DIB header can contain more than 40 bytes
	//So this will work as a place to contain the rest data of BMP file that computer needs to read
	char* BitmapData;		//Save the pixel data of the BMP file
	img()
	{
		dibReserved = NULL;
		BitmapData = NULL;
	}
};

void readBMPfile(char* location, img& image);		//function to read the information of BMP file
void writeBMPfile(char* destination, img image);	//function to write the information of BMP file
RGB* ReadPixelArray(img& image);					//function to read the pixel data in details
void WritePixelArray(img& image, RGB*& data);		//function to write the pixel data in details
void ConvertTo8BitFile(char* destination, RGB*& rgb, img image);		//funciton to convert 24-bit or 32-bit bmp file into 8-bit file
void ReSizetoSmallerSize(char* destination, img image, int S, RGB*& data);		//function to resize the bmp file to smaller one with have the given ratio S
RGB CalculateAveragePerBlock(RGB* data, int i, int j, int S, img image, unsigned long newSize);