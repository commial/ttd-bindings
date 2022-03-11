#pragma once
#include <windows.h> 
#include <stdio.h>
#include <iostream>

char* base64_encode(const unsigned char* data,
	size_t input_length,
	size_t* output_length);

void DumpHex(const void* data, size_t size);