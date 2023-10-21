#include <Windows.h>

#include "ModUtils.h"
#include "ini.h"

using namespace mINI;
using namespace ModUtils;
using namespace std;

const string author = "SchuhBaum";
const string version = "0.0.1";

DWORD WINAPI MainThread(LPVOID lpParam) {
    Log("author " + author);
	Log("version " + version);
    
    // vanilla bug:
    // when you don't move the camera with the mouse then you get a brief auto
    // rotation (lean effect?) every time you move your character left or right;
    // the rotation variable of the camera does not change; this is why the auto
    // rotation option in the menu does not help here; I checked using a cheat
    // table that can alter and show camera variables;
    
    // vanilla:
    // the variable at [rcx+00000140] has to do with moving the camera; in that case 
    // it is set to one; when you stop moving the camera then it decreases to zero
    // after a couple of seconds; when it hits zero then the bug happens;
    // F3 0F11 89 40010000      --  movss [rcx+00000140],xmm1
    // 76 06                    --  jna <current_address + 06> (I think)
    // 89 B9 40010000           --  mov [rcx+00000140],edi
    vector<uint16_t> vanilla = { 0xF3, 0x0F, 0x11, 0x89, 0x40, 0x01, 0x00, 0x00, 0x76, 0x06, 0x89, 0xB9, 0x40, 0x01, 0x00, 0x00 };
    
    // modded:
    // C7 81 40010000 0000803F  --  mov [rcx+00000140],3F800000 where (float)1 = 0x3F800000
    // 90 90 90 90 90 90        --  6 times nop
    vector<uint8_t> modded = { 0xC7, 0x81, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

	uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location == 0) {
        Log("Assembly changes could not be applied.");
        CloseLog();
	    return 0;
    }
    
	Replace(assembly_location, vanilla, modded);
	CloseLog();
	
    // are the return values used for anything?;
    return 1;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
	if (reason != DLL_PROCESS_ATTACH) return 0;
	DisableThreadLibraryCalls(module);
	CreateThread(0, 0, &MainThread, 0, 0, NULL);
	return 1;
}
