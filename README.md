# Hook-inject-lab
The program can intercept the tracking of calls to another program and intercept them
The program uses the Hook & Inject technique, which is to force the target process to load the specified DLL
A program and dll library have been implemented that allow:
1. Track the call of predefined functions. Each function call in the target process must be accompanied by printing a message to the console of the program being developed. The printed message should contain the function name and timestamp.
2. Change the behavior of the FindFirstFile, FindNextFile, CreateFile functions in such a way that the file specified in advance disappears for the process. To change the behavior, it is enough to change the values ​​returned by these functions in accordance with certain input parameters.
The program must be compiled by Visual Studio.
The program offers the following actions:
a. –Pid <Target_Process_PID> - the target process is selected by the PID value.
b. –Name <Target_Process_Name> —The target process is selected by name.
c. –Func <Function_Name> - the name of the function whose call you want to track.
d. –Hide <File_Name> - name of the file to be hidden from the process.
3. It is believed that the following application startup scripts are available:
a. Monitor.exe –pid 123 –func CreateFile
b. Monitor.exe –name explorer.exe –func CreateFile
c. Monitor.exe –name explorer.exe –hide “C: \ hello.txt”
