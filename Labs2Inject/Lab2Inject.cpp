#include "Lab2Inject.h"

extern int ConnectServ(std::string request);

BOOL Lab2SetPrivilege(HANDLE hToken, LPCTSTR szPrivName, BOOL fEnable) {
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    LookupPrivilegeValue(NULL, szPrivName, &tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
    return((GetLastError() == ERROR_SUCCESS));
}

DWORD Lab2GetProcessID_v2(LPCTSTR targetName)
{
    DWORD processID = 0;
    HANDLE snapHandle;
    PROCESSENTRY32 processEntry = { 0 };

    if ((snapHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE) {
        return 0;
    }

    processEntry.dwSize = sizeof(PROCESSENTRY32);
    Process32First(snapHandle, &processEntry);
    do {
        //printf("*%S*%S*\n", processEntry.szExeFile, targetName);
        if (wcscmp(processEntry.szExeFile, targetName) == 0) {
            return processEntry.th32ProcessID;
        }
    } while (Process32Next(snapHandle, &processEntry));

    if (snapHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(snapHandle);
    }

    return 0;
}

HANDLE Lab2_InjectDLLIntoProcess(DWORD procID, LPCWSTR dllName)
{
    HANDLE hToken = NULL;
    HANDLE hCurrentProc = GetCurrentProcess();
    HANDLE processHandel = NULL;
    LPVOID dll_name = 0;
    SIZE_T dwWritten = 0;
    LPVOID load_library_p = NULL;
    DWORD ThreadID = 0;
    HANDLE hThread = NULL;

    if (!OpenProcessToken(hCurrentProc, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)) 
    {
        LAB2_PRINT("OpenProcessToken Error 0x%x", GetLastError());
        goto COMPLETE;
    }
    else
    {
      /*  if (!Lab2SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) 
        {
            LAB2_PRINT("SetPrivlegesSE_DEBUG_NAME Error 0x%x", GetLastError());
            goto COMPLETE;
        }*/
    }
      
    processHandel = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);    
    LAB2_PRINT("OpenProcess 0x%x\n", GetLastError());
    dll_name = VirtualAllocEx(processHandel, NULL, MAX_PATH * sizeof(TCHAR), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    LAB2_PRINT("VirtualAllocEx 0x%x\n", GetLastError());

    if (dll_name == 0)
    {
        LAB2_PRINT("VirtualAllocEx error 0x%x\n", GetLastError());
        CloseHandle(processHandel);
        goto COMPLETE;
    }

    if (WriteProcessMemory(processHandel, dll_name, dllName, (lstrlen(dllName)+1)*sizeof(TCHAR), NULL) == 0) 
    {
        LAB2_PRINT("WriteProcessMemory error 0x%x\n", GetLastError());
        goto COMPLETE;
    }

#ifdef UNICODE
    load_library_p = GetProcAddress(LoadLibraryA("kernel32.dll"), "LoadLibraryW");
#else
    load_library_p = GetProcAddress(LoadLibrary(TEXT("kernel32.dll")), "LoadLibraryA");
#endif
    hThread = CreateRemoteThread(processHandel, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_p, dll_name, 0, &ThreadID);
    if (hThread == NULL) {
        LAB2_PRINT("Error creating thread 0x%x\n", GetLastError());
        goto COMPLETE;
    }

COMPLETE:
    if (hToken != NULL)
        CloseHandle(hToken);
    if (processHandel != NULL)
        CloseHandle(processHandel);

    return hThread;
}

int main(int argc, char*argv[])
{
    OutputDebugStringW(TEXT("CC"));
    TCHAR proc_name[NAMES_LEN] = { 0 };
    std::string func_or_filename; //name of monitoring func or hiding file
    LPCTSTR dll_name = TEXT("Lab2Hook.dll");
    TCHAR full_dll_path[MAX_PATH];
    DWORD pid = 0;
    HANDLE remote_tread_h;
    std::vector<std::string> input_data;
    if (argc < 4) {
        LAB2_PRINT("ERROR: To few arguments!");
        return __LINE__;
    }
    //retrieving command line parameters
    for (int i = 1; i < argc; i++) {
        input_data.push_back(argv[i]);
    } 

    memset(full_dll_path, 0, sizeof(full_dll_path));
    if (GetFullPathName(dll_name, sizeof(full_dll_path) / sizeof(TCHAR), full_dll_path, NULL) == 0)
    {
        LAB2_PRINT("ERROR: Unable to get full path of %S!\n", dll_name);
        return __LINE__;
    }

	LAB2_PRINT("Starting...");
    //If we search for a process by name - look for its pid
    if (input_data[0] == "-name") {
        LAB2_PRINT("Searching by process name");
        swprintf_s(proc_name, L"%S", (char*)input_data[1].c_str());
        pid = Lab2GetProcessID_v2(proc_name);
        if (pid == 0)
        {
            LAB2_PRINT("ERROR: Unable to find process %S.\n", proc_name);
            return __LINE__;
        }
    }
    else if (input_data[0] == "-pid") {
        LAB2_PRINT("Searching by process PID");
        pid = std::atoi(input_data[1].c_str());
    } else {
        LAB2_PRINT("ERROR: Wrong arguments!\n");
        return __LINE__;
    }
    //define the command to observe or hide
    if (input_data[2] == "-func") {
        func_or_filename = input_data[3]+" func";
        LAB2_PRINT("Function call monitoring");
    }
    else if (input_data[2] == "-hide") {
        func_or_filename = input_data[3]+" hide";   
        LAB2_PRINT("Filename hiding");
    }
    else {
        LAB2_PRINT("ERROR: Wrong arguments!\n");
        return __LINE__;
    }
    remote_tread_h = Lab2_InjectDLLIntoProcess(pid, full_dll_path);
    if (remote_tread_h == NULL)
    {
        LAB2_PRINT("ERROR: Unable to inject DLL to process %S.\n", proc_name);
        return __LINE__;
    }

    LAB2_PRINT("Injection to %S success!\n", proc_name);
    ConnectServ(func_or_filename); //pipe connect
    LAB2_PRINT("Completed!\n");
    system("pause");
    return 0;
}
