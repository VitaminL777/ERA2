extern "C"{

BOOL
WINAPI
EnumProcesses(
  DWORD *lpidProcess,  // array of process identifiers
  DWORD cb,            // size of array
  DWORD *cbNeeded      // number of bytes returned
);

BOOL
WINAPI
EnumProcessModules(
  HANDLE hProcess,      // handle to process
  HMODULE *lphModule,   // array of module handles
  DWORD cb,             // size of array
  LPDWORD lpcbNeeded    // number of bytes required
);

#ifdef UNICODE
#define GetModuleFileNameEx GetModuleFileNameExW
#else
#define GetModuleFileNameEx  GetModuleFileNameExA
#endif // !UNICODE

DWORD
WINAPI 
GetModuleFileNameExA(
  HANDLE hProcess,    // handle to process
  HMODULE hModule,    // handle to module
  LPSTR lpFilename,  // path buffer
  DWORD nSize         // maximum characters to retrieve
);

DWORD
WINAPI 
GetModuleFileNameExW(
  HANDLE hProcess,    // handle to process
  HMODULE hModule,    // handle to module
  LPWSTR lpFilename,  // path buffer
  DWORD nSize         // maximum characters to retrieve
);

#ifdef UNICODE
#define GetModuleBaseName GetModuleFileNameExW
#else
#define GetModuleBaseName  GetModuleBaseNameA
#endif // !UNICODE


DWORD 
WINAPI
GetModuleBaseNameA(
  HANDLE hProcess,    // handle to process
  HMODULE hModule,    // handle to module
  LPSTR lpBaseName,  // base name buffer
  DWORD nSize         // maximum characters to retrieve
);

DWORD 
WINAPI
GetModuleBaseNameW(
  HANDLE hProcess,    // handle to process
  HMODULE hModule,    // handle to module
  LPWSTR lpBaseName,  // base name buffer
  DWORD nSize         // maximum characters to retrieve
);

typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;

BOOL 
WINAPI
GetModuleInformation(
  HANDLE hProcess,         // handle to process
  HMODULE hModule,         // handle to module
  LPMODULEINFO lpmodinfo,  // information buffer
  DWORD cb                 // size of buffer
);

#ifdef UNICODE
#define GetMappedFileName GetMappedFileNameW
#else
#define GetMappedFileName GetMappedFileNameA
#endif // !UNICODE

DWORD
WINAPI
GetMappedFileNameW(
  HANDLE hProcess,    // handle to process
  LPVOID lpv,         // address to verify
  LPWSTR lpFilename,  // file name buffer
  DWORD nSize         // size of buffer
);

DWORD 
WINAPI
GetMappedFileNameA(
  HANDLE hProcess,    // handle to process
  LPVOID lpv,         // address to verify
  LPSTR lpFilename,  // file name buffer
  DWORD nSize         // size of buffer
);


typedef struct _PROCESS_MEMORY_COUNTERS {
    DWORD cb;
    DWORD PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS;
typedef PROCESS_MEMORY_COUNTERS *PPROCESS_MEMORY_COUNTERS;

BOOL
WINAPI
GetProcessMemoryInfo(
  HANDLE Process,                          // handle to process
  PPROCESS_MEMORY_COUNTERS ppsmemCounters, // buffer
  DWORD cb                                 // size of buffer
);


}