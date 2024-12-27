#include <Windows.h>
#include <Shlwapi.h>
#include <iostream>
#include <Psapi.h>
#include <tchar.h>
#include <string>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "psapi.lib")

DWORD GetProcessIdByName(const wchar_t* processName) {
    DWORD processList[1024], processCount, processId = 0;

    // Get the list of all process IDs
    if (!EnumProcesses(processList, sizeof(processList), &processCount)) {
        std::cerr << "Failed to enumerate processes." << std::endl;
        return 0;
    }

    processCount /= sizeof(DWORD);  // Number of processes

    for (unsigned int i = 0; i < processCount; i++) {
        DWORD pid = processList[i];
        if (pid == 0) continue;

        // Open the process to check its name
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            TCHAR processPath[MAX_PATH];
            if (GetModuleFileNameEx(hProcess, NULL, processPath, MAX_PATH)) {
                // Extract the filename from the full path
                wchar_t* fileName = wcsrchr(processPath, L'\\');
                if (fileName) {
                    fileName++;  // Move pointer to start of the filename (after the last '\\')
                }
                else {
                    fileName = processPath;  // In case there is no '\\' (unlikely)
                }

                // Check if the process name matches
                if (_wcsicmp(fileName, processName) == 0) {
                    processId = pid;
                    CloseHandle(hProcess);
                    break;
                }
            }
            CloseHandle(hProcess);
        }
    }

    return processId;  // Return the process ID or 0 if not found
}

int main() {
    // Path to the DLL that creates the widgets
    TCHAR szLibPath[_MAX_PATH];
    GetModuleFileName(GetModuleHandle(NULL), szLibPath, _MAX_PATH);
    PathRemoveFileSpec(szLibPath);
    wcscat_s(szLibPath, _MAX_PATH, L"\\LUEdll.dll");

    // Target process for DLL injection (e.g., dwm.exe or explorer.exe)
    LPCWSTR targetProcess = L"explorer.exe";  // Adjust as necessary

    DWORD pid = GetProcessIdByName(L"explorer.exe");
    if (pid == 0) {
        std::cout << "Process not found!" << std::endl;
    }
    else {
        std::cout << "Found process ID: " << pid << std::endl;
    }

    // Open the target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetProcessIdByName(targetProcess));
    if (!hProcess) {
        std::cerr << "Failed to open target process." << std::endl;
        return 1;
    }

    // Allocate memory in the target process for the DLL path
    LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, wcslen(szLibPath) * sizeof(TCHAR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pDllPath) {
        std::cerr << "Failed to allocate memory in the target process." << std::endl;
        return 1;
    }

    // Write the DLL path into the target process's memory
    WriteProcessMemory(hProcess, pDllPath, szLibPath, wcslen(szLibPath) * sizeof(TCHAR), NULL);

    // Get the address of LoadLibraryW
    FARPROC pLoadLibraryW = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

    // Create a remote thread to load the DLL in the target process
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pDllPath, 0, NULL);
    if (!hThread) {
        std::cerr << "Failed to create remote thread." << std::endl;
        return 1;
    }

    // Wait for the thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return 0;
}