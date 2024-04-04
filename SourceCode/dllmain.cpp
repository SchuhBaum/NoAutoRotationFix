#include "include/ModUtils.h"
#include "include/ini.h"
#include <chrono>
#include <thread>
#include <Windows.h>

using namespace mINI;
using namespace ModUtils;

const std::string author = "SchuhBaum";
const std::string version = "0.0.3";

DWORD WINAPI MainThread(LPVOID lpParam) {
    // this is for ModEngine2-only users; apparently the first change is not applied
    // otherwise; it reaches the end of scannable memory;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    Log("author " + author);
    Log("version " + version);
    Log_Separator();
    Log_Separator();
    
    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;
    
    // vanilla bug:
    // when you don't move the camera with the mouse then you get a brief auto
    // rotation (lean effect?) every time you move your character left or right; the
    // rotation variable (yaw) of the camera does not change; this is why the auto
    // rotation option in the menu does not help here; I checked using a cheat table
    // that can alter and show camera variables;
    
    // vanilla:
    // the variable at [rcx+00000140] has to do with moving the camera; in that case 
    // it is set to one; when you stop moving the camera then it decreases to zero
    // after like three seconds; when it hits zero then the bug happens;
    // F3 0F11 89 40010000      --  movss [rcx+00000140],xmm1
    // 76 06                    --  jna <current_address + 06> (I think)
    // 89 B9 40010000           --  mov [rcx+00000140],edi
    //
    // modded:
    // C7 81 40010000 0000803F  --  mov [rcx+00000140],3F800000 where (float)1 = 0x3F800000
    // 90 90 90 90 90 90        --  6 times nop
    //
    // 0x3F800000 = (0)(0111111 1)(0000000 00000000 00000000) binary; as a float this gives
    // 1 = (-1)^0 * 2^(01111111 - offset) * (1 + 0*2^-1 + 0*2^-2 + ...) where offset = 127 = 0111111

    vanilla = "f3 0f 11 89 40 01 00 00 76 06 89 b9 40 01 00 00";
    modded = "c7 81 40 01 00 00 00 00 80 3f 90 90 90 90 90 90";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);
    
    Log_Separator();
    
    // when you lock-on then the same variable as above gets set to zero => replace
    // with nops; otherwise you get the same brief auto rotation;
    
    // vanilla: 
    // 74 0C                    --  je <current_address + 0C>
    // 48 89 B9 40010000        --  mov [rcx+00000140],rdi              <-- replace
    // E9 DD040000              --  jmp <current_address + 4DD> (I think)
    vanilla = "74 0c 48 89 b9 40 01 00 00 e9 dd 04 00 00";
    modded = "74 0c 90 90 90 90 90 90 90 e9 dd 04 00 00";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);
    
    Log_Separator();
    Log_Separator();
    
    // are the return values used for anything?;
    CloseLog();
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
    // someone wrote online that some processes might crash when returning false;
    if (reason != DLL_PROCESS_ATTACH) return true;
    DisableThreadLibraryCalls(module);
    CreateThread(0, 0, &MainThread, 0, 0, NULL);
    return true;
}
