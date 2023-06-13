#define _CRT_SECURE_NO_WARNINGS
//#include <windows.h>
#include "forever.h"
#include <ctype.h>
#include "getopt.h"
#include <direct.h> 
#include <io.h>
//#include "ntifs.h"
#include <Psapi.h>
#include <stdlib.h>          // 随机数
#include <time.h>            // 随机数
//#include <sys/types.h>       // 创建新进程
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

const char* args = "vt:he:";     // argv解析
const option args_long[] =       // argv解析
{
    {(LPSTR)"version", no_argument,0, 'v'},
    {(LPSTR)"help", no_argument,0, 'h'},
    {(LPSTR)"timer", required_argument, 0, 't'},
    //{(LPSTR)"create-file", required_argument, 0, 'c'},
    {(LPSTR)"execute", required_argument, 0, 'e'},
    //{(LPSTR)"process", no_argument, 0, 'p'},
    //{(LPSTR)"size", required_argument, 0, 0xA},       // create-file  额外参数 返回0xA 创建大小  /*弃用*/
    //{(LPSTR)"cmode", required_argument, 0, 0xB},      // create-file  额外参数 返回0xB 创建模式  /*弃用*/
    //{(LPSTR)"no-display", no_argument, 0, 0xC},       // create-file  额外参数 返回0xC 显示状态
    {(LPSTR)"memory-alloc", required_argument, 0, 0xA},
    {(LPSTR)"memory-info", required_argument, 0, 0xB},    // 原先是optional_argument
    {(LPSTR)"terminate-pid", required_argument, 0, 0xD},  // process      额外参数 返回0xD 终止进程
    {(LPSTR)"suspend-pid", required_argument, 0, 0xE},    // process      额外参数 返回0xE 挂起进程
    {(LPSTR)"resume-pid", required_argument, 0, 0xF},     // process      额外参数 返回0xF 继续挂起的进程
    {(LPSTR)"terminate", required_argument, 0, 0x11},     // process      额外参数 返回0x11 终止进程
    {(LPSTR)"suspend", required_argument, 0, 0x12},       // process      额外参数 返回0x12 挂起进程
    {(LPSTR)"resume", required_argument, 0, 0x13},        // process      额外参数 返回0x13 继续挂起的进程
    {(LPSTR)"process-list", no_argument, 0, 0x10},        // 进程列表      无参数   返回0x10
    {(LPSTR)"sort-by", required_argument, 0, 0x14},       // 进程排序方式   额外参数 返回0x14
    {(LPSTR)"restart-pid", required_argument, 0, 0x15},   // 重启进程      额外参数 返回0x15
    //{(LPSTR)"restart", required_argument, 0, 0x1A},     // 重启进程      额外参数 返回0x1A  /*弃用*/
    {(LPSTR)"process-pid", required_argument, 0, 0x1A},       // 返回进程PID   额外参数 返回0x1A
    //{(LPSTR)"start", required_argument, 0, 0x1B},         // 启动并返回进程PID 额外参数 返回0x1B /*函数危险 弃用*/
    {(LPSTR)"process-info", required_argument, 0, 0x1B},  // 返回进程信息   额外参数 返回0x1B 
    {0, 0, 0, 0}                                      // 尾部需要空 否则有bug
};

HMODULE hDll = GetModuleHandleA("NtDll.dll");    // 载入ntdll (其他文件定义)
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
//typedef short mode;                  // 模式  /*弃用*/
//enum createItemMode   /*弃用*/
//{
//    cEmpty = -1,     // 空文件
//    cNormal = 0,     // 普通分配 全部填充空格
//    cSparse = 1,     // 立刻分配 指定稀疏文件 十六进制填充 00
//    cRandom = 2,     // 随机分配 全部填ASCII随机字符
//
//};
LPSTR terminatorName;                // 主程序名
long timer = 0;                      // 计时器
//unsigned long memorySize = 0;      // 填充内存大小
//LPSTR process;                     // 结束进程的进程名儿
char path[MAX_PATH];                 // 创建文件路径
LPSTR fileName;                      // 创建文件名
char fullfileName[0x1000];           // 创建的绝对路径
DWORD processPid = 0;         // 进程pid
char processName[256] = { 0 };             // 进程名字
LPSTR createdFileSize = 0;           // 创建的文件大小 (字符串)
UINT64 MemoryAllocSize = 0;          // 内存分配大小
unsigned int countProcess = 0;       // 进程总数

//HANDLE hTargetProcess = NULL;      // 结束进程的句柄 由FindProcess写入 在main中定义
bool fPowerOffSystem = false;          // 断电
bool fResetSystem = false;             // 电源重置
bool fThrowBlueScreen = false;         // 测试蓝屏
bool fHibernateSystem = false;         // 立即休眠
bool fSleepingSystem = false;          // 立即睡眠
bool fTimerActive = false;             // 计时器启动
//bool fFillMemory = false;            // 填充内存 (弃用)
//bool fOperateProcess = false;        // 操作进程 (弃用)
bool fTerminateProcessbyPid = false;   //  结束进程
bool fSuspendProcessbyPid = false;     //  挂起进程
bool fResumeProcessbyPid = false;      //  继续进程
bool fTerminateProcessbyName = false;   //  结束进程
bool fSuspendProcessbyName = false;     //  挂起进程
bool fResumeProcessbyName = false;      //  继续进程
//bool fTerminateProcessbyPid = false; // 使用进程pid结束进程 (弃用)
//bool fTerminateProcessbyName = false;// 使用进程名字结束进程 (弃用)
bool fMemoryInfo = false;              // 内存操作开关
bool fMemoryTotal = false;             // 内存大小总数报告
bool fMemoryAvail = false;             // 内存大小可用报告
bool fMemoryCache = false;             // 内存缓存大小报告
bool fMemoryPageSize = false;          // 页大小报告
bool fMemoryTotalPages = false;        // 系统已经提交的页数报告
bool fMemoryLimitPages = false;        // 系统限制的最大页数报告
bool fMemoryPeakPages = false;         // 重新启动以来的已经提交过的最大页数报告
bool fMemoryKernelTotal = false;       // 内核内存总数报告
bool fMemoryKernelPaged = false;       // 分页内核池中当前位于页中的内存报告
bool fMemoryKernelNonPaged = false;    // 非分页内核池中的内存报告
bool fMemoryHandleCount = false;       // 当前打开的句柄数报告
bool fMemoryProcessCount = false;      // 当前进程数报告
bool fMemoryThreadCount = false;       // 当前线程数报告
bool fMemoryAllocByBytes = false;      // 内存分配开辟
bool fReadFullProcessList = false;     // 进程列表
bool fReadFullProcessSortByName = false;     // 进程列表 按名称排序
bool fReadFullProcessSortByMemory = false;   // 进程列表 按内存用量排序
bool fReadFullProcessSortByPid = false;      // 进程列表 按PID排序
bool fRestartProcess = false;          // 重启进程
bool fPrintProcessPids = false;        // 获得进程pid
//bool fStartProcessAndShowPid = false;  // 启动进程同时显示pid
bool fPrintProcessInfo = false;        // 输出进程信息
//bool fCreateItem = false;              // 创建文件 (弃用)
//bool fCreateItemSizeSet = false;       // 创建文件时指定大小 (其他文件定义) (弃用)
//bool fCreateItemNoDisplay = false;     // 创建文件时不显示状态 (其他文件定义) (弃用)
//bool fCreateItemModeSet = false;       // 创建文件时指定模式 (其他文件定义) (弃用)
//mode cMode;                            // 创建文件时的模式 (其他文件定义) (弃用)

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
UINT64 GetCacheMemorySize(void);             // 内存缓存大小报告
UINT64 GetPageSize(void);          // 页大小报告
UINT64 GetTotalPages(void);        // 系统已经提交的页数报告
UINT64 GetLimitPages(void);        // 系统限制的最大页数报告
UINT64 GetPeakPages(void);         // 重新启动以来的已经提交过的最大页数报告
UINT64 GetKernelTotalMemorySize(void);       // 内核内存总数报告
UINT64 GetKernelPagedMemorySize(void);       // 分页内核池中当前位于页中的内存报告
UINT64 GetKernelNonPagedMemorySize(void);    // 非分页内核池中的内存报告
UINT64 GetHandleCount(void);       // 当前打开的句柄数报告
UINT64 GetProcessCount(void);      // 当前进程数报告
UINT64 GetThreadCount(void);       // 当前线程数报告
//int MemoryAllocByBytes(UINT64 size); 



#define ONEXIT SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)  // 检测退出
bool WINAPI CtrlHandler(DWORD fdwctrltype)
{
    HWND cHwnd = FindWindow(NULL, terminatorName);
    switch (fdwctrltype)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:  //关闭控制台的出口
    case CTRL_BREAK_EVENT:  //程序正常退出出口
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
    HANDLE hToken = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processPid); //获得进程的最大权限 (DWORD)hProcess
    if (hToken != 0)
    {
        char pidStr[8] = { 0 };
        fNtTerminateProcess(hToken, 1); //关闭程序
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
    HANDLE hToken = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processPid); //获得进程的最大权限 (DWORD)hProcess
    if (hToken != 0)
    {
        char pidStr[8] = { 0 };
        fNtSuspendProcess(hToken); //挂起
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
    HANDLE hToken = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processPid); //获得进程的最大权限 (DWORD)hProcess
    if (hToken != 0)
    {
        char pidStr[8] = { 0 };
        fNtResumeProcess(hToken); //挂起
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
    PROCESSENTRY32 currentProcess;                    // 存放快照信息
    currentProcess.dwSize = sizeof(currentProcess);   // 初始化结构体大小 (必要)
    HANDLE hProcesses = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // 拍照
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS processMemoryCounters;

    if (hProcesses == INVALID_HANDLE_VALUE)
    {
        prints("ERROR: ", RED);
        prints("Process snapshot failed", RESET);
        return -1;
    }

    bool bMore = Process32First(hProcesses, &currentProcess);    //获取第一个进程信息
    while (bMore)
    {
        processList[countProcess].pPID = currentProcess.th32ProcessID;
        strcpy_s(processList[countProcess].pName, currentProcess.szExeFile);
        //processList[countProcess].autoTrans();         // 自动转化pid字符串
        //printf("PID=%5u    PName= %s\n", currentProcess.th32ProcessID, currentProcess.szExeFile);
        bMore = Process32Next(hProcesses, &currentProcess);    //遍历下一个
        countProcess++;
    }

    //sort(processList, processList + countProcess, ReadFullProcessListCompareByName);

    for (int i = 0; i < countProcess; i++)
    {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processList[i].pPID);  //遍历pid获取进程handle
        GetProcessMemoryInfo(hProcess, &processMemoryCounters, sizeof(processMemoryCounters)); //获取内存占用
        processList[i].pMemoryOccupied = processMemoryCounters.PagefileUsage / 1024;  // KB单位
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

    CloseHandle(hProcesses);    //清除hProcess句柄
    SetPrintColor(YELLOW);
    printf("\n%d processes running in total\n", countProcess);
    SetPrintColor(RESET);
    return 0;
}

int GetFullProcessList(bool sort_or_not)
{
    PROCESSENTRY32 currentProcess;                    // 存放快照信息
    currentProcess.dwSize = sizeof(currentProcess);   // 初始化结构体大小 (必要)
    HANDLE hProcesses = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // 拍照
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS processMemoryCounters;

    if (hProcesses == INVALID_HANDLE_VALUE)
    {
        prints("ERROR: ", RED);
        prints("Process snapshot failed", RESET);
        return -1;
    }

    bool bMore = Process32First(hProcesses, &currentProcess);    //获取第一个进程信息
    while (bMore)
    {
        processList[countProcess].pPID = currentProcess.th32ProcessID;
        strcpy_s(processList[countProcess].pName, currentProcess.szExeFile);
        //processList[countProcess].autoTrans();         // 自动转化pid字符串
        //printf("PID=%5u    PName= %s\n", currentProcess.th32ProcessID, currentProcess.szExeFile);
        bMore = Process32Next(hProcesses, &currentProcess);    //遍历下一个
        countProcess++;
    }

    //sort(processList, processList + countProcess, ReadFullProcessListCompareByName);

    for (int i = 0; i < countProcess; i++)
    {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processList[i].pPID);  //遍历pid获取进程handle
        GetProcessMemoryInfo(hProcess, &processMemoryCounters, sizeof(processMemoryCounters)); //获取内存占用
        processList[i].pMemoryOccupied = processMemoryCounters.PagefileUsage / 1024;  // KB单位
        CloseHandle(hProcess);
    }

    if (sort_or_not)
        sort(processList, processList + countProcess, ReadFullProcessListCompareByPid);

    CloseHandle(hProcesses);    //清除hProcess句柄
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

    DWORD imageFilePathStrMaxSize = MAX_PATH;     // 镜像文件字符串长度
    char imageFilePath[MAX_PATH];                 // 镜像文件路径
    char imageFileDevicePath[MAX_PATH];           // 镜像文件设备路径
    DWORD physicalMemoryOccupied = 0;             // 内存使用量
    IO_COUNTERS ioCounters;                       // I/O读取
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
            createdFileSize = optarg; // 传入文件大小
            break;

        case 0xB:
            fCreateItemModeSet = true; 
            // 创建文件模式
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
            processPid = atol(optarg); //传入进程pid
            break;

        case 0xE:
            fSuspendProcessbyPid = true;
            processPid = atol(optarg); //传入进程pid
            break;

        case 0xF:
            fResumeProcessbyPid = true;
            processPid = atol(optarg); //传入进程pid
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
        //HANDLE hTargetProcess = NULL;  // 结束进程的句柄 由FindProcess写入
        AdjustPrivilege(SE_DEBUG_NAME, true);
        //hTargetProcess = FindProcess(process); // 不传入hTargetProcess 改成pid了
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