#pragma once
#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_WARNINGS 
#include <cstdint>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cassert>
#include <cmath>

typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint8_t uint8;
typedef const char* cstring;

bool compare(uint8 arr[], cstring string, int size);
inline std::vector<uint8_t> read_file(cstring path);

/**
* Structs for storing WAV information
* Information of WAVE RIFF header format
* Information on this format can be found @ http://soundfile.sapp.org/doc/WaveFormat/
*/
struct RIFF_Header
{
	uint8 chunckId[4];
	int32 chunckSize;
	uint8 format[4];
};

struct WAVE_Format
{
	uint8 subChunckId[4];
	int32 subchunckSize;
	int16 audioFormat;
	int16 channels;
	int32 sampleRate;
	int32 byteRate;
	int16 blockAlign;
	int16 bitsPerSample;
};

struct WAVE_Data 
{
	uint8 subChunckId[4];
	int32 subChunck2Size;
};

void load_wav(cstring filePath, unsigned char** data, int* size, int* freq)
{
	WAVE_Format wave_format;
	RIFF_Header riff_header;
	WAVE_Data wave_data;
	std::vector<uint8> fileData;

	fileData = read_file(filePath);

	riff_header.chunckId[0] = fileData[0];
	riff_header.chunckId[1] = fileData[1];
	riff_header.chunckId[2] = fileData[2];
	riff_header.chunckId[3] = fileData[3];
	if (compare(riff_header.chunckId, "RIFF", 4))
	{
		std::cout << "Valid RIFF" << std::endl;
	}
	else return;

	riff_header.format[0] = fileData[8];
	riff_header.format[1] = fileData[9];
	riff_header.format[2] = fileData[10];
	riff_header.format[3] = fileData[11];
	if (compare(riff_header.format, "WAVE", 4))
	{
		std::cout << "Valid WAVE" << std::endl;
	}
	else return;

	wave_format.subChunckId[0] = fileData[12];
	wave_format.subChunckId[1] = fileData[13];
	wave_format.subChunckId[2] = fileData[14];
	wave_format.subChunckId[3] = fileData[15];
	if (compare(wave_format.subChunckId, "fmt ", 4))
	{
		std::cout << "Valid fmt " << std::endl;
	}
	else return;

	memcpy(&wave_format.subchunckSize, (void*)&fileData[16], 4);
	memcpy(&wave_format.audioFormat, (void*)&fileData[20], 2);
	memcpy(&wave_format.channels, (void*)&fileData[22], 2);
	memcpy(&wave_format.sampleRate, (void*)&fileData[24], 4);
	memcpy(&wave_format.byteRate, (void*)&fileData[28], 4);
	memcpy(&wave_format.blockAlign, (void*)&fileData[32], 2);
	memcpy(&wave_format.bitsPerSample, (void*)&fileData[34], 2);
	std::cout << "Chuncksize: " << wave_format.subchunckSize << std::endl;
	std::cout << "Audio format: " << wave_format.audioFormat << std::endl;
	std::cout << "Channels: " << wave_format.channels << std::endl;
	std::cout << "sampleRate: " << wave_format.sampleRate << std::endl;
	std::cout << "byteRate: " << wave_format.byteRate << std::endl;
	std::cout << "blockAlign: " << wave_format.blockAlign << std::endl;
	std::cout << "bitsPerSample: " << wave_format.bitsPerSample << std::endl;

	wave_data.subChunckId[0] = fileData[36];
	wave_data.subChunckId[1] = fileData[37];
	wave_data.subChunckId[2] = fileData[38];
	wave_data.subChunckId[3] = fileData[39];
	if (compare(wave_data.subChunckId, "data", 4))
	{
		std::cout << "Valid data" << std::endl;
	}
	else return;

	memcpy(&wave_data.subChunck2Size, (void*)&fileData[40], 4);
	std::cout << "subChunck2Size: " << wave_data.subChunck2Size << std::endl;

	int dataSize = (fileData.size() - 44);
	std::cout << "mono data size: " << dataSize << std::endl;
	std::cout << "stereo data size: " << fileData.size()-44 << std::endl;
	/*unsigned char* left = new unsigned char[dataSize];
	memcpy(left, (void*)&fileData[44], dataSize);
	unsigned char* right = new unsigned char[dataSize];
	memcpy(right, (void*)&fileData[dataSize + 44], dataSize);*/

	unsigned char* mono = new unsigned char[dataSize/2];
	int j = 0;
	for (int i = 0; i < dataSize; i += 2)
	{
		int left = fileData[44 + i];
		int right = fileData[44 + i+1];
		char add = (left + right) / 2; // add the two volumes
		mono[j] = add;
		++j;
	}

	*size = dataSize / 2;// (fileData.size() - 44);
	*data = new unsigned char[*size];
	memcpy(*data, mono, dataSize/2);

	*freq = wave_format.sampleRate;
	std::cout << "Loaded audio file!" << std::endl;
}

bool compare(uint8 arr[], cstring string, int size)
{
	for (int i = 0; i < size; ++i)
	{
		if (arr[i] != string[i]) return false;
	}
	return true;
}

inline std::vector<uint8_t> read_file(cstring path)
{
	//create managed FILE ptr
	const std::unique_ptr<FILE, decltype(&fclose)> file(
		fopen(std::string(path).c_str(), "r+b"),
		fclose // deleter is fclose
	);

	assert("could not open file", file);

	//get size and create container of that size
	fseek(file.get(), 0L, SEEK_END);
	std::vector<uint8_t> container(ftell(file.get()));
	fseek(file.get(), 0L, SEEK_SET);

	//read file 
	fread(container.data(), sizeof(unsigned char), container.size(), file.get());

	return container;
}