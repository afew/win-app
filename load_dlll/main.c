
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>


#include <stdio.h>

#ifdef _WIN64
  #define __EXT __declspec(dllexport)
#elif _WIN32
  #define __EXT __declspec(dllexport)
#else
  #define __EXT extern
#endif

static void (*callback)(int value) = NULL;

__EXT const char *rs_str(void) {
  return "Hello World";
}

__EXT int rs_trigger(int val) {
  printf("Trigger called\n");
  if (val == 1) {
    printf("Invoking callback\n");
    if (callback != NULL) {
      (*callback)(100);
    }
    else {
      printf("No callback bound\n");
    }
  }
  else {
    printf("Nope\n");
  }
  return val + 100;
}

__EXT void rs_register(void *cb) {
  printf("Register called\n");
  callback = cb;
}


static void WinErrMsg()
{
	LPTSTR str_msg={0};

	DWORD hr = GetLastError();
	FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
					, NULL
					, hr
					, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
					, (LPTSTR)&str_msg
					, NULL
					, NULL );
	printf("err: %s\n", str_msg);

	LocalFree(str_msg);
}


int main(int argc, char** argv)
{
	char dll_file[] = "E:\\rust_library1.dll";

	FILE* fp = fopen(dll_file, "r");
	fclose(fp);

	HMODULE h_dll = LoadLibrary(dll_file);

	if(NULL == h_dll)
	{
		WinErrMsg();
	}

	void* h_func = GetProcAddress(h_dll, "helloworld");
	FreeLibrary(h_dll);
	return 0;
}