# How It Works
In static DLL loading, the lpReserved argument of DllMain is of type PCONTEXT. 
This PCONTEXT contains context data used by ntdll!RtlUserThreadStart to start execution at the program's EntryPoint. 
Since the argument of RtlUserThreadStart is RCX = exe.entrypoint, modifying lpReserved->Rcx or lpReserved->Rip allows changing the EntryPoint.
