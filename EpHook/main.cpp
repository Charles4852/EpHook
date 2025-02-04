#include <windows.h>
#include <iostream>

#ifdef _DEBUG
#pragma comment(lib,"../x64/Debug/StaticDll.lib")
#else
#pragma comment(lib,"../x64/Release/StaticDll.lib")
#endif
DECLSPEC_IMPORT int X;

int main(int argc, char* argv[]) {
	printf("argv[0] = \n");
	printf("%s\n", argv[0]);
	return X;
}


