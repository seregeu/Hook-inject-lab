#include "Lab2Hook.h"
//#include "Labs2PipeCli.cpp"

extern int ConnectCli(void);
extern HANDLE hPipe;
char* token1;
std::string	FullPath;
std::string	Path;
std::string	FileName;
std::wstring wFull_Path;
CHAR DataDate[DATA_BUFSIZE];
CHAR Timer[DATA_BUFSIZE];
DWORD	     NumberWriteBytes;
CHAR		 FunctionName[BUFSIZE];

HANDLE(WINAPI* pCreateFileA) (LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileA;
HANDLE(WINAPI* pCreateFileW) (LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileW;
HANDLE(WINAPI* pFindFirstFileW) (LPCWSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData) = FindFirstFileW;
HANDLE(WINAPI* pFindFirstFileA)(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) = FindFirstFileA;
BOOL(WINAPI* pFindNextFileW) (HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData) = FindNextFileW;
BOOL(WINAPI* pFindNextFileA) (HANDLE hFindFile, LPWIN32_FIND_DATAA  lpFindFileData) = FindNextFileA;
HANDLE(WINAPI* pFindFirstFileExA) (LPCSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS  fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags) = FindFirstFileExA;
HANDLE(WINAPI* pFindFirstFileExW) (LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS  fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags) = FindFirstFileExW;

HANDLE WINAPI CreateFileA_hide(LPCSTR a1, DWORD a2, DWORD a3, LPSECURITY_ATTRIBUTES a4, DWORD a5, DWORD a6, HANDLE a7)
{
    if (a1 == FullPath)
    {
        return INVALID_HANDLE_VALUE;
    }
    return pCreateFileA(a1, a2, a3, a4, a5, a6, a7);
}

HANDLE WINAPI CreateFileW_hide(LPCWSTR a1, DWORD a2, DWORD a3, LPSECURITY_ATTRIBUTES a4, DWORD a5, DWORD a6, HANDLE a7)
{
    if (a1 == wFull_Path)
    {
        return INVALID_HANDLE_VALUE;
    }
    return pCreateFileW(a1, a2, a3, a4, a5, a6, a7);
}

HANDLE WINAPI FindFirstFileA_hide(LPCSTR a1, LPWIN32_FIND_DATAA a2)
{
    if (a1 == FullPath)
    {
        return INVALID_HANDLE_VALUE;
    }
    return pFindFirstFileA(a1, a2);
}

HANDLE WINAPI FindFirstFileW_hide(LPCWSTR a1, LPWIN32_FIND_DATA a2)
{
    if (a1 == wFull_Path)
    {
        return INVALID_HANDLE_VALUE;
    }
    return pFindFirstFileW(a1, a2);
}

BOOL WINAPI FindNextFileA_hide(HANDLE a1, LPWIN32_FIND_DATAA a2)
{
    bool ret = pFindNextFileA(a1, a2);
    if (a2->cFileName == FullPath)
    {
        ret = pFindNextFileA(a1, a2);
    }
    return ret;
}

BOOL WINAPI FindNextFileW_hide(HANDLE a1, LPWIN32_FIND_DATAW a2)
{
    bool ret = pFindNextFileW(a1, a2);
    if (a2->cFileName == wFull_Path)
    {
        ret = pFindNextFileW(a1, a2);
    }
    return ret;
}

HANDLE FindFirstFileExW_hide(LPCWSTR a0, FINDEX_INFO_LEVELS a1, LPWIN32_FIND_DATAW a2, FINDEX_SEARCH_OPS a3, LPVOID a4, DWORD a5)
{
    HANDLE ret = pFindFirstFileExW(a0, a1, a2, a3, a4, a5);
    if (a2->cFileName == wFull_Path)
    {
        ret = INVALID_HANDLE_VALUE;
    }
    return ret;
}

HANDLE FindFirstFileExA_hide(LPCSTR a0, FINDEX_INFO_LEVELS a1, LPWIN32_FIND_DATAA a2, FINDEX_SEARCH_OPS a3, LPVOID a4, DWORD a5)
{
    HANDLE ret = pFindFirstFileExA(a0, a1, a2, a3, a4, a5);
    if (a2->cFileName == FullPath)
    {
        pFindNextFileA(ret, a2);
    }
    return ret;
}


int HideFile(std::string& fileName)
{
    size_t pos_slash = fileName.rfind('\\')+1;

    FullPath = fileName;
    Path = FullPath.substr(0, pos_slash);
    FileName = FullPath.substr(pos_slash, FullPath.length());
    wFull_Path = std::wstring(FullPath.begin(), FullPath.end());

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pCreateFileA, CreateFileA_hide);
    LONG err = DetourTransactionCommit();
    if (err != NO_ERROR)
        return -1;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pCreateFileW, CreateFileW_hide);
    err = DetourTransactionCommit();
    if (err != NO_ERROR)
        return -1;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pFindFirstFileW, FindFirstFileW_hide);
    err = DetourTransactionCommit();
    if (err != NO_ERROR)
        return -1;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pFindFirstFileA, FindFirstFileA_hide);
    err = DetourTransactionCommit();
    if (err != NO_ERROR)
        return -1;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pFindNextFileW, FindNextFileW_hide);
    err = DetourTransactionCommit();
    if (err != NO_ERROR)
        return -1;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pFindNextFileA, FindNextFileA_hide);
    err = DetourTransactionCommit();
    if (err != NO_ERROR)
        return -1;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pFindFirstFileExW, FindFirstFileExW_hide);
    err = DetourTransactionCommit();
    if (err != NO_ERROR)
        return -1;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)pFindFirstFileExA, FindFirstFileExA_hide);
    err = DetourTransactionCommit();
    if (err != NO_ERROR)
        return -1;

    return 0;
}


void Lab2DebugOut(const char* fmt, ...) 
{
    char s[MAX_PRINT_TEXT_LENGTH];
    va_list args;
    va_start(args, fmt);
    vsnprintf(s, MAX_PRINT_TEXT_LENGTH, fmt, args);
    OutputDebugStringA(s);
    va_end(args);
}


CHAR gLab2FuncName[MAX_FUNC_NAME];
CHAR gBuffer[BUFSIZE];
BOOLEAN gLab2Hooked = FALSE;
extern "C" LPVOID gLab2OrigPoin = NULL;

extern "C" void Lab2Hook();

BOOLEAN Labs2CreateHook()
{
    if (gLab2OrigPoin == NULL || gLab2Hooked == FALSE)
    {
        gLab2OrigPoin = GetProcAddress(LoadLibrary(DLL_TO_HOOK), gLab2FuncName);
        if (gLab2OrigPoin == 0)
        {
            LAB2_PRINT("ERROR: Unable to get proc address %s!\n", gLab2FuncName);
            return FALSE;
        }
        gLab2Hooked = TRUE;


        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)gLab2OrigPoin, Lab2Hook);
        LONG err = DetourTransactionCommit();
        if (err == NO_ERROR)
        {
            LAB2_PRINT("%s() detoured successfully\n", gLab2FuncName);
        }
        else
        {
            LAB2_PRINT("ERROR 0x%x: %s() detoured failed\n", err, gLab2FuncName);
            return FALSE;
        }
    }
}




extern "C" VOID Lab2HookCallback()
{

    CHAR MessageToSend[BUFSIZE];
    SYSTEMTIME SysTime;
    DWORD Count_written_bytes;

    GetLocalTime(&SysTime);
    sprintf_s(DataDate, "%02d:%02d:%02d", SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
    if (Timer == NULL || strcmp(DataDate, Timer))
    {
        sprintf_s(Timer, "%02d:%02d:%02d", SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
        sprintf_s(MessageToSend, "%s()\tTime: %s", FunctionName, Timer);
        NumberWriteBytes = (lstrlenA(MessageToSend) + 1) * sizeof(CHAR);
        WriteFile(hPipe, MessageToSend, NumberWriteBytes, &Count_written_bytes, NULL);
    }

}


BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved)  // reserved
{
    //printf("AA");
    //OutputDebugStringA("AA");
    bool result = false;
    LPCSTR message = ("Hooking DLL is in business!");
    DWORD count_write_bytes = (lstrlenA(message) + 1) * sizeof(CHAR);
    DWORD Count_written_bytes;
    DWORD Count_read_bytes;

    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        if (!ConnectCli()) {
            //Successfully connected to server
            result = WriteFile(hPipe, message, count_write_bytes, &Count_written_bytes, NULL);
            if (!result)
            {
                printf("ERROR: WriteFile pipe %d\n", GetLastError());
                return -1;
            }

            do {
                result = ReadFile(hPipe, gBuffer, BUFSIZE * sizeof(CHAR), &Count_read_bytes, NULL);
                if (!result && GetLastError() != ERROR_MORE_DATA)
                    break;
            } while (!result);
            //now we got the message, next we should make an action from command
            char* func_to_hook = NULL;
            char* comand = NULL;
            func_to_hook = strtok_s(gBuffer, " ", &token1);
            comand = strtok_s(NULL, " ", &token1);
            if (!strcmp(comand, "func")) {

            }
            else if (!strcmp(comand, "hide")) {
                std::string HideName(func_to_hook);
                HideFile(HideName);
            }
            else {
                break;
            }
            strncpy_s(gLab2FuncName, func_to_hook, MAX_FUNC_NAME);
            // Initialize once for each new process.
            // Return FALSE to fail DLL load.
            DisableThreadLibraryCalls(hinstDLL);
            Labs2CreateHook();
            LAB2_PRINT("Procces attach...");
        }
        break;
    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        LAB2_PRINT("Thread attach...");
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        LAB2_PRINT("Thread detach...");

        break;

    case DLL_PROCESS_DETACH:
        // Perform any necessary cleanup.
        LAB2_PRINT("Procces detach...");
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

