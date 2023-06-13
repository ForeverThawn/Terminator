#pragma once
#include <Psapi.h>
#include <Windows.h>
#ifndef PROCESS_STAT_H
#define PROCESS_STAT_H
#pragma comment(lib, "psapi.lib")

typedef long long int64_t;
typedef unsigned long long uint64_t;

/// ��ȡ��ǰ���̵�cpuʹ���ʣ�����-1ʧ��
int get_cpu_usage();

/// ��ȡ��ǰ�����ڴ�������ڴ�ʹ����������-1ʧ�ܣ�0�ɹ�
int get_memory_usage(uint64_t* mem, uint64_t* vmem);

/// ��ȡ��ǰ�����ܹ�����д��IO�ֽ���������-1ʧ�ܣ�0�ɹ�
int get_io_bytes(uint64_t* read_bytes, uint64_t* write_bytes);


#endif/*PROCESS_STAT_H*/

/**
* ��Ҫ���ӵ�psapi.lib
*/
#include <Psapi.h>
/// ʱ��ת��
static uint64_t file_time_2_utc(const FILETIME* ftime)
{
    LARGE_INTEGER li;

    assert(ftime);
    li.LowPart = ftime->dwLowDateTime;
    li.HighPart = ftime->dwHighDateTime;
    return li.QuadPart;
}

/// ���CPU�ĺ���
static int get_processor_number()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (int)info.dwNumberOfProcessors;
}

int get_cpu_usage()
{
    //cpu����
    static int processor_count_ = -1;
    //��һ�ε�ʱ��
    static int64_t last_time_ = 0;
    static int64_t last_system_time_ = 0;

    FILETIME now;
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    int64_t system_time;
    int64_t time;
    int64_t system_time_delta;
    int64_t time_delta;

    int cpu = -1;

    if (processor_count_ == -1)
    {
        processor_count_ = get_processor_number();
    }

    GetSystemTimeAsFileTime(&now);

    if (!GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time,
        &kernel_time, &user_time))
    {
        // We don��t assert here because in some cases (such as in the Task

        Manager)
        // we may call this function on a process that has just exited but

            we have
            // not yet received the notification.
            return -1;
    }
    system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time))

        processor_count_;
    time = file_time_2_utc(&now);

    if ((last_system_time_ == 0) || (last_time_ == 0))
    {
        // First call, just set the last values.
        last_system_time_ = system_time;
        last_time_ = time;
        return -1;
    }

    system_time_delta = system_time �C last_system_time_;
    time_delta = time �C last_time_;

    assert(time_delta != 0);

    if (time_delta == 0)
        return -1;

    // We add time_delta / 2 so the result is rounded.
    cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
    last_system_time_ = system_time;
    last_time_ = time;
    return cpu;
}

int get_memory_usage(uint64_t* mem, uint64_t* vmem)
{
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        if (mem) *mem = pmc.WorkingSetSize;
        if (vmem) *vmem = pmc.PagefileUsage;
        return 0;
    }
    return -1;
}

int get_io_bytes(uint64_t* read_bytes, uint64_t* write_bytes)
{
    IO_COUNTERS io_counter;
    if (GetProcessIoCounters(GetCurrentProcess(), &io_counter))
    {
        if (read_bytes) *read_bytes = io_counter.ReadTransferCount;
        if (write_bytes) *write_bytes = io_counter.WriteTransferCount;
        return 0;
    }
    return -1;
}