#include <filesystem>
#include "pe.h"
#include "art.h"

//defs
__MIDL_DECLSPEC_DLLEXPORT int X = EXIT_SUCCESS;
HANDLE console_handle = INVALID_HANDLE_VALUE;


#pragma section(".text")
__declspec(allocate(".text")) unsigned char enjoy_bytes[] = { 0x48,0x89,0xC1,//mov rcx,rax
                                                              0xC3 };        //ret

void NtLog(_In_z_ PCSTR fmt, ...) {
    assert(console_handle != INVALID_HANDLE_VALUE);
    char buffer[0x2000];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    IO_STATUS_BLOCK isb{};
    NTSTATUS status = NtWriteFile(
        console_handle,
        NULL,
        NULL,
        NULL,
        &isb,
        buffer,
        len,
        NULL,
        NULL
    );
}

int HookEntryPoint(void(*entry_point)()) {
    NtLog("Called HookThreadCreateFunction.\n");

    NtLog("Call original entry point? Y/N : ");
    auto result = getchar();

    NtLog("\n");
    if (result == 'y' || result == 'Y') {
#ifdef _DEBUG
        HMODULE hucrt = GetModuleHandleW(L"ucrtbased.dll");
#else
        HMODULE hucrt = GetModuleHandleW(L"ucrtbase.dll");
#endif
        if (!hucrt) {
            return EXIT_FAILURE;
        }
        auto __p__acmdln = (char** (*)())GetProcAddress(hucrt, "__p__acmdln");
        assert(__p__acmdln);

        auto p_command = __p__acmdln();
        p_command[0] = (char*)ascii_art;
        entry_point();
    }
    return EXIT_SUCCESS;
}


void ContextTroll(_Inout_ PCONTEXT ctx) {

    auto peb = reinterpret_cast<Peb64*>(ctx->Rdx);
    console_handle = peb->ProcessParameters->StandardOutput;

    //Rip == ntdll!RtlUserThreadStart
    //Rcx == exe.entry_point
    ctx->Rip = (DWORD64)HookEntryPoint;

    //Since the return address [rsp] for RtlUserThreadStart is NULL, it needs to be properly set
    *(PDWORD64)ctx->Rsp = (DWORD64)enjoy_bytes;
    *(PDWORD64)(ctx->Rsp + 8) = (DWORD64)RtlExitUserProcess;

    NtLog("%ls Hook completed. \n", 
        std::filesystem::path(peb->ProcessParameters->ImagePathName.Buffer).filename().c_str());
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       _Inout_ PCONTEXT lpReserved //lpReserved is null when loaded dynamically, therefore static loading is required
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        assert(lpReserved);
        LdrDisableThreadCalloutsForDll(hModule);
        ContextTroll(lpReserved);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

