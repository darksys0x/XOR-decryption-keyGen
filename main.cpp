#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "b64.h"
#include <vector>
#include <cstring>
#include <cstdint>
#define NOMINMAX
#include<Windows.h>
#include <assert.h>
#include <algorithm>
#include <string>
#include<stdexcept>

//int
//main(void) {
//	const char* str = "brian the monkey and bradley the kinkajou are friends";
//	char* enc = b64_encode((unsigned char*)str, strlen(str));
//
//	printf("%s\n", enc); // YnJpYW4gdGhlIG1vbmtleSBhbmQgYnJhZGxleSB0aGUga2lua2Fqb3UgYXJlIGZyaWVuZHM=
//
//	char* dec = (char*)b64_decode((const char*)enc, strlen(enc));
//
//	printf("%s\n", dec); // brian the monkey and bradley the kinkajou are friends
//	free(enc);
//	free(dec);
//	return 0;
//}

//1. Decode the first base64 and the result is UTF - 8 binary(2 bytes = 1 character)
//2. Convert UTF - 8 binary to UTF - 16 string
//3. Take the UTF - 16 string and decrypt it with the XOR function
//4. Decode the UTF - 16 string as base64 and you get PE file


__declspec(noinline) char* ReadInputFile() {
	FILE* hamadFile = fopen("C:\\Users\\DFIR\\Documents\\samples\\mShell\\brutforce-shell\\x64\\Release\\input.txt", "rb");
	//FILE* hamadFile = fopen("C:\\Users\\DFIR\\Documents\\samples\\mShell\\brutforce-shell\\x64\\Release\\input2.txt", "rb");
	if (hamadFile) {
		fseek(hamadFile, 0, SEEK_END);
		long fileSize = ftell(hamadFile);
		fseek(hamadFile, 0, SEEK_SET);
		char* data = (char*)malloc(fileSize);
		if (!data)
			return nullptr;
		size_t bytesRead = fread(data, 1, fileSize, hamadFile);
		if (bytesRead != fileSize) {
			printf("fread: failed to read file | bytesRead = %u | fileSize = %u\n", bytesRead, fileSize);
			return nullptr;
		}
		fclose(hamadFile);
		return data;
	}
	printf("failed to open file\n");
	return nullptr;
}

__declspec(noinline) std::wstring utf8_to_utf16(const std::string& utf8)
{
	std::vector<unsigned long> unicode;
	size_t i = 0;
	while (i < utf8.size())
	{
		unsigned long uni;
		size_t todo;
		bool error = false;
		unsigned char ch = utf8[i++];
		if (ch <= 0x7F)
		{
			uni = ch;
			todo = 0;
		}
		else if (ch <= 0xBF)
		{
			throw std::logic_error("not a UTF-8 string");
		}
		else if (ch <= 0xDF)
		{
			uni = ch & 0x1F;
			todo = 1;
		}
		else if (ch <= 0xEF)
		{
			uni = ch & 0x0F;
			todo = 2;
		}
		else if (ch <= 0xF7)
		{
			uni = ch & 0x07;
			todo = 3;
		}
		else
		{
			throw std::logic_error("not a UTF-8 string");
		}
		for (size_t j = 0; j < todo; ++j)
		{
			if (i == utf8.size())
				throw std::logic_error("not a UTF-8 string");
			unsigned char ch = utf8[i++];
			if (ch < 0x80 || ch > 0xBF)
				throw std::logic_error("not a UTF-8 string");
			uni <<= 6;
			uni += ch & 0x3F;
		}
		if (uni >= 0xD800 && uni <= 0xDFFF)
			throw std::logic_error("not a UTF-8 string");
		if (uni > 0x10FFFF)
			throw std::logic_error("not a UTF-8 string");
		unicode.push_back(uni);
	}
	std::wstring utf16;
	for (size_t i = 0; i < unicode.size(); ++i)
	{
		unsigned long uni = unicode[i];
		if (uni <= 0xFFFF)
		{
			utf16 += (wchar_t)uni;
		}
		else
		{
			uni -= 0x10000;
			utf16 += (wchar_t)((uni >> 10) + 0xD800);
			utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
		}
	}
	return utf16;
}

__declspec(noinline) void ConvertUTF8ToUTF16(uint8_t* bytes, size_t size) {
	std::string str;
	str.resize(size);
	for (int i = 0; i < size; i++) {
		str[i] = bytes[i];
	}
	std::wstring utf16String = utf8_to_utf16(str);
	uint8_t* data = (uint8_t*)utf16String.data();
	for (int i = 0; i < size; i++) {
		bytes[i] = data[i];
	}
	//printf("Converted to UTF16 bytes = ");
	//PrintBytesArray(bytes, 32);
}

int main() {
	char* firstBase64String = ReadInputFile();
	if (!firstBase64String)
		return 0;
	size_t decodedSize = 0;//stack 
	// lea r8, [ebp-0x10]
	char* hamad_utf8= (char*)b64_decode_ex((const char*)firstBase64String, strlen(firstBase64String), &decodedSize);// decode b64 to binary utf-8
	/*mov rcx, [ebp - 8];
	call strlen;
	mov rdx, rax;
	lea r8, [ebp - 0x10];
	call b64_decode_ex;
	mov rsi, [ebp - 0x10]*/


	uint8_t* bytes = (uint8_t * )hamad_utf8;
	printf("%x\n", bytes[0]);
	ConvertUTF8ToUTF16((uint8_t*)hamad_utf8,  decodedSize);
	



	return 0;
}