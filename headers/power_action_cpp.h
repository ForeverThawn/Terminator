#include "forever.h"

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

HMODULE hDll = GetModuleHandleA("NtDll.dll");

int ResetSystem(void);
int PowerOffSystem(void);
int ThrowBlueScreen(void);
int HibernateSystem(void);
int SleepingSystem(void);

int ResetSystem(void)
{
    TYPE_NtInitiatePowerAction NtInitiatePowerAction = (TYPE_NtInitiatePowerAction)GetProcAddress(hDll, "NtInitiatePowerAction");
    prints("Send ", RESET);
    prints("Reset", YELLOW);
    prints(" signal successfully", RESET);
    NtInitiatePowerAction(PowerActionShutdownReset, PowerSystemShutdown, 0, TRUE);
    return 0;
}

int PowerOffSystem(void)
{
    TYPE_NtInitiatePowerAction NtInitiatePowerAction = (TYPE_NtInitiatePowerAction)GetProcAddress(hDll, "NtInitiatePowerAction");
    prints("Send ", RESET);
    prints("PowerOff", YELLOW);
    prints(" signal successfully", RESET);
    NtInitiatePowerAction(PowerActionShutdownOff, PowerSystemShutdown, 0, TRUE);
    return 0;
}

int ThrowBlueScreen(void)
{
    ULONG uResp;
    pdef_NtRaiseHardError ThrowHardError = (pdef_NtRaiseHardError)GetProcAddress(hDll, "NtRaiseHardError");
    prints("Send ", RESET);
    prints("Error", YELLOW);
    prints(" signal successfully", RESET);
    ThrowHardError(STATUS_FLOAT_MULTIPLE_FAULTS, 0, 0, 0, 6, &uResp);
    return 0;
}

int HibernateSystem(void)
{
    TYPE_NtInitiatePowerAction NtInitiatePowerAction = (TYPE_NtInitiatePowerAction)GetProcAddress(hDll, "NtInitiatePowerAction");
    prints("Send ", RESET);
    prints("Hibernate", YELLOW);
    prints(" signal successfully", RESET);
    NtInitiatePowerAction(PowerActionHibernate, PowerSystemHibernate, 0, TRUE);
    return 0;
}

int SleepingSystem(void)
{
    TYPE_NtInitiatePowerAction NtInitiatePowerAction = (TYPE_NtInitiatePowerAction)GetProcAddress(hDll, "NtInitiatePowerAction");
    prints("Send ", RESET);
    prints("Sleep", YELLOW);
    prints(" signal successfully", RESET);
    NtInitiatePowerAction(PowerActionSleep, PowerSystemSleeping1, 0, TRUE);
    return 0;
}