#pragma once
#include <psapi.h>
#include <TlHelp32.h>
#include <iostream>
using namespace std;

bool get_process_name(IN HANDLE hProcess, OUT LPWSTR nameBuf, IN SIZE_T nameMax)
{
    memset(nameBuf, 0, nameMax);
    DWORD out = GetModuleBaseName(hProcess, 0, nameBuf, nameMax);
    if (out == 0) {
        out = GetProcessImageFileName(hProcess, nameBuf, nameMax);
    }
    return (out > 0);
}

bool is_searched_process(DWORD processID, LPWSTR searchedName, bool is64b)
{
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
    if (hProcess == NULL) return false;
    bool proc64b = !is_system32b() && !is_wow64(hProcess);
    if (is64b != proc64b) {
        CloseHandle(hProcess);
        return false;
    }
    WCHAR szProcessName[MAX_PATH];
    if (get_process_name(hProcess, szProcessName, MAX_PATH)) {
        
        if (wcsstr(szProcessName, searchedName) != NULL) {
            printf("%S  (PID: %u) : %d\n", szProcessName, processID, proc64b);
            CloseHandle(hProcess);
            return true;   
        }
   }
    CloseHandle(hProcess);
    return false;
}

HANDLE find_running_process(LPWSTR searchedName)
{
    bool is64b = !is_compiled_32b();

    HANDLE hProcessSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
    PROCESSENTRY32 process_entry = { 0 };
    process_entry.dwSize = sizeof(process_entry);

    if (!Process32First(hProcessSnapShot, &process_entry)) {
        return NULL;
    }

    do
    {
        if (is_searched_process(process_entry.th32ProcessID, searchedName, is64b)) {
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_entry.th32ProcessID);
            return hProcess;
        }

    } while (Process32Next(hProcessSnapShot, &process_entry));

    // Close the handle
    CloseHandle(hProcessSnapShot);
    return NULL;
}


HANDLE find_running_process2(LPWSTR searchedName)
{
    bool is64b = !is_compiled_32b();
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return NULL;
    }

    //calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    //search handle to the process of defined name
    for ( i = 0; i < cProcesses; i++ ) {
        if (aProcesses[i] != 0) {
            if (is_searched_process(aProcesses[i], searchedName, is64b)) {
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, aProcesses[i]);
                return hProcess;
            }
        }
    }
    return NULL;
}