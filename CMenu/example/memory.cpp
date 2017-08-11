/*
* Localize Bug Fix
* Copyright (c) 2013 - 2014 AGHL.RU Dev Team
*
* http://aghl.ru/forum/ - Russian Half-Life and Adrenaline Gamer Community
*
*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*    In addition, as a special exception, the author gives permission to
*    link the code of this program with the Half-Life Game Engine ("HL
*    Engine") and Modified Game Libraries ("MODs") developed by Valve,
*    L.L.C ("Valve").  You must obey the GNU General Public License in all
*    respects for all of the code used other than the HL Engine and MODs
*    from Valve.  If you modify this file, you may extend this exception
*    to your version of the file, but you are not obligated to do so.  If
*    you do not wish to do so, delete this exception statement from your
*    version.
*
*/

#include "memory.h"
#include "amxxmodule.h"

#ifdef _WIN32
int lib_load_info(void *addr, lib_t *lib)
{
	MEMORY_BASIC_INFORMATION mem;
	VirtualQuery(addr, &mem, sizeof(mem));

	IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)mem.AllocationBase;
	IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS*)((dword)dos + (dword)dos->e_lfanew);

	if (pe->Signature != IMAGE_NT_SIGNATURE)
		return 0;

	lib->base = (char *)mem.AllocationBase;
	lib->size = (size_t)pe->OptionalHeader.SizeOfImage;

	return 1;
}
#else
char *lib_find_symbol(lib_t *lib, const char *symbol)
{
	return (char *)dlsym(lib->handler, symbol);
}
static ElfW(Addr) dlsize(void *base)
{
	int i;
	ElfW(Ehdr) *ehdr;
	ElfW(Phdr) *phdr;
	ElfW(Addr) end;

	ehdr = (ElfW(Ehdr)*)base;
	phdr = (ElfW(Phdr)*)((ElfW(Addr))ehdr + ehdr->e_phoff);

	for (i = 0; i < ehdr->e_phnum; ++i)
	{
		if (phdr[i].p_type == PT_LOAD)
			end = phdr[i].p_vaddr + phdr[i].p_memsz;
	}
	return end;
}
int lib_load_info(void *addr, lib_t *lib)
{
	Dl_info info;
	if (!dladdr(addr, &info) || !info.dli_fbase || !info.dli_fname)
		return 0;

	lib->base = (char *)info.dli_fbase;
	lib->size = dlsize(info.dli_fbase);
	lib->handler = dlopen(info.dli_fname, RTLD_NOW);

	return 1;
}
#endif
static inline int mem_compare_c(const char *addr, const char *pattern, const char *pattern_end)
{
	const char *c;
	for (c = pattern; c < pattern_end; ++c, ++addr)
	{
		if (*c == *addr || *c == '\x2A')
			continue;

		return 0;
	}
	return 1;
}
static char *mem_find_ref_c(char *pos, char *end, int opcode, dword ref, int relative)
{
	for (; pos < end; ++pos)
	{
		if (*pos == opcode)
		{
			if (relative)
			{
				if ((dword)pos + 5 + *(dword *)(pos + 1) == ref)
					return pos;
			}
			else
			{
				if (*(dword *)(pos + 1) == ref)
					return pos;
			}
		}
	}
	return NULL;
}
static char *mem_find_pattern(char *pos, int range, const char *pattern, int len)
{
	char *end;
	const char *pattern_end;

	pattern_end = pattern + len;

	for (end = pos + range - len; pos < end; ++pos)
	{
		if (mem_compare_c(pos, pattern, pattern_end))
			return pos;
	}
	return NULL;
}
char *mem_find_ref(char *start, int range, int opcode, dword ref, int relative)
{
	return mem_find_ref_c(start, start + range - 5, opcode, ref, relative);
}
char *lib_find_string_push(lib_t *lib, const char *string)
{
	char *addr;
	addr = mem_find_pattern(lib->base, lib->size, string, strlen(string) + 1);
	return mem_find_ref(lib->base, lib->size, '\x68', (dword)addr, 0);
}
char *lib_find_pattern(lib_t *lib, const char *pattern, int len)
{
	return mem_find_pattern(lib->base, lib->size, pattern, len);
}
char *lib_find_pattern_fstr(lib_t *lib, const char *string, int range, const char *pattern, int len)
{
	char *addr;
	addr = lib_find_string_push(lib, string);

	if (addr)
	{
		if (range < 0)
		{
			addr += range;
			range = -range;
		}
		return mem_find_pattern(addr, range, pattern, len);
	}
	return NULL;
}
int mem_remove_hook(hooks_t *a)
{
	if (!a->hook)
		return 0;

	a->hook = 0;
	mem_unpatch(a);

	return 1;
}
void mem_patch(hooks_t *a)
{
	memcpy(a->addr, &(a->jmpBytes), sizeof(jmp_t));
}
void mem_unpatch(hooks_t *a)
{
	memcpy(a->addr, a->originalBytes, sizeof(jmp_t));
}
static int mem_change_protection_c(void *addr, void *patch, int len, int cpy)
{
#ifdef _WIN32
	static HANDLE process = 0;
	DWORD OldProtection, NewProtection = PAGE_EXECUTE_READWRITE;

	if (!process)
		process = GetCurrentProcess();

	FlushInstructionCache(process, addr, len);
	if (VirtualProtect(addr, len, NewProtection, &OldProtection))
	{
		if (cpy)
			memcpy(addr, patch, len);

		return VirtualProtect(addr, len, OldProtection, &NewProtection);
	}
#else
	size_t size = sysconf(_SC_PAGESIZE);
	void *alignedAddress = Align(addr);

	if (Align(addr + len - 1) != alignedAddress)
		size *= 2;

	if (!mprotect(alignedAddress, size, (PROT_READ | PROT_WRITE | PROT_EXEC)))
	{
		if (cpy)
			memcpy(addr, patch, len);

		return !mprotect(alignedAddress, size, (PROT_READ | PROT_EXEC));
	}
#endif
	return 0;
}
int mem_change_protection(void *addr)
{
	return mem_change_protection_c(addr, NULL, 5, 0);
}
int mem_change_protection(void *addr, void *patch, int len)
{
	return mem_change_protection_c(addr, patch, len, 1);
}
static int mem_add_hook_c(hooks_t *a, void *addr, void *handler)
{
	if (!addr)
		return 0;

	a->hook = 1;
	a->addr = addr;
	a->jmpBytes._jmp = 0xe9;
	a->jmpBytes.addr = (void *)((dword)handler - (dword)a->addr - sizeof(jmp_t));

	memcpy(a->originalBytes, a->addr, sizeof(jmp_t));

#ifdef _WIN32
	DWORD Oldp;
	VirtualProtect(a->addr, 5, PAGE_EXECUTE_READWRITE, &Oldp);
#else
	size_t size = sysconf(_SC_PAGESIZE);
	void *alignedAddress = Align(a->addr);
	mprotect(alignedAddress, size, (PROT_READ | PROT_WRITE | PROT_EXEC));
#endif

	return mem_change_protection_c(a->addr, &(a->jmpBytes), sizeof(jmp_t), 1);
}
int mem_add_hook(hooks_t *a, void *addr)
{
	return mem_add_hook_c(a, addr, a->handler);
}
int mem_add_hook(hooks_t *a, void *addr, void *handler)
{
	return mem_add_hook_c(a, addr, handler);
}
static int mem_virtual_addr(hooks_t *a, int revert)
{
	edict_t *pEdict = CREATE_ENTITY();
	CALL_GAME_ENTITY(PLID, a->classname, &pEdict->v);

	if (!pEdict->pvPrivateData)
	{
		REMOVE_ENTITY(pEdict);
		return 0;
	}
	void **vtable = *((void ***)(((char *)pEdict->pvPrivateData)));

	if (!vtable)
		return 0;

	int **ivtable = (int **)vtable;
	int offset = a->offset;

	if (!revert)
		a->addr = (void *)ivtable[offset];

	mem_change_protection(&ivtable[offset]);




	//ivtable[offset] = (int *)(revert ? a->addr : a->handler);

	printf("*** #%d. %s\n", ENTINDEX(pEdict), a->classname);

	REMOVE_ENTITY(pEdict);
	return 1;
}
int mem_add_hook_virtual(hooks_t *a)
{
	if (a->hook)
		return false;

	return a->hook = mem_virtual_addr(a, 0);
}
int mem_remove_hook_virtual(hooks_t *a)
{
	if (!a->hook)
		return 0;

	return !(a->hook = !mem_virtual_addr(a, 1));
}