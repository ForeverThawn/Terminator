#include "forever.h"
#include <Psapi.h>

/*typedef struct _PERFORMANCE_INFORMATION {
        DWORD  cb;
        UINT64 CommitTotal;
        UINT64 CommitLimit;
        UINT64 CommitPeak;
        UINT64 PhysicalTotal;
        UINT64 PhysicalAvailable;
        UINT64 SystemCache;
        UINT64 KernelTotal;
        UINT64 KernelPaged;
        UINT64 KernelNonpaged;
        UINT64 PageSize;
        DWORD  HandleCount;
        DWORD  ProcessCount;
        DWORD  ThreadCount;
    } PERFORMANCE_INFORMATION, * PPERFORMANCE_INFORMATION, PERFORMACE_INFORMATION, * PPERFORMACE_INFORMATION;*/

int GetSystemMemoryInfo(void);
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

/*
int MemoryAllocByByte(UINT64 bytes)
{
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    if (CreateProcess(NULL, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInfo))
    {

        return 0;
    }
    else
    {
        prints("ERROR: ", RED);
        prints("Process creation failed", RESET);
        return -1;
    }
}
*/