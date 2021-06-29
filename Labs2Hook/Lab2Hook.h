#pragma once

//Shared stuff
#include <windows.h>
#include <stdio.h>
#include <detours.h>

#include <string>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PRINT_TEXT_LENGTH 1024
#define BUFSIZE 512
#define MAX_FUNC_NAME 1024
#define DATA_BUFSIZE 50
//#define FUNC_TO_HOOK "CreateFileW"
#define DLL_TO_HOOK TEXT("kernel32.dll")


void Lab2DebugOut(const char* fmt, ...);

#define LAB2_PRINT(a, ...) Lab2DebugOut("\nLABS2 HOOK : " a, ##__VA_ARGS__)
