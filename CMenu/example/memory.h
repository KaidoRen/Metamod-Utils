#pragma once

#ifndef _MEMORY_H_
#define _MEMORY_H_

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>

//#ifdef _MSC_VER <= 1600
//	#define PSAPI_VERSION 1
//#endif

#else
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <link.h>

#define Align(addr)	((void *)(((long)addr) & ~(sysconf(_SC_PAGESIZE) - 1)))
#endif

#define T_DEFAULT(a)\
{\
	(void *)NULL,(void *)a,{"","",0},0,0,"","",{},{}\
}

#ifdef _MSC_VER
#pragma comment(lib,"psapi.lib")
#endif

#ifndef dword
#define dword unsigned long
#endif

#pragma pack(push,1)
struct jmp_t
{
	unsigned char _jmp;
	void *addr;
};
#pragma pack(pop)

struct lib_t
{
	char *base;
	void *handler;
	dword size;
};

struct sig_t
{
	char *name;
	char *str;
	size_t len;
};

struct hooks_t
{
	void *addr;
	void *handler;

	sig_t sig;
	size_t offset;
	size_t hook;

	const char *classname;
	const char *name;

	jmp_t jmpBytes;
	unsigned char originalBytes[sizeof(jmp_t)];
};

int lib_load_info(void *addr, lib_t *lib);

void mem_patch(hooks_t *a);
void mem_unpatch(hooks_t *a);

int mem_add_hook_virtual(hooks_t *a);
int mem_remove_hook_virtual(hooks_t *a);

char *lib_find_pattern(lib_t *lib, const char *pattern, int len);
char *lib_find_pattern_fstr(lib_t *lib, const char *string, int range, const char *pattern, int len);
char *mem_find_ref(char *start, int range, int opcode, dword ref, int relative);

int mem_add_hook(hooks_t *a, void *addr);
int mem_add_hook(hooks_t *a, void *addr, void *handler);

int mem_change_protection(void *addr);
int mem_change_protection(void *addr, void *patch, int len);

#ifndef _WIN32
char *lib_find_symbol(lib_t *lib, const char *symbol);
#endif

#endif //_MEMORY_H_

