#define _CRT_SECURE_NO_WARNINGS
//#include <windows.h>
#include "forever.h"
#include <ctype.h>
#include "getopt.h"
#include <direct.h> 
#include <io.h>
//#include "ntifs.h"
#include <Psapi.h>
#include <stdlib.h>          // �����
#include <time.h>            // �����
//#include <sys/types.h>       // �����½���
#include <ntsecapi.h> 
//#include "power_action.cpp"
//#include "file_process.cpp"
//#include "memory.cpp"
//#include "process.cpp"
#include <tlhelp32.h>
#include <algorithm>
#include <assert.h>

using namespace std;

#define STATUS_INFO_LENGTH_MISMATCH  ((NTSTATUS)0xC0000004L)
#define NT_SUCCESS(x) ((x) >= 0)

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG                   NextEntryOffset;
    ULONG                   NumberOfThreads;
    LARGE_INTEGER           Reserved[3];
    LARGE_INTEGER           CreateTime;
    LARGE_INTEGER           UserTime;
    LARGE_INTEGER           KernelTime;
    UNICODE_STRING          ImageName;  //LPWSTR ImageName;
    DWORD					BasePriority;
    HANDLE                  ProcessId;
    HANDLE                  InheritedFromProcessId;
    ULONG                   HandleCount;
    ULONG                   Reserved2[2];
    ULONG                   PrivatePageCount;
    DWORD					VirtualMemoryCounters;
    IO_COUNTERS             IoCounters;
    PVOID					Threads[0];
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemMirrorMemoryInformation,
    SystemPerformanceTraceInformation,
    SystemObsolete0,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemVerifierAddDriverInformation,
    SystemVerifierRemoveDriverInformation,
    SystemProcessorIdleInformation,
    SystemLegacyDriverInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation,
    SystemTimeSlipNotification,
    SystemSessionCreate,
    SystemSessionDetach,
    SystemSessionInformation,
    SystemRangeStartInformation,
    SystemVerifierInformation,
    SystemVerifierThunkExtend,
    SystemSessionProcessInformation,
    SystemLoadGdiDriverInSystemSpace,
    SystemNumaProcessorMap,
    SystemPrefetcherInformation,
    SystemExtendedProcessInformation,
    SystemRecommendedSharedDataAlignment,
    SystemComPlusPackage,
    SystemNumaAvailableMemory,
    SystemProcessorPowerInformation,
    SystemEmulationBasicInformation,
    SystemEmulationProcessorInformation,
    SystemExtendedHandleInformation,
    SystemLostDelayedWriteInformation,
    SystemBigPoolInformation,
    SystemSessionPoolTagInformation,
    SystemSessionMappedViewInformation,
    SystemHotpatchInformation,
    SystemObjectSecurityMode,
    SystemWatchdogTimerHandler,
    SystemWatchdogTimerInformation,
    SystemLogicalProcessorInformation,
    SystemWow64SharedInformation,
    SystemRegisterFirmwareTableInformationHandler,
    SystemFirmwareTableInformation,
    SystemModuleInformationEx,
    SystemVerifierTriageInformation,
    SystemSuperfetchInformation,
    SystemMemoryListInformation,
    SystemFileCacheInformationEx,
    MaxSystemInfoClass
} SYSTEM_INFORMATION_CLASS;

typedef NTSTATUS (WINAPI* pfnNtQuerySystemInformation)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength);
pfnNtQuerySystemInformation NtQuerySystemInformation = NULL;

typedef enum _SHUTDOWN_ACTION
{
    ShutdownNoReboot,
    ShutdownReboot,
    ShutdownPowerOff
} SHUTDOWN_ACTION;

typedef NTSTATUS(NTAPI* TYPE_NtShutdownSystem)(SHUTDOWN_ACTION);
typedef NTSTATUS(NTAPI* TYPE_NtInitiatePowerAction)(
    POWER_ACTION SystemAction,
    SYSTEM_POWER_STATE MinSystemState,
    ULONG Flags,
    BOOLEAN Asynchronous);

//typedef NTSTATUS(__cdecl* fnRtlSetProcessIsCritical)(IN  BOOLEAN  NewValue, OUT PBOOLEAN OldValue OPTIONAL, IN  BOOLEAN  CheckFlag);

typedef NTSTATUS(NTAPI* pdef_NtRaiseHardError)(
    NTSTATUS ErrorStatus,
    ULONG NumberOfParameters,
    ULONG UnicodeStringParameterMask OPTIONAL,
    PULONG_PTR Parameters,
    ULONG ResponseOption,
    PULONG Response);

typedef DWORD(WINAPI* NtTerminateProcess)(HANDLE, UINT);
typedef DWORD(WINAPI* NtSuspendProcess)(HANDLE ProcessHandle); 
typedef DWORD(WINAPI* NtResumeProcess)(HANDLE hProcess);

DWORD WINAPI AdjustPrivilege(LPCSTR lpPrivilegeName, bool fEnable) 
{
    HANDLE hToken;
    TOKEN_PRIVILEGES NewState;
    LUID luidPrivilegeLUID;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) /* Open process token */
        return GetLastError();

    if (fEnable == false) /*We disable all the privileges */
    {
        if (!AdjustTokenPrivileges(hToken, TRUE, NULL, 0, NULL, NULL))
            return GetLastError();

        else return ERROR_SUCCESS;
    }
    /*Look up the LUID value of the privilege */
    LookupPrivilegeValue(NULL, lpPrivilegeName, &luidPrivilegeLUID);

    /* Fill the TOKEN_STATE structure*/
    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = luidPrivilegeLUID;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    /* Adjust the process token's privileges */
    if (!AdjustTokenPrivileges(hToken, FALSE, &NewState, 0, NULL, NULL))
        return GetLastError();

    /* We've done successfully, return */
    return ERROR_SUCCESS;
}

const char* args = "vt:he:";     // argv����
const option args_long[] =       // argv����
{
    {(LPSTR)"version", no_argument,0, 'v'},
    {(LPSTR)"help", no_argument,0, 'h'},
    {(LPSTR)"timer", required_argument, 0, 't'},
    //{(LPSTR)"create-file", required_argument, 0, 'c'},
    {(LPSTR)"execute", required_argument, 0, 'e'},
    //{(LPSTR)"process", no_argument, 0, 'p'},
    //{(LPSTR)"size", required_argument, 0, 0xA},       // create-file  ������� ����0xA ������С  /*����*/
    //{(LPSTR)"cmode", required_argument, 0, 0xB},      // create-file  ������� ����0xB ����ģʽ  /*����*/
    //{(LPSTR)"no-display", no_argument, 0, 0xC},       // create-file  ������� ����0xC ��ʾ״̬
    {(LPSTR)"memory-alloc", required_argument, 0, 0xA},
    {(LPSTR)"memory-info", required_argument, 0, 0xB},    // ԭ����optional_argument
    {(LPSTR)"terminate-pid", required_argument, 0, 0xD},  // process      ������� ����0xD ��ֹ����
    {(LPSTR)"suspend-pid", required_argument, 0, 0xE},    // process      ������� ����0xE �������
    {(LPSTR)"resume-pid", required_argument, 0, 0xF},     // process      ������� ����0xF ��������Ľ���
    {(LPSTR)"terminate", required_argument, 0, 0x11},     // process      ������� ����0x11 ��ֹ����
    {(LPSTR)"suspend", required_argument, 0, 0x12},       // process      ������� ����0x12 �������
    {(LPSTR)"resume", required_argument, 0, 0x13},        // process      ������� ����0x13 ��������Ľ���
    {(LPSTR)"process-list", no_argument, 0, 0x10},        // �����б�      �޲���   ����0x10
    {(LPSTR)"sort-by", required_argument, 0, 0x14},       // ��������ʽ   ������� ����0x14
    {(LPSTR)"restart-pid", required_argument, 0, 0x15},   // ��������      ������� ����0x15
    //{(LPSTR)"restart", required_argument, 0, 0x1A},     // ��������      ������� ����0x1A  /*����*/
    {(LPSTR)"process-pid", required_argument, 0, 0x1A},       // ���ؽ���PID   ������� ����0x1A
    //{(LPSTR)"start", required_argument, 0, 0x1B},         // ���������ؽ���PID ������� ����0x1B /*����Σ�� ����*/
    {(LPSTR)"process-info", required_argument, 0, 0x1B},  // ���ؽ�����Ϣ   ������� ����0x1B 
    {0, 0, 0, 0}                                      // β����Ҫ�� ������bug
};

HMODULE hDll = GetModuleHandleA("NtDll.dll");    // ����ntdll (�����ļ�����)
typedef struct sortedProcessList
{
    DWORD pPID;
    char pName[260];
    DWORD pMemoryOccupied;
    //inline bool operator < (const struct sortedProcessList& x) const
    //{
    //    return pName < x.pName;
    //}
} ProcessList;
ProcessList processList[32767];
ProcessList buffer;
//typedef short mode;                  // ģʽ  /*����*/
//enum createItemMode   /*����*/
//{
//    cEmpty = -1,     // ���ļ�
//    cNormal = 0,     // ��ͨ���� ȫ�����ո�
//    cSparse = 1,     // ���̷��� ָ��ϡ���ļ� ʮ��������� 00
//    cRandom = 2,     // ������� ȫ����ASCII����ַ�
//
//};
LPSTR terminatorName;                // ��������
long timer = 0;                      // ��ʱ��
//unsigned long memorySize = 0;      // ����ڴ��С
//LPSTR process;                     // �������̵Ľ�������
char path[MAX_PATH];                 // �����ļ�·��
LPSTR fileName;                      // �����ļ���
char fullfileName[0x1000];           // �����ľ���·��
DWORD processPid = 0;         // ����pid
char processName[256] = { 0 };             // ��������
LPSTR createdFileSize = 0;           // �������ļ���С (�ַ���)
UINT64 MemoryAllocSize = 0;          // �ڴ�����С
unsigned int countProcess = 0;       // ��������

//HANDLE hTargetProcess = NULL;      // �������̵ľ�� ��FindProcessд�� ��main�ж���
bool fPowerOffSystem = false;          // �ϵ�
bool fResetSystem = false;             // ��Դ����
bool fThrowBlueScreen = false;         // ��������
bool fHibernateSystem = false;         // ��������
bool fSleepingSystem = false;          // ����˯��
bool fTimerActive = false;             // ��ʱ������
//bool fFillMemory = false;            // ����ڴ� (����)
//bool fOperateProcess = false;        // �������� (����)
bool fTerminateProcessbyPid = false;   //  ��������
bool fSuspendProcessbyPid = false;     //  �������
bool fResumeProcessbyPid = false;      //  ��������
bool fTerminateProcessbyName = false;   //  ��������
bool fSuspendProcessbyName = false;     //  �������
bool fResumeProcessbyName = false;      //  ��������
//bool fTerminateProcessbyPid = false; // ʹ�ý���pid�������� (����)
//bool fTerminateProcessbyName = false;// ʹ�ý������ֽ������� (����)
bool fMemoryInfo = false;              // �ڴ��������
bool fMemoryTotal = false;             // �ڴ��С��������
bool fMemoryAvail = false;             // �ڴ��С���ñ���
bool fMemoryCache = false;             // �ڴ滺���С����
bool fMemoryPageSize = false;          // ҳ��С����
bool fMemoryTotalPages = false;        // ϵͳ�Ѿ��ύ��ҳ������
bool fMemoryLimitPages = false;        // ϵͳ���Ƶ����ҳ������
bool fMemoryPeakPages = false;         // ���������������Ѿ��ύ�������ҳ������
bool fMemoryKernelTotal = false;       // �ں��ڴ���������
bool fMemoryKernelPaged = false;       // ��ҳ�ں˳��е�ǰλ��ҳ�е��ڴ汨��
bool fMemoryKernelNonPaged = false;    // �Ƿ�ҳ�ں˳��е��ڴ汨��
bool fMemoryHandleCount = false;       // ��ǰ�򿪵ľ��������
bool fMemoryProcessCount = false;      // ��ǰ����������
bool fMemoryThreadCount = false;       // ��ǰ�߳�������
bool fMemoryAllocByBytes = false;      // �ڴ���俪��
bool fReadFullProcessList = false;     // �����б�
bool fReadFullProcessSortByName = false;     // �����б� ����������
bool fReadFullProcessSortByMemory = false;   // �����б� ���ڴ���������
bool fReadFullProcessSortByPid = false;      // �����б� ��PID����
bool fRestartProcess = false;          // ��������
bool fPrintProcessPids = false;        // ��ý���pid
//bool fStartProcessAndShowPid = false;  // ��������ͬʱ��ʾpid
bool fPrintProcessInfo = false;        // ���������Ϣ
//bool fCreateItem = false;              // �����ļ� (����)
//bool fCreateItemSizeSet = false;       // �����ļ�ʱָ����С (�����ļ�����) (����)
//bool fCreateItemNoDisplay = false;     // �����ļ�ʱ����ʾ״̬ (�����ļ�����) (����)
//bool fCreateItemModeSet = false;       // �����ļ�ʱָ��ģʽ (�����ļ�����) (����)
//mode cMode;                            // �����ļ�ʱ��ģʽ (�����ļ�����) (����)

int ResetSystem(void);
int PowerOffSystem(void);
int ThrowBlueScreen(void);
int HibernateSystem(void);
int SleepingSystem(void);
int TerminateProcessByPid(void);
int SuspendProcessByPid(void);
int ResumeProcessByPid(void);
int GetSystemMemoryInfo(void);
enum sortBy
{
    byName = 0,
    byMemory = 1,
    byPid = 2
};
int ReadFullProcessList(short sort_by);
bool ReadFullProcessListCompareRule(ProcessList pl1, ProcessList pl2);
bool ReadFullProcessListCompareByMemory(ProcessList pl1, ProcessList pl2);
bool ReadFullProcessListCompareByPid(ProcessList pl1, ProcessList pl2);
#define ReadFullProcessListCompareByName ReadFullProcessListCompareRule
int GetFullProcessList(bool sort_or_not);
int PrintPidByProcessName(void);
int StartProcessAndShowPid(void);
UINT64 GetPhysicalMemoryTotalSize(void);
UINT64 GetPhysicalMemoryAvailSize(void);
UINT64 GetVirtualMemoryTotalSize(void);
UINT64 GetVirtualMemoryAvailSize(void);
UINT64 GetCacheMemorySize(void);             // �ڴ滺���С����
UINT64 GetPageSize(void);          // ҳ��С����
UINT64 GetTotalPages(void);        // ϵͳ�Ѿ��ύ��ҳ������
UINT64 GetLimitPages(void);        // ϵͳ���Ƶ����ҳ������
UINT64 GetPeakPages(void);         // ���������������Ѿ��ύ�������ҳ������
UINT64 GetKernelTotalMemorySize(void);       // �ں��ڴ���������
UINT64 GetKernelPagedMemorySize(void);       // ��ҳ�ں˳��е�ǰλ��ҳ�е��ڴ汨��
UINT64 GetKernelNonPagedMemorySize(void);    // �Ƿ�ҳ�ں˳��е��ڴ汨��
UINT64 GetHandleCount(void);       // ��ǰ�򿪵ľ��������
UINT64 GetProcessCount(void);      // ��ǰ����������
UINT64 GetThreadCount(void);       // ��ǰ�߳�������
//int MemoryAllocByBytes(UINT64 size); 



#define ONEXIT SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)  // ����˳�
bool WINAPI CtrlHandler(DWORD fdwctrltype)
{
    HWND cHwnd = FindWindow(NULL, terminatorName);
    switch (fdwctrltype)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:  //�رտ���̨�ĳ���
    case CTRL_BREAK_EVENT:  //���������˳�����
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        //if (fCreateItem == true)
        //{
            printf("\n\nExit 0x000001: User break\n");
            exit(1);
            return true;
        //}

    default:
        return false;
    }
}

int ResetSystem(void)
{
    TYPE_NtInitiatePowerAction NtInitiatePowerAction = (TYPE_NtInitiatePowerAction)GetProcAddress(hDll, "NtInitiatePowerAction");
    prints("Send ", RESET);
    prints("Reset", YELLOW);
    prints(" signal successfully\n", RESET);
    NtInitiatePowerAction(PowerActionShutdownReset, PowerSystemShutdown, 0, TRUE);
    return 0;
}

int PowerOffSystem(void)
{
    TYPE_NtInitiatePowerAction NtInitiatePowerAction = (TYPE_NtInitiatePowerAction)GetProcAddress(hDll, "NtInitiatePowerAction");
    prints("Send ", RESET);
    prints("PowerOff", YELLOW);
    prints(" signal successfully\n", RESET);
    NtInitiatePowerAction(PowerActionShutdownOff, PowerSystemShutdown, 0, TRUE);
    return 0;
}

int ThrowBlueScreen(void)
{
    ULONG uResp;
    pdef_NtRaiseHardError ThrowHardError = (pdef_NtRaiseHardError)GetProcAddress(hDll, "NtRaiseHardError");
    prints("Send ", RESET);
    prints("Error", YELLOW);
    prints(" signal successfully\n", RESET);
    ThrowHardError(STATUS_FLOAT_MULTIPLE_FAULTS, 0, 0, 0, 6, &uResp);
    return 0;
}

int HibernateSystem(void)
{
    TYPE_NtInitiatePowerAction NtInitiatePowerAction = (TYPE_NtInitiatePowerAction)GetProcAddress(hDll, "NtInitiatePowerAction");
    prints("Send ", RESET);
    prints("Hibernate", YELLOW);
    prints(" signal successfully\n", RESET);
    NtInitiatePowerAction(PowerActionHibernate, PowerSystemHibernate, 0, TRUE);
    return 0;
}

int SleepingSystem(void)
{
    TYPE_NtInitiatePowerAction NtInitiatePowerAction = (TYPE_NtInitiatePowerAction)GetProcAddress(hDll, "NtInitiatePowerAction");
    prints("Send ", RESET);
    prints("Sleep", YELLOW);
    prints(" signal successfully\n", RESET);
    NtInitiatePowerAction(PowerActionSleep, PowerSystemSleeping1, 0, TRUE);
    return 0;
}

int TerminateProcessByPid(void)
{
    NtTerminateProcess fNtTerminateProcess = NULL;

    fNtTerminateProcess = (NtTerminateProcess)GetProcAddress(hDll, "NtTerminateProcess");
    HANDLE hToken = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processPid); //��ý��̵����Ȩ�� (DWORD)hProcess
    if (hToken != 0)
    {
        char pidStr[8] = { 0 };
        fNtTerminateProcess(hToken, 1); //�رճ���
        _ltoa_s(processPid, pidStr, 0xA);
        prints("Send termination signal to PID ", RESET);
        prints(pidStr, RED);
        prints(" successfully\n", RESET);
        return 0;
    }
    prints("ERROR: ", RED);
    prints("Process Not Found\n", RESET);
    return 1;
}

int SuspendProcessByPid(void)
{
    NtSuspendProcess fNtSuspendProcess = NULL;

    fNtSuspendProcess = (NtSuspendProcess)GetProcAddress(hDll, "NtSuspendProcess");
    HANDLE hToken = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processPid); //��ý��̵����Ȩ�� (DWORD)hProcess
    if (hToken != 0)
    {
        char pidStr[8] = { 0 };
        fNtSuspendProcess(hToken); //����
        _ltoa_s(processPid, pidStr, 10);
        prints("Send suspend signal to PID ", RESET);
        prints(pidStr, YELLOW);
        prints(" successfully", RESET);
        return 0;
    }
    prints("ERROR: ", RED);
    prints("Process Not Found", RESET);
    return 1;
}

int ResumeProcessByPid(void)
{
    NtResumeProcess fNtResumeProcess = NULL;

    fNtResumeProcess = (NtResumeProcess)GetProcAddress(hDll, "NtResumeProcess");
    HANDLE hToken = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processPid); //��ý��̵����Ȩ�� (DWORD)hProcess
    if (hToken != 0)
    {
        char pidStr[8] = { 0 };
        fNtResumeProcess(hToken); //����
        _ltoa_s(processPid, pidStr, 10);
        prints("Send resume signal to PID ", RESET);
        prints(pidStr, GREEN);
        prints(" successfully", RESET);
        return 0;
    }
    prints("ERROR: ", RED);
    prints("Process Not Found", RESET);
    return 1;
}

bool ReadFullProcessListCompareRule(ProcessList pl1, ProcessList pl2)
{
    if (_stricmp(pl1.pName, pl2.pName) < 0)
        return pl1.pName < pl2.pName;
    else if (_stricmp(pl1.pName, pl2.pName) == 0)
        return pl1.pName == pl2.pName;
    else
        return pl1.pName > pl2.pName;
}

bool ReadFullProcessListCompareByMemory(ProcessList pl1, ProcessList pl2)
{
    return pl1.pMemoryOccupied < pl2.pMemoryOccupied;
}

bool ReadFullProcessListCompareByPid(ProcessList pl1, ProcessList pl2)
{
    return pl1.pPID < pl2.pPID;
}

int ReadFullProcessList(short sort_by)
{
    PROCESSENTRY32 currentProcess;                    // ��ſ�����Ϣ
    currentProcess.dwSize = sizeof(currentProcess);   // ��ʼ���ṹ���С (��Ҫ)
    HANDLE hProcesses = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // ����
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS processMemoryCounters;

    if (hProcesses == INVALID_HANDLE_VALUE)
    {
        prints("ERROR: ", RED);
        prints("Process snapshot failed", RESET);
        return -1;
    }

    bool bMore = Process32First(hProcesses, &currentProcess);    //��ȡ��һ��������Ϣ
    while (bMore)
    {
        processList[countProcess].pPID = currentProcess.th32ProcessID;
        strcpy_s(processList[countProcess].pName, currentProcess.szExeFile);
        //processList[countProcess].autoTrans();         // �Զ�ת��pid�ַ���
        //printf("PID=%5u    PName= %s\n", currentProcess.th32ProcessID, currentProcess.szExeFile);
        bMore = Process32Next(hProcesses, &currentProcess);    //������һ��
        countProcess++;
    }

    //sort(processList, processList + countProcess, ReadFullProcessListCompareByName);

    for (int i = 0; i < countProcess; i++)
    {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processList[i].pPID);  //����pid��ȡ����handle
        GetProcessMemoryInfo(hProcess, &processMemoryCounters, sizeof(processMemoryCounters)); //��ȡ�ڴ�ռ��
        processList[i].pMemoryOccupied = processMemoryCounters.PagefileUsage / 1024;  // KB��λ
        CloseHandle(hProcess);
    }

    if (sort_by == byMemory)
        sort(processList, processList + countProcess, ReadFullProcessListCompareByMemory);
    else if (sort_by == byPid)
        sort(processList, processList + countProcess, ReadFullProcessListCompareByPid);
    else
        sort(processList, processList + countProcess, ReadFullProcessListCompareByName);

    prints("Process Name                                    PID           Memory Usage\n", GREEN);
    prints("============================================================================\n", RESET);
    for (int i = 0; i < countProcess; i++)
    {
        printf("%-47s ", processList[i].pName);
        SetPrintColor(YELLOW);
        printf("%-8ld", processList[i].pPID);
        SetPrintColor(RESET);
        printf("%16s KB", StrLongInt(processList[i].pMemoryOccupied));
        printEnter;
    }

    CloseHandle(hProcesses);    //���hProcess���
    SetPrintColor(YELLOW);
    printf("\n%d processes running in total\n", countProcess);
    SetPrintColor(RESET);
    return 0;
}

int GetFullProcessList(bool sort_or_not)
{
    PROCESSENTRY32 currentProcess;                    // ��ſ�����Ϣ
    currentProcess.dwSize = sizeof(currentProcess);   // ��ʼ���ṹ���С (��Ҫ)
    HANDLE hProcesses = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // ����
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS processMemoryCounters;

    if (hProcesses == INVALID_HANDLE_VALUE)
    {
        prints("ERROR: ", RED);
        prints("Process snapshot failed", RESET);
        return -1;
    }

    bool bMore = Process32First(hProcesses, &currentProcess);    //��ȡ��һ��������Ϣ
    while (bMore)
    {
        processList[countProcess].pPID = currentProcess.th32ProcessID;
        strcpy_s(processList[countProcess].pName, currentProcess.szExeFile);
        //processList[countProcess].autoTrans();         // �Զ�ת��pid�ַ���
        //printf("PID=%5u    PName= %s\n", currentProcess.th32ProcessID, currentProcess.szExeFile);
        bMore = Process32Next(hProcesses, &currentProcess);    //������һ��
        countProcess++;
    }

    //sort(processList, processList + countProcess, ReadFullProcessListCompareByName);

    for (int i = 0; i < countProcess; i++)
    {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processList[i].pPID);  //����pid��ȡ����handle
        GetProcessMemoryInfo(hProcess, &processMemoryCounters, sizeof(processMemoryCounters)); //��ȡ�ڴ�ռ��
        processList[i].pMemoryOccupied = processMemoryCounters.PagefileUsage / 1024;  // KB��λ
        CloseHandle(hProcess);
    }

    if (sort_or_not)
        sort(processList, processList + countProcess, ReadFullProcessListCompareByPid);

    CloseHandle(hProcesses);    //���hProcess���
    return 0;
}

int PrintPidByProcessName()
{
    GetFullProcessList(true);
    prints(processName, YELLOW);
    prints(" has these PID(s):\n", RESET);
    SetPrintColor(YELLOW);
    for (int i = 0; i < countProcess; i++)
    {
        if (STRCMP(processList[i].pName, processName))
        {
            processPid = processList[i].pPID;
            printf("%ld\n", processPid);
        }
    }
    SetPrintColor(RESET);
    return 0;
}

//int StartProcessAndShowPid()
//{
//    STARTUPINFO startupInfo;
//    PROCESS_INFORMATION processInfo;
//    char specifiedPath[MAX_PATH] = "/c ";
//
//    STRCAT(specifiedPath, processName);
//    ZeroMemory(&processInfo, sizeof(processInfo));
//    ZeroMemory(&startupInfo, sizeof(startupInfo));
//    startupInfo.cb = sizeof(startupInfo);
//
//    if (CreateProcess
//        ("C:\\Windows\\System32\\cmd.exe", 0, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInfo)
//        )
//    {
//        prints("Process start successfully\n", GREEN);
//        prints("PID: ", RESET);
//        SetPrintColor(YELLOW);
//        printf("%ld\n", processInfo.dwProcessId);
//        SetPrintColor(RESET);
//    }
//    CloseHandle(processInfo.hThread);
//    CloseHandle(processInfo.hProcess);
//    return 0;
//}

int PrintProcessInfo()
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processPid);
    if (hProcess == 0)
    {
        prints("ERROR: ", RED);
        prints("Process Not Found\n", RESET);
        return -1;
    }

    DWORD imageFilePathStrMaxSize = MAX_PATH;     // �����ļ��ַ�������
    char imageFilePath[MAX_PATH];                 // �����ļ�·��
    char imageFileDevicePath[MAX_PATH];           // �����ļ��豸·��
    DWORD physicalMemoryOccupied = 0;             // �ڴ�ʹ����
    IO_COUNTERS ioCounters;                       // I/O��ȡ
    UINT64 readBytes = 0;
    UINT64 writeBytes = 0;
    int64_t cpuUsage = 0;
    if (GetProcessIoCounters(hProcess, &ioCounters))
    {
        readBytes = ioCounters.ReadTransferCount;
        writeBytes = ioCounters.WriteTransferCount;
    }

    GetFullProcessList(true);
    for (int i = 0; i < countProcess; i++)
    {
        if (processList[i].pPID == processPid)
        {
            STRCPY(processName, processList[i].pName);
            physicalMemoryOccupied = processList[i].pMemoryOccupied;
            break;
        }
    }

    GetProcessImageFileName(hProcess, imageFileDevicePath, sizeof(imageFileDevicePath));
    QueryFullProcessImageName(hProcess, 0, imageFilePath, &imageFilePathStrMaxSize);
    //cpuUsage = get_cpu_usage(hProcess);

    prints("Process Pid ", RESET);
    SetPrintColor(YELLOW);
    printf("%ld", processPid);
    SetPrintColor(RESET);
    prints(" has the following informations\n", RESET);
    printEnter;
    prints("Process Name : \n", BLUE);
    prints(processName, RESET);
    printEnter;
    prints("\nPath : \n", BLUE);
    prints(imageFilePath, RESET);
    printEnter;
    prints("\nDevice Path : \n", BLUE);
    prints(imageFileDevicePath, RESET);
    printEnter;
    prints("\nPhysical Memory Page Total Usage: \n", BLUE);
    prints(StrLongInt(physicalMemoryOccupied), RESET);
    prints(" KB", RESET);
    printEnter;
    //prints("\nCPU Usage : \n", BLUE);
    //printf("%lld", cpuUsage);
    //printEnter;
    prints("\nI/O Read and Write : \n", BLUE);
    prints("Read   ", LAKE_BLUE);
    printf("%10s  Bytes\n", StrLongInt(readBytes));
    prints("Write  ", LAKE_BLUE);
    printf("%10s  Bytes\n", StrLongInt(writeBytes));
    printEnter;
    return 0;
}

int RestartProcessByPid()
{
    //GetPidByProcessName();

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processPid);
    if (hProcess == 0)
    {
        prints("ERROR: ", RED);
        prints("Process Not Found\n", RESET);
        return -1;
    }
    DWORD imageFileNameStrMaxSize = MAX_PATH;
    char imageFileName[MAX_PATH];
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;

    //GetProcessImageFileName(hProcess, imageFileName, sizeof(imageFileName));
    QueryFullProcessImageName(hProcess, 0, imageFileName, &imageFileNameStrMaxSize);

    prints(imageFileName, BLUE);
    ZeroMemory(&processInfo, sizeof(processInfo));
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    TerminateProcessByPid();
    if (CreateProcess
        (imageFileName, 0, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInfo)
        )
    {
        prints("Process restart successfully\n", GREEN);
        prints("Previous PID: ", RESET);
        SetPrintColor(YELLOW);
        printf("%ld\n", processPid);
        SetPrintColor(RESET);
        prints("New PID: ", RESET);
        SetPrintColor(YELLOW);
        printf("%ld\n", processInfo.dwProcessId);
        SetPrintColor(RESET);
    }

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return 0;
}

UINT64 GetPhysicalMemoryTotalSize(void)
{
    MEMORYSTATUSEX mStatus;
    mStatus.dwLength = sizeof(mStatus);
    GlobalMemoryStatusEx(&mStatus);
    return mStatus.ullTotalPhys;
}

UINT64 GetPhysicalMemoryAvailSize(void)
{
    MEMORYSTATUSEX mStatus;
    mStatus.dwLength = sizeof(mStatus);
    GlobalMemoryStatusEx(&mStatus);
    return mStatus.ullAvailPhys;
}

UINT64 GetVirtualMemoryTotalSize(void)
{
    MEMORYSTATUSEX mStatus;
    mStatus.dwLength = sizeof(mStatus);
    GlobalMemoryStatusEx(&mStatus);
    return mStatus.ullTotalVirtual;
}

UINT64 GetVirtualMemoryAvailSize(void)
{
    MEMORYSTATUSEX mStatus;
    mStatus.dwLength = sizeof(mStatus);
    GlobalMemoryStatusEx(&mStatus);
    return mStatus.ullAvailVirtual;
}

int GetSystemMemoryInfo(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    MEMORYSTATUSEX mStatus;
    mStatus.dwLength = sizeof(mStatus);

    GlobalMemoryStatusEx(&mStatus);
    GetPerformanceInfo(&pInfo, cb);

    prints("Memory Summary\n", YELLOW); 
    printEnter;
    prints("==============================================================\n", RESET);
    prints("    Physical Memory ", GREEN); prints("  (Bytes)\n", GREY);
    prints("          Total Size:        ", RESET); printf("%30s", StrLongInt(mStatus.ullTotalPhys)); printEnter;
    prints("          In Use Size:       ", RESET); printf("%30s", StrLongInt(mStatus.ullTotalPhys - mStatus.ullAvailPhys)); printEnter;
    prints("          Available Size:    ", RESET); printf("%30s", StrLongInt(mStatus.ullAvailPhys)); printEnter;
    printEnter;
    prints("------------------------------------------------------------\n", RESET);
    prints("    Virtual Memory ", VIOLET); prints("  (Bytes)\n", GREY);
    prints("          Total Size:        ", RESET); printf("%30s", StrLongInt(mStatus.ullTotalVirtual)); printEnter;
    prints("          In Use Size:       ", RESET); printf("%30s", StrLongInt(mStatus.ullTotalVirtual - mStatus.ullAvailVirtual)); printEnter;
    prints("          Available Size:    ", RESET); printf("%30s", StrLongInt(mStatus.ullAvailVirtual)); printEnter;
    printEnter;
    prints("------------------------------------------------------------\n", RESET);
    prints("    Page File Information ", LAKE_BLUE); prints("  (Bytes)\n", GREY);
    prints("          Page Size:         ", RESET); printf("%30s", StrLongInt(pInfo.PageSize)); printEnter;
    prints("          Committed Limit:   ", RESET); printf("%30s", StrLongInt(pInfo.CommitLimit)); printEnter;
    prints("          Committed Peak:    ", RESET); printf("%30s", StrLongInt(pInfo.CommitPeak)); printEnter;
    prints("          Committed Total:   ", RESET); printf("%30s", StrLongInt(pInfo.CommitTotal)); printEnter;
    prints("          Cache Size:        ", RESET); printf("%30s", StrLongInt(pInfo.SystemCache)); printEnter;
    prints("          Kernel Paged:      ", RESET); printf("%30s", StrLongInt(pInfo.KernelPaged)); printEnter;
    prints("          Kernel Non-paged:  ", RESET); printf("%30s", StrLongInt(pInfo.KernelNonpaged)); printEnter;
    prints("          Kernel Total:      ", RESET); printf("%30s", StrLongInt(pInfo.KernelTotal)); printEnter;
    printEnter;
    prints("------------------------------------------------------------\n", RESET);
    prints("    Count Information \n", LAKE_BLUE);
    prints("          Handle Count:      ", RESET); printf("%30s", StrLongInt(pInfo.HandleCount)); printEnter;
    prints("          Process Count:     ", RESET); printf("%30s", StrLongInt(pInfo.ProcessCount)); printEnter;
    prints("          Thread Count:      ", RESET); printf("%30s", StrLongInt(pInfo.ThreadCount)); printEnter;
    printEnter;
    prints("==============================================================\n", RESET);
    printEnter;
    return 0;
}

UINT64 GetCacheMemorySize(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.SystemCache;
}

UINT64 GetPageSize(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.PageSize;
}

UINT64 GetTotalPages(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.CommitTotal;
}

UINT64 GetLimitPages(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.CommitLimit;
}

UINT64 GetPeakPages(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.CommitPeak;
}

UINT64 GetKernelTotalMemorySize(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.KernelTotal;
}

UINT64 GetKernelPagedMemorySize(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.KernelPaged;
}

UINT64 GetKernelNonPagedMemorySize(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.KernelNonpaged;
}

UINT64 GetHandleCount(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.HandleCount;
}

UINT64 GetProcessCount(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.ProcessCount;
}

UINT64 GetThreadCount(void)
{
    PERFORMANCE_INFORMATION pInfo;
    DWORD cb = sizeof(PERFORMANCE_INFORMATION);
    GetPerformanceInfo(&pInfo, cb);
    return pInfo.ThreadCount;
}

void PrintHelp(void)
{
    printEnter;
    prints("Terminator  1.9 \n", RED);
    prints("Copyright (C) Forever 2020-2023\n", RESET);
    printEnter;
    prints("Usage\n", RESET);
    printEnter;
    prints("  -e / --execute       Execute a kernel command below\n", YELLOW);
    prints("      \"off\"      --  Shutdown the computer immediately\n", RESET);
    prints("      \"reset\"    --  Restart the computer immediately\n", RESET);
    prints("      \"error\"    --  Throw blue screen error\n", RESET);
    prints("      \"hiber\"    --  Hibernate the computer immediately\n", RESET);
    prints("      \"sleep\"    --  Make the computer sleeping mode immediately\n", RESET);
    //prints("      \"process\"  --  <process name> Terminate process immediately (REQUIRE ntsd.exe)\n", RESET);
    printEnter;
    prints("  -t / --timer         Specify a timer before anything is executed\n", YELLOW);
    prints("      <#####>      --  Time as seconds\n", RESET);
    printEnter;
    prints("  --process-list       Show running process names and PIDs\n", YELLOW);
    prints("       --sort-by       Sort the list by: (without this para means to sort by name)\n", YELLOW);
    prints("       \"name\"        Image name (Default)\n", RESET);
    prints("       \"memory\"      Process memory usage\n", RESET);
    prints("       \"pid\"         Process ID \n", RESET);
    printEnter;
    prints("  --process-info       Show specified process info\n", YELLOW);
    prints("      <#####>    --  Process PID\n", RESET);
    printEnter;
    prints("  --process-pid        Show specified process PID(s)\n", YELLOW);
    prints("      <#####>    --  Process name\n", RESET);
    printEnter;
    prints("  --terminate-pid      Terminate the process by its PID\n", YELLOW);
    prints("  --suspend-pid        Suspend the process by its PID\n", YELLOW);
    prints("  --resume-pid         Resume the process by its PID\n", YELLOW);
    prints("  --restart-pid        Restart the program completely by its PID\n", YELLOW);
    prints("      <#####>    --  Process PID\n", RESET);
    printEnter;
    prints("  --terminate          Terminate the process by its name\n", YELLOW);
    prints("  --suspend            Suspend the process by its name\n", YELLOW);
    prints("  --resume             Resume the process by its name\n", YELLOW);
    //prints("  --start              Start a program with showing its PID thread", YELLOW);
    //prints("      Arguments: \n", LAKE_BLUE);
    //prints("            \"--pid\"        Terminate the process by the pid\n", RESET);
    prints("      <#####>    --  Process name\n", RESET);
    //prints("            \"--name\"       Terminate the process by its name\n", RESET);
    //prints("                  \"<name>\" --  Process name\n", RESET);
    //printEnter;
    //prints("  -c / --create-item   Create an empty file (You should specify the name and the type)\n", YELLOW);
    //prints("      <name>.<type>  \n", RESET);
    //prints("      Arguments: \n", LAKE_BLUE);
    //prints("            --size\       Resize the file newly created\n", RESET);
    //prints("            --no-display Do not display the status (for more performance)\n", RESET);
    //prints("            --cmode      (\'normal\' / \'sparse\' / \'random\')\n", RESET);
    //prints("                  \"normal\"  --  Normal Fill (Resize it by filling space)\n", RESET);
    //prints("                  \"sparse\"  --  Sparse Fill (Set the endline of it (filling nothing))\n", RESET);
    //prints("                  \"random\"  --  Random Fill (Fill it with random characters)\n", RESET);
    printEnter;
    prints("  --memory-info        Look up memory\n", YELLOW);
    prints("      \"all\"          --    Show all memory information\n", RESET);
    prints("      \"total\"        --    Return the total physical memory size\n", RESET);
    prints("      \"avail\"        --    Return the available physical memory size\n", RESET);
    prints("      \"page\"         --    Return the page memory information\n", RESET);
    prints("      \"page_size\"    --    Return the page memory size\n", RESET);
    prints("      \"page_total\"   --    Return the total page num that the system can submit\n", RESET);
    prints("      \"page_limit\"   --    Return the maximum page num that the system can submit\n", RESET);
    prints("      \"page_peak\"    --    Return the maximum page num that the system already submitted\n", RESET);
    prints("      \"cache\"        --    Return the cache memory size the system use by pages\n", RESET);
    prints("      \"kernel\"       --    Return the kernel memory information\n", RESET);
    prints("      \"kernel_total\" --    Return the total kernel memory size\n", RESET);
    prints("      \"kernel_paged\" --    Return the paged kernel memory size\n", RESET);
    prints("      \"kernel_npaged\"--    Return the non-paged kernel memory size\n", RESET);
    prints("      \"handle_count\" --    Return the handle counts\n", RESET);
    prints("      \"process_count\"--    Return the process counts\n", RESET);
    prints("      \"thread_count\" --    Return the thread counts\n", RESET);
    //printEnter;
    //prints("  --memory-alloc <#####>     Alloc new memory by bytes\n", YELLOW);
    printEnter;
    prints("  -h / --help          Show this help page\n", VIOLET);
    prints("  -v / --version       Show the program version\n", VIOLET);
    printEnter;
    return;
}

void PrintVersion(void)
{
    prints("Terminator 1.9 (for development use)\n", GREEN);
    prints("Copyright (C) Forever 2020-2023\n", RESET);
    return;
}

int main(int argc, char** argv) 
{
    byte executionFlag = 0;
    byte helpFlag = 0;
    //int index;
    int c;
    terminatorName = argv[0];

    if (argc == 1)
    {
        PrintHelp();
        system("pause");
        return 0;
    }

    while ((c = getopt_long(argc, argv, args, args_long, NULL)) != -1)
    {
        if (c == 'h')
        {
            PrintHelp();
            return 0;
        }
        switch (c)
        {
        case 't':
            fTimerActive = true;
            timer = atoi(optarg);
            break;

        case 'v':
            PrintVersion();
            break;

        case 0xA:
            fMemoryAllocByBytes = true;
            MemoryAllocSize = atol(optarg);
            break;

        case 0xB:
            if (STRCMP(optarg, "all"))
            {
                fMemoryInfo = true;
            }
            else if (STRCMP(optarg, "total"))
            {
                fMemoryTotal = true;
            }
            else if (STRCMP(optarg, "avail"))
            {
                fMemoryAvail = true;
            }
            else if (STRCMP(optarg, "cache"))
            {
                fMemoryCache = true;
            }
            else if (STRCMP(optarg, "page_size"))
            {
                fMemoryPageSize = true;
            }
            else if (STRCMP(optarg, "page_total"))
            {
                fMemoryTotalPages = true;
            }
            else if (STRCMP(optarg, "page_limit"))
            {
                fMemoryLimitPages = true;
            }
            else if (STRCMP(optarg, "page_peak"))
            {
                fMemoryPeakPages = true;
            }
            else if (STRCMP(optarg, "kernel_total"))
            {
                fMemoryKernelTotal = true;
            }
            else if (STRCMP(optarg, "kernel_paged"))
            {
                fMemoryKernelPaged = true;
            }
            else if (STRCMP(optarg, "kernel_npaged"))
            {
                fMemoryKernelNonPaged = true;
            }
            else if (STRCMP(optarg, "handle_count"))
            {
                fMemoryHandleCount = true;
            }
            else if (STRCMP(optarg, "process_count"))
            {
                fMemoryProcessCount = true;
            }
            else if (STRCMP(optarg, "thread_count"))
            {
                fMemoryThreadCount = true;
            }
            else
            {
                prints("ERROR: ", RED);
                prints("\'--memory-info\' expects a suitable argument: check help by using \'-h\'", RESET);
                return 1;
            }
            break;

        /*
        case 'c':
            fCreateItem = true;
            fileName = optarg;
            break;

        case 0xA:
            fCreateItemSizeSet = true;
            createdFileSize = optarg; // �����ļ���С
            break;

        case 0xB:
            fCreateItemModeSet = true; 
            // �����ļ�ģʽ
            if (STRCMP(optarg, "normal") || STRCMP(optarg, "Normal"))
                cMode = cNormal;
            if (STRCMP(optarg, "sparse") || STRCMP(optarg, "Sparse"))
                cMode = cSparse;
            if (STRCMP(optarg, "random") || STRCMP(optarg, "Random"))
                cMode = cRandom;
            break;

        case 0xC:
            fCreateItemNoDisplay = true;
            break;
        */

        //case 'p':
        //    fOperateProcess = true;
        //    //processPid = atoi(optarg);
        //    break;

        case 0xD:
            fTerminateProcessbyPid = true;
            processPid = atol(optarg); //�������pid
            break;

        case 0xE:
            fSuspendProcessbyPid = true;
            processPid = atol(optarg); //�������pid
            break;

        case 0xF:
            fResumeProcessbyPid = true;
            processPid = atol(optarg); //�������pid
            break;

        case 0x11:
            fTerminateProcessbyName = true;
            strcpy_s(processName, optarg);
            break;

        case 0x12:
            fSuspendProcessbyName = true;
            strcpy_s(processName, optarg);
            break;

        case 0x13:
            fResumeProcessbyName = true;
            strcpy_s(processName, optarg);
            break;

        case 0x10:
            fReadFullProcessList = true;
            break;

        case 0x14:
            if (STRCMP(optarg, "memory"))
                fReadFullProcessSortByMemory = true;
            else if (STRCMP(optarg, "pid"))
                fReadFullProcessSortByPid = true;
            else
                fReadFullProcessSortByName = true;
            break;

        case 0x15:
            fRestartProcess = true;
            processPid = atol(optarg);
            break;

        //case 0x1A:
        //    fRestartProcess = true;
        //    STRCPY(processName, optarg);
        //    GetPidByProcessName();
        //    break;

        case 0x1A:
            fPrintProcessPids = true;
            STRCPY(processName, optarg);
            break;

        case 0x1B:
            fPrintProcessInfo = true;
            processPid = atol(optarg);
            break;

        case 'e':
            executionFlag = 1;
            if (STRCMP(optarg, "off"))
                fPowerOffSystem = true;
            else if (STRCMP(optarg, "reset"))
                fResetSystem = true;
            else if (STRCMP(optarg, "error"))
                fThrowBlueScreen = true;
            else if (STRCMP(optarg, "hiber"))
                fHibernateSystem = true;
            else if (STRCMP(optarg, "sleep"))
                fSleepingSystem = true;
            else
            {
                printf("\'-c\' expects a correct argument but not: %s\n", optarg);
                return 1;
            }
            break;

        case 'h':
            PrintHelp();
            return 0;

        case '?':
            prints("Error: ", RED);
            printf("Unknown option, type \"-h\" or \"--help\" to get help\n");
            return 1;
            //if (optopt == 'c')
            //    _ftprintf(stderr, _T("Option -%c requires an argument.\n"), optopt);
            //else if (isprint(optopt))
            //    _ftprintf(stderr, _T("Unknown option `-%c'.\n"), optopt);
            //else
            //    _ftprintf(stderr, _T("Unknown option character `\\x%x'.\n"), optopt);
            //return 1;

        default:
            abort();
        }
    }

    ONEXIT;

    if (fTimerActive == true)
    {
        Sleep(timer * 1000);
    }

    if (fPowerOffSystem == true)
    {
        AdjustPrivilege(SE_SHUTDOWN_NAME, true);
        PowerOffSystem();
        return 0;
    }

    if (fResetSystem == true)
    {
        AdjustPrivilege(SE_SHUTDOWN_NAME, true);
        ResetSystem();
        return 0;
    }

    if (fThrowBlueScreen == true)
    {
        AdjustPrivilege(SE_SHUTDOWN_NAME, true);
        ThrowBlueScreen();
        return 0;
    }

    if (fHibernateSystem == true)
    {
        AdjustPrivilege(SE_SHUTDOWN_NAME, true);
        HibernateSystem();
        return 0;
    }

    if (fSleepingSystem == true)
    {
        AdjustPrivilege(SE_SHUTDOWN_NAME, true);
        SleepingSystem();
        return 0;
    }

    if (fTerminateProcessbyPid == true)
    {
        //HANDLE hTargetProcess = NULL;  // �������̵ľ�� ��FindProcessд��
        AdjustPrivilege(SE_DEBUG_NAME, true);
        //hTargetProcess = FindProcess(process); // ������hTargetProcess �ĳ�pid��
        TerminateProcessByPid();
        return 0;
    }

    if (fSuspendProcessbyPid == true)
    {
        AdjustPrivilege(SE_DEBUG_NAME, true);
        SuspendProcessByPid();
        return 0;
    }

    if (fResumeProcessbyPid == true)
    {
        AdjustPrivilege(SE_DEBUG_NAME, true);
        ResumeProcessByPid();
        return 0;
    }

    if (fTerminateProcessbyName == true)
    {
        AdjustPrivilege(SE_DEBUG_NAME, true);
        GetFullProcessList(true);
        prints("Terminate process(es) : \"", RESET);
        prints(processName, RED);
        prints("\"\n", RESET);
        for (int i = 0; i < countProcess; i++)
        {
            if (STRCMP(processList[i].pName, processName))
            {
                processPid = processList[i].pPID;
                TerminateProcessByPid();
            }
        }
        return 0;
    }

    if (fSuspendProcessbyName == true)
    {
        AdjustPrivilege(SE_DEBUG_NAME, true);
        GetFullProcessList(true);
        prints("Suspend process(es) : \"", RESET);
        prints(processName, YELLOW);
        prints("\"\n", RESET);
        for (int i = 0; i < countProcess; i++)
        {
            if (STRCMP(processList[i].pName, processName))
            {
                processPid = processList[i].pPID;
                SuspendProcessByPid();
            }
        }
        return 0;
    }

    if (fResumeProcessbyName == true)
    {
        AdjustPrivilege(SE_DEBUG_NAME, true);
        GetFullProcessList(true);
        prints("Resume process(es) : \"", RESET);
        prints(processName, GREEN);
        prints("\"\n", RESET);
        for (int i = 0; i < countProcess; i++)
        {
            if (STRCMP(processList[i].pName, processName))
            {
                processPid = processList[i].pPID;
                ResumeProcessByPid();
            }
        }
        return 0;
    }

    if (fReadFullProcessList == true)
    {
        if (fReadFullProcessSortByMemory == true)
            ReadFullProcessList(byMemory);
        else if (fReadFullProcessSortByPid == true)
            ReadFullProcessList(byPid);
        else
            ReadFullProcessList(byName);
        return 0;
    }

    //MEMORY 
    if (fMemoryTotal == true)
    {
        printf("%lld", GetPhysicalMemoryTotalSize());
        return 0;
    }
    if (fMemoryAvail == true)
    {
        printf("%lld", GetPhysicalMemoryAvailSize());
        return 0;
    }
    if (fMemoryCache == true)
    {
        printf("%lld", GetCacheMemorySize());
        return 0;
    }
    if (fMemoryPageSize == true)
    {
        printf("%lld", GetPageSize());
        return 0;
    }
    if (fMemoryTotalPages == true)
    {
        printf("%lld", GetTotalPages());
        return 0;
    }
    if (fMemoryLimitPages == true)
    {
        printf("%lld", GetLimitPages());
        return 0;
    }
    if (fMemoryPeakPages == true)
    {
        printf("%lld", GetPeakPages());
        return 0;
    }
    if (fMemoryKernelTotal == true)
    {
        printf("%lld", GetKernelTotalMemorySize());
        return 0;
    }
    if (fMemoryKernelPaged == true)
    {
        printf("%lld", GetKernelPagedMemorySize());
        return 0;
    }
    if (fMemoryKernelNonPaged == true)
    {
        printf("%lld", GetKernelNonPagedMemorySize());
        return 0;
    }
    if (fMemoryHandleCount == true)
    {
        printf("%lld", GetHandleCount());
        return 0;
    }
    if (fMemoryProcessCount == true)
    {
        printf("%lld", GetProcessCount());
        return 0;
    }
    if (fMemoryThreadCount == true)
    {
        printf("%lld", GetThreadCount());
        return 0;
    }
    if (
        (fMemoryInfo == true) &&
        (fMemoryAvail == false) &&
        (fMemoryCache == false) &&
        (fMemoryHandleCount == false) &&
        (fMemoryKernelNonPaged == false) &&
        (fMemoryKernelPaged == false) &&
        (fMemoryKernelTotal == false) &&
        (fMemoryLimitPages == false) &&
        (fMemoryPageSize == false) &&
        (fMemoryPeakPages == false) &&
        (fMemoryProcessCount == false) &&
        (fMemoryThreadCount == false) &&
        (fMemoryTotal == false) &&
        (fMemoryTotalPages == false)
        )
    {
        GetSystemMemoryInfo();
        return 0;
    }
    if (fMemoryAllocByBytes == true)
    {
        //MemoryAllocByBytes(MemoryAllocSize);
    }
    if (fRestartProcess == true)
    {
        RestartProcessByPid();
        return 0;
    }
    if (fPrintProcessPids == true)
    {
        PrintPidByProcessName();
        return 0;
    }
    if (fPrintProcessInfo == true)
    {
        PrintProcessInfo();
        return 0;
    }
} 