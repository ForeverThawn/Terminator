#include "forever.h"
#include <tlhelp32.h>
#include <algorithm>
#include <Psapi.h>

using namespace std;

// �Ҿ���Ӧ����������Ȩ�� AdjustPrivilege(SE_DEBUG_NAME, true);

// NT Defined Privileges
#define SE_CREATE_TOKEN_NAME                         TEXT("SeCreateTokenPrivilege")
#define SE_ASSIGNPRIMARYTOKEN_NAME                   TEXT("SeAssignPrimaryTokenPrivilege")
#define SE_LOCK_MEMORY_NAME                          TEXT("SeLockMemoryPrivilege")
#define SE_INCREASE_QUOTA_NAME                       TEXT("SeIncreaseQuotaPrivilege")
#define SE_UNSOLICITED_INPUT_NAME                    TEXT("SeUnsolicitedInputPrivilege")
#define SE_MACHINE_ACCOUNT_NAME                      TEXT("SeMachineAccountPrivilege")
#define SE_TCB_NAME                                  TEXT("SeTcbPrivilege")
#define SE_SECURITY_NAME                             TEXT("SeSecurityPrivilege")
#define SE_TAKE_OWNERSHIP_NAME                       TEXT("SeTakeOwnershipPrivilege")
#define SE_LOAD_DRIVER_NAME                          TEXT("SeLoadDriverPrivilege")
#define SE_SYSTEM_PROFILE_NAME                       TEXT("SeSystemProfilePrivilege")
#define SE_SYSTEMTIME_NAME                           TEXT("SeSystemtimePrivilege")
#define SE_PROF_SINGLE_PROCESS_NAME                  TEXT("SeProfileSingleProcessPrivilege")
#define SE_INC_BASE_PRIORITY_NAME                    TEXT("SeIncreaseBasePriorityPrivilege")
#define SE_CREATE_PAGEFILE_NAME                      TEXT("SeCreatePagefilePrivilege")
#define SE_CREATE_PERMANENT_NAME                     TEXT("SeCreatePermanentPrivilege")
#define SE_BACKUP_NAME                               TEXT("SeBackupPrivilege")
#define SE_RESTORE_NAME                              TEXT("SeRestorePrivilege")
#define SE_SHUTDOWN_NAME                             TEXT("SeShutdownPrivilege")
#define SE_DEBUG_NAME                                TEXT("SeDebugPrivilege")
#define SE_AUDIT_NAME                                TEXT("SeAuditPrivilege")
#define SE_SYSTEM_ENVIRONMENT_NAME                   TEXT("SeSystemEnvironmentPrivilege")
#define SE_CHANGE_NOTIFY_NAME                        TEXT("SeChangeNotifyPrivilege")
#define SE_REMOTE_SHUTDOWN_NAME                      TEXT("SeRemoteShutdownPrivilege")
#define SE_UNDOCK_NAME                               TEXT("SeUndockPrivilege")
#define SE_SYNC_AGENT_NAME                           TEXT("SeSyncAgentPrivilege")
#define SE_ENABLE_DELEGATION_NAME                    TEXT("SeEnableDelegationPrivilege")
#define SE_MANAGE_VOLUME_NAME                        TEXT("SeManageVolumePrivilege")
#define SE_IMPERSONATE_NAME                          TEXT("SeImpersonatePrivilege")
#define SE_CREATE_GLOBAL_NAME                        TEXT("SeCreateGlobalPrivilege")
#define SE_TRUSTED_CREDMAN_ACCESS_NAME               TEXT("SeTrustedCredManAccessPrivilege")
#define SE_RELABEL_NAME                              TEXT("SeRelabelPrivilege")
#define SE_INC_WORKING_SET_NAME                      TEXT("SeIncreaseWorkingSetPrivilege")
#define SE_TIME_ZONE_NAME                            TEXT("SeTimeZonePrivilege")
#define SE_CREATE_SYMBOLIC_LINK_NAME                 TEXT("SeCreateSymbolicLinkPrivilege")
#define SE_DELEGATE_SESSION_USER_IMPERSONATE_NAME    TEXT("SeDelegateSessionUserImpersonatePrivilege")

// begin_ntosifs

//
// List Of String Capabilities.
//
#define SE_ACTIVATE_AS_USER_CAPABILITY L"activateAsUser"
#define SE_CONSTRAINED_IMPERSONATION_CAPABILITY L"constrainedImpersonation"
#define SE_SESSION_IMPERSONATION_CAPABILITY L"sessionImpersonation"
#define SE_MUMA_CAPABILITY L"muma"
#define SE_DEVELOPMENT_MODE_NETWORK_CAPABILITY L"developmentModeNetwork"
#define SE_PERMISSIVE_LEARNING_MODE_CAPABILITY L"permissiveLearningMode"

// end_ntosifs

typedef DWORD(__stdcall* NtTerminateProcess)(HANDLE, UINT); // __stdcall��WINAPI 
typedef DWORD(WINAPI* NtSuspendProcess)(HANDLE ProcessHandle);
typedef DWORD(WINAPI* NtResumeProcess)(HANDLE hProcess);

int TerminateProcessByPid(void);
int SuspendProcessByPid(void);
int ResumeProcessByPid(void);
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
int GetPidByProcessName(void);

HMODULE hDll = GetModuleHandleA("NtDll.dll");
DWORD processPid = 0;         // ����pid
char processName[256] = { 0 };       // ��������
unsigned int countProcess = 0;       // ��������
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
        prints(pidStr, YELLOW);
        prints(" successfully", RESET);
        return 0;
    }
    prints("ERROR: ", RED);
    prints("Process Not Found", RESET);
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
        prints(pidStr, YELLOW);
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
    HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // ����

    if (hProcess == INVALID_HANDLE_VALUE)
    {
        prints("ERROR: ", RED);
        prints("Process snapshot failed", RESET);
        return -1;
    }

    bool bMore = Process32First(hProcess, &currentProcess);    //��ȡ��һ��������Ϣ
    while (bMore)
    {
        processList[countProcess].pPID = currentProcess.th32ProcessID;
        strcpy_s(processList[countProcess].pName, currentProcess.szExeFile);
        //processList[countProcess].autoTrans();         // �Զ�ת��pid�ַ���
        //printf("PID=%5u    PName= %s\n", currentProcess.th32ProcessID, currentProcess.szExeFile);
        bMore = Process32Next(hProcess, &currentProcess);    //������һ��
        countProcess++;
    }

    if (sort_or_not == true)
        sort(processList, processList + countProcess, ReadFullProcessListCompareRule);

    CloseHandle(hProcess);    //���hProcess���
    return 0;
}

int GetProcessFullImageName(LPSTR returnName, PDWORD returnNameLength) 
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processPid);
    return (QueryFullProcessImageName(hProcess, 0, returnName, returnNameLength)); // ��ȡ�������������� ʧ�ܷ���0
}

void GetPidByProcessName()
{
    GetFullProcessList(true);
    for (int i = 0; i < countProcess; i++)
    {
        if (STRCMP(processList[i].pName, processName))
        {
            processPid = processList[i].pPID;
        }
    }
}

int RestartProcessByPid()
{
    GetPidByProcessName();

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processPid);
    LPSTR imageFileName = (LPSTR)malloc(sizeof(char) * MAX_PATH);
    PROCESS_INFORMATION processInfo;

    GetProcessImageFileName(hProcess, imageFileName, sizeof(imageFileName));

    TerminateProcessByPid();
    if (CreateProcess
         (imageFileName, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, NULL, &processInfo)
          != 0)
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

    free(imageFileName);

    return 0;
}