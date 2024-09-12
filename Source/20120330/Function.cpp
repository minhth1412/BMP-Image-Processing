#include "Header.h"

void readBMPfile(char* location, img& image) {
	//First, let's open the BMP file to read the information inside, and check if it is opened or not
	FILE* fp = fopen(location, "rb");
	if (fp == NULL) {											//Check if not open the file yet
		cout << "Can't open the file to read!";
		return;
	}
	cout << "Read the file sucessfully!\n";
	//2 lines below is to read the Header of BMP file, cause I left the structure of the Signature is an array with 2 elements 
	fread(image.header.Signature, 2, 1, fp);
	fread(&image.header.FileSize, 3 * sizeof(unsigned long), 1, fp);

	//Continue to read the DIB Header
	fread(&image.dibHeader.Size, sizeof(DIB_Header), 1, fp);
	
	//Let's read the rest information of the DIB header
	unsigned long temp = image.header.DataOffset - 14 - sizeof(DIB_Header);			//this is the size of dibReserved which is the data between the first 40 bytes of DIB and the first byte of pixel data
	if (temp != 0) {
		image.dibReserved = new char[temp];
		fread(image.dibReserved, sizeof(char), temp, fp);
	}

	//4 lines below is in case when the bmp file don't have ImageSize, I will create one. I met some circumstances like that
	unsigned long paddingBytes = (4 - (((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width) % 4)) % 4;						//the padding bytes
	unsigned long DataSize = ((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width + paddingBytes) * image.dibHeader.Height;		//size of the pixel array
	image.dibHeader.ImageSize = DataSize;
	image.BitmapData = new char[DataSize];

	fread(image.BitmapData, sizeof(char), DataSize, fp);			//And finally, let's read the pixel data and save it in image.BitmapData
	fclose(fp);					//Of course, let's close the file when we are done to read it
}

void writeBMPfile(char* destination, img image) {
	FILE* fp = fopen(destination, "wb");
	if (!fp) {												//Check if not open the file yet
		cout << "Can't open the file to write!";
		return;
	}
	cout << "Write the file successfully!\n";					//Then write down the BMP file following the known-order
	fwrite(image.header.Signature, 2, 1, fp);
	fwrite(&image.header.FileSize, 3 * sizeof(unsigned long), 1, fp);
	fwrite(&image.dibHeader.Size, sizeof(DIB_Header), 1, fp);
	if (image.header.DataOffset - 14 - sizeof(DIB_Header) != 0)
		fwrite(image.dibReserved, sizeof(char), image.header.DataOffset - 14 - sizeof(DIB_Header), fp);
	unsigned long paddingBytes = (4 - (((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width) % 4)) % 4;						//the padding bytes
	unsigned long DataSize = ((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width + paddingBytes) * image.dibHeader.Height;		//size of the pixel array
	fwrite(image.BitmapData, sizeof(char), DataSize, fp);
	fclose(fp);							//close the file when we are done to read it
}

RGB* ReadPixelArray(img& image) {
	unsigned long Count = 0;					//A value presented to the current position of the data to be written
	unsigned long paddingBytes = (4 - (((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width) % 4)) % 4;						//the padding bytes
	unsigned long DataSize = ((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width + paddingBytes) * image.dibHeader.Height;		//size of the pixel array
	RGB* data = new RGB[DataSize];										//This is what we will return
	char* itrt = image.BitmapData;										//Set up a pointer to image's pixel data for short
	for (unsigned long i = 0; i < image.dibHeader.Height; i++) {
		for (unsigned long j = 0; j < image.dibHeader.Width; j++) {
			if (Count >= DataSize)						//This line is just in case the IDE warning the error violent accessing to the unprovided heap memories!
				break;
			if (image.dibHeader.BitPerPixel >= 8)		//This is in case the image is a 8-bit BMP file
				data[Count].Blue = *(itrt++);
			if (image.dibHeader.BitPerPixel >= 24) {	//This is in case the image is a 24-bit BMP file
				data[Count].Green = *(itrt++);
				data[Count].Red = *(itrt++);
			}
			if (image.dibHeader.BitPerPixel == 32)		//This is in case the image is a 32-bit BMP file
				data[Count].Reserved = *(itrt++);
			Count++;
		}
		itrt += paddingBytes;		//Move the pointer a distance equal to padding bytes to read the next pixel data
	}
	return data;
}

void WritePixelArray(img& image, RGB*& data) {
	long Count = 0;				//A value presented to the current position of the data to be saved
	unsigned long paddingBytes = (4 - (((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width) % 4)) % 4;						//the padding bytes
	unsigned long DataSize = ((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width + paddingBytes) * image.dibHeader.Height;		//size of the pixel array
	image.BitmapData = new char[DataSize];
	char* itrt = image.BitmapData;						//Set up a new pointer to image's pixel data for short
	for (unsigned long i = 0; i < image.dibHeader.Height; i++) {
		for (unsigned long j = 0; j < image.dibHeader.Width; j++) {
			if (image.dibHeader.BitPerPixel >= 8) {		//This is in case the image is a 8-bit BMP file
				*(itrt++) = data[Count].Blue;
			}
			if (image.dibHeader.BitPerPixel >= 24) {	//This is in case the image is a 24-bit BMP file
				*(itrt++) = data[Count].Green;
				*(itrt++) = data[Count].Red;
			}
			if (image.dibHeader.BitPerPixel == 32) {	//This is in case the image is a 32-bit BMP file
				*(itrt++) = data[Count].Reserved;
			}
			Count++;
		}
		if (paddingBytes == 0)				//Check if the padding bytes equal with 0 to skip the next command line
			continue;
		for (unsigned long a = 0; a < paddingBytes; a++)			//Walkthrough the padding bytes to continue the saving data stuff
			*(itrt++) = 0;
	}
}

void ConvertTo8BitFile(char* destination, RGB*& rgb, img image) {
	img newImage;											//This will be our new location to save data of the converted file!
	newImage.header = image.header;							//The new image has the header information like the old one, except the data offset!
	newImage.header.DataOffset = 14 + 40 + 1024;			//1024 is the size of the color table that we need to put it on the file
	newImage.dibHeader = image.dibHeader;					//And the DIB header information different just with the size and the 'bit per pixel'
	newImage.dibHeader.Size = 40;							//8-bit BMP file just need 40-bytes-information in DIB header
	newImage.dibHeader.BitPerPixel = 8;						//Of course this is 8 bit/pixel :))

	//Next, let's create the Color Table version of the 8-bit BMP file ^^, with that we don't have in the 32-bit or 24-bit image version
	newImage.dibReserved = new char[256 * 4];
	for (int i = 0; i < 256; i++) {						//Each i we run through will make a 4-bytes data!
		for (int j = 0; j < 3; j++)
			newImage.dibReserved[i * 4 + j] = i;		//The first 3 of 4-bytes is equal to i, which increased from 0 to 255
		newImage.dibReserved[i * 4 + 3] = 0;			//The last of 4-bytes is equal to 0
	}
	//Still have the Image Size handmaded :(((
	long newPaddingBytes = (4 - (((newImage.dibHeader.BitPerPixel / 8) * newImage.dibHeader.Width) % 4)) % 4;			//padding bytes of the new pic
	long newSize = ((newImage.dibHeader.BitPerPixel / 8) * newImage.dibHeader.Width + newPaddingBytes) * newImage.dibHeader.Height;		//Size of new image's pixel data to save
	newImage.header.FileSize = newImage.header.DataOffset + newSize;						//Set up the new data
	newImage.dibHeader.ImageSize = newSize;

	//Let's have the Pixel Data
	unsigned char tempValue;			//This is created like an temporary memory to save the average value of each pixel point!
	for (unsigned long i = 0; i < image.dibHeader.Height * image.dibHeader.Width; i++) {
		tempValue = (unsigned char)((double)rgb[i].Blue + (double)rgb[i].Green + (double)rgb[i].Red) / 3;		//I pressed the type
		rgb[i].Blue = rgb[i].Green = rgb[i].Red = rgb[i].Reserved = tempValue;									//Make it all equal
	}
	WritePixelArray(newImage, rgb);					//Save the calculated pixel data into newImage

	//Now write down the bmp file with the data we've got in the location given
	writeBMPfile(destination, newImage);

	//And finally, remember to delete the array to free the heap memories
	unsigned long temp = image.header.DataOffset - 14 - sizeof(DIB_Header);			//this is the size of dibReserved
	if (temp != 0) {								//delete the dynamic array if exists
		delete[] newImage.dibReserved;
		newImage.dibReserved = NULL;
	}
	delete[] newImage.BitmapData;					//delete the dynamic array
	newImage.BitmapData = NULL;
}

RGB CalculateAveragePerBlock(RGB* data, int i, int j, int S, img image, unsigned long oldSize) {	//function to calculate the average value of an S*S blocks of pixel data
	RGB temp;
	int SumBlue = 0, SumGreen = 0, SumRed = 0, SumReserved = 0;
	unsigned long Count = i * image.dibHeader.Width + j;		//A value presented to the current position of the data to be equaled with
	for (int a = i; a < i + S; a++) {
		for (int b = j; b < j + S; b++) {
			if (Count >= oldSize)						//The size of data is oldSize, so this is written to avoid the violent access to unknown heap memories
				break;
			if (image.dibHeader.BitPerPixel >= 8)		//This is in case the image is a 8-bit BMP file
				SumBlue += data[Count].Blue;
			if (image.dibHeader.BitPerPixel >= 24) {	//This is in case the image is a 24-bit BMP file
				SumRed += data[Count].Red;
				SumGreen += data[Count].Green;
			}
			if (image.dibHeader.BitPerPixel == 32)		//This is in case the image is a 32-bit BMP file
				SumReserved += data[Count].Reserved;
			Count++;
		}
		Count += image.dibHeader.Width - S;					//Add this to run into the next coordinate
	}
	if (image.dibHeader.BitPerPixel >= 8)		//This is in case the image is a 8-bit BMP file
		temp.Blue = SumBlue / (S * S);
	if (image.dibHeader.BitPerPixel >= 24) {	//This is in case the image is a 24-bit BMP file
		temp.Red = SumRed / (S * S);
		temp.Green = SumGreen / (S * S);
	}
	if (image.dibHeader.BitPerPixel == 32)		//This is in case the image is a 32-bit BMP file
		temp.Reserved = SumReserved / (S * S);
	return temp;
}

void ReSizetoSmallerSize(char* destination, img image, int S, RGB*& data) {
	//Check S if it not equal with 1
	if (S != 1)
	{
		img newImage;							//This will be our new location to save data of the resize file!
		newImage.header = image.header;							//The new image has the header information like the old one
		newImage.dibHeader = image.dibHeader;					//And the DIB header information different just with the size and the width, height
		newImage.dibHeader.Height = image.dibHeader.Height / S;		//Resize the height
		newImage.dibHeader.Width = image.dibHeader.Width / S;		//Resize the width
		//If the size mod with S not equal with 0, then plus each size with one to add it up
		if (image.dibHeader.Width % S != 0)
			newImage.dibHeader.Width++;								
		if (image.dibHeader.Height % S != 0)
			newImage.dibHeader.Height++;

		unsigned long temp = image.header.DataOffset - 14 - sizeof(DIB_Header);			//this is the size of dibReserved
		if (temp != 0) {							//check if the dynamic array of the read image's dib reserved exists
			newImage.dibReserved = new char[temp];
			newImage.dibReserved = image.dibReserved;
		}
		unsigned long oldPaddingBytes = (4 - (((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width) % 4)) % 4;
		unsigned long oldSize = ((image.dibHeader.BitPerPixel / 8) * image.dibHeader.Width + oldPaddingBytes) * image.dibHeader.Height;
		int Count = 0;											//A value presented to the current position of the newData to be placed
		unsigned long paddingBytes = (4 - (((newImage.dibHeader.BitPerPixel / 8) * newImage.dibHeader.Width) % 4)) % 4;
		int newSize = ((newImage.dibHeader.BitPerPixel / 8) * newImage.dibHeader.Width + paddingBytes) * newImage.dibHeader.Height;		//Size of new image's pixel data to save
		RGB* newData = new RGB[newSize];				//New place to save the calculated pixel data

		for (unsigned int i = 0; i < image.dibHeader.Height; i += S) {
			for (unsigned int j = 0; j < image.dibHeader.Width; j += S) {
				RGB tempData = CalculateAveragePerBlock(data, i, j, S, image, oldSize);
				if (Count >= newSize)							//The size of newData is newSize, so this is written to avoid the violent access to unknown heap memories
					break;
				if (newImage.dibHeader.BitPerPixel >= 8)		//This is in case the image is a 8-bit BMP file
					newData[Count].Blue = tempData.Blue;
				if (newImage.dibHeader.BitPerPixel >= 24) {		//This is in case the image is a 24-bit BMP file
					newData[Count].Red = tempData.Red;
					newData[Count].Green = tempData.Green;
				}
				if (newImage.dibHeader.BitPerPixel == 32)		//This is in case the image is a 32-bit BMP file
					newData[Count].Reserved = tempData.Reserved;
				Count++;
			}
		}
		//Our job now is save the newData into the newImage.bitmapData and write the new BMP file into the given destination
		newImage.header.FileSize = image.header.DataOffset + newSize;
		newImage.dibHeader.ImageSize = newSize;
		WritePixelArray(newImage, newData);
		writeBMPfile(destination, newImage);

		delete[] newData;										//delete the dynamic array
		newData = NULL;
		if (temp != 0) {										//delete the dynamic array if exists
			delete[] newImage.dibReserved;
			newImage.dibReserved = NULL;
		}
		delete[] newImage.BitmapData;							//delete the dynamic array
		newImage.BitmapData = NULL;
	}
	else writeBMPfile(destination, image);						//if S = 1, this action is like reading BMP file and writing it down immediately with no action added
}