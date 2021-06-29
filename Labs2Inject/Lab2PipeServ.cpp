#include "Lab2Inject.h"


LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

//server pipe
DWORD WINAPI InstanceThread(LPVOID, std::string);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD);

int ConnectServ(std::string request)
{
    BOOL   fConnected = FALSE;
    DWORD  dwThreadId = 0;
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;

    for (;;)
    {
        LAB2_PRINT(" Pipe Ser: Main thread awaiting client connection on %s\n", lpszPipename);
        hPipe = CreateNamedPipe(
            lpszPipename,             // pipe name 
            PIPE_ACCESS_DUPLEX,       // read/write access 
            PIPE_TYPE_MESSAGE |       // message type pipe 
            PIPE_READMODE_MESSAGE |   // message-read mode 
            PIPE_WAIT,                // blocking mode 
            PIPE_UNLIMITED_INSTANCES, // max. instances  
            BUFSIZE,                  // output buffer size 
            BUFSIZE,                  // input buffer size 
            0,                        // client time-out 
            NULL);                    // default security attribute 

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            LAB2_PRINT("CreateNamedPipe failed, GLE=%d.\n", GetLastError());
            return -1;
        }


        fConnected = ConnectNamedPipe(hPipe, NULL) ?
            TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected)
        {
            printf("Client connected, creating a processing thread.\n");

            // Create a thread for this client. 

            InstanceThread((LPVOID)hPipe, request);

            if (hThread == NULL)
            {
                LAB2_PRINT("CreateThread failed, GLE=%d.\n", GetLastError());
                return -1;
            }
            else CloseHandle(hThread);
        }
        else
            // The client could not connect, so close the pipe. 
            CloseHandle(hPipe);
    }

    return 0;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam, std::string request)
{
    HANDLE hHeap = GetProcessHeap();
    TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
    TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));

    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    HANDLE hPipe = NULL;

    if (lpvParam == NULL)
    {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
        printf("   InstanceThread exitting.\n");
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    if (pchRequest == NULL)
    {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL heap allocation.\n");
        printf("   InstanceThread exitting.\n");
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
        return (DWORD)-1;
    }

    if (pchReply == NULL)
    {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL heap allocation.\n");
        printf("   InstanceThread exitting.\n");
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    // Print verbose messages. In production code, this should be for debugging only.
    printf("InstanceThread created, receiving and processing messages.\n");

    // The thread's parameter is a handle to a pipe object instance. 

    hPipe = (HANDLE)lpvParam;

    // Loop until done reading
    while (1)
    {
        // Read client requests from the pipe. This simplistic code only allows messages
        // up to BUFSIZE characters in length.
        fSuccess = ReadFile(
            hPipe,        // handle to pipe 
            pchRequest,    // buffer to receive data 
            BUFSIZE * sizeof(TCHAR), // size of buffer 
            &cbBytesRead, // number of bytes read 
            NULL);        // not overlapped I/O 

        if (!fSuccess || cbBytesRead == 0)
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                LAB2_PRINT("InstanceThread: client disconnected.\n");
            }
            else
            {
                LAB2_PRINT("InstanceThread ReadFile failed, GLE=%d.\n", GetLastError());
            }
            break;
        }

        // Process the incoming message.
        GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes);
        std::string HookReq(request);
        // Write the reply to the pipe. 
        fSuccess = WriteFile(
            hPipe,        // handle to pipe 
            request.c_str(),     // buffer to write from 
            cbReplyBytes, // number of bytes to write 
            &cbWritten,   // number of bytes written 
            NULL);        // not overlapped I/O 

        if (!fSuccess || cbReplyBytes != cbWritten)
        {
            LAB2_PRINT("InstanceThread WriteFile failed, GLE=%d.\n", GetLastError());
            break;
        }
    }

    // Flush the pipe


    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    HeapFree(hHeap, 0, pchRequest);
    HeapFree(hHeap, 0, pchReply);

    return 1;
}

VOID GetAnswerToRequest(LPTSTR pchRequest,
    LPTSTR pchReply,
    LPDWORD pchBytes)
    // This routine is a simple function to print the client request to the console
    // and populate the reply buffer with a default data string. This is where you
    // would put the actual client request processing code that runs in the context
    // of an instance thread. Keep in mind the main thread will continue to wait for
    // and receive other client connections while the instance thread is working.
{
    LAB2_PRINT("Client Request String:\"%s\"\n", pchRequest);

    // Check the outgoing message to make sure it's not too long for the buffer.
    if (FAILED(StringCchCopy(pchReply, BUFSIZE, TEXT("default answer from server"))))
    {
        *pchBytes = 0;
        pchReply[0] = 0;
        printf("StringCchCopy failed, no outgoing message.\n");
        return;
    }
    *pchBytes = (lstrlen(pchReply) + 1) * sizeof(TCHAR);
}
//server pipe
