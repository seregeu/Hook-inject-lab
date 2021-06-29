#pragma once
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <debugapi.h>
#include <vector>
//#include <string>
#include <iostream>
#include <tchar.h>
#include <strsafe.h>

#define MAX_PRINT_TEXT_LENGTH 1024
#define NAMES_LEN 50
#define BUFSIZE 512


void Lab2DebugOut(const char* fmt, ...);
#define LAB2_PRINT(a, ...) printf("\nLAB2 INJECT : " a, ##__VA_ARGS__); fflush(stdout);