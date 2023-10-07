
#include <stdint.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include "memory.h"

#define EMU_PTR 0x432058

static uint32_t emuoffset = 0;
static HANDLE emuhandle = NULL;

static uintptr_t modBaseAddr;

uint8_t off = 0;


uint8_t MEM_Init(void);
void MEM_Quit(void);
void MEM_UpdateEmuoffset(void);
int32_t MEM_ReadInt(const uint32_t addr);
float MEM_ReadFloat(const uint32_t addr);
void MEM_WriteInt(const uint32_t addr, uint32_t value);
void MEM_WriteFloat(const uint32_t addr, float value);


DWORD GetProcId(const wchar_t* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_wcsicmp(procEntry.szExeFile, procName))
				{
					procId = procEntry.th32ProcessID;
					//std::cout << procId << std::endl;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);

	return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t moduleBaseAddress = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 moduleEntry;
		moduleEntry.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnap, &moduleEntry))
		{
			do
			{
				if (!_wcsicmp(moduleEntry.szModule, modName))
				{
					moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &moduleEntry));
		}
	}
	CloseHandle(hSnap);

	return moduleBaseAddress;
}


//==========================================================================
// Purpose: initialize dolphin handle and setup for memory injection
// Changed Globals: emuhandle
//==========================================================================
uint8_t MEM_Init(void)
{
	const wchar_t gameName[] = L"Supermodel.exe";
	emuhandle = NULL;
	DWORD procID = GetProcId(gameName);
	emuhandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, procID);
	modBaseAddr = GetModuleBaseAddress(procID, gameName);
	return emuhandle != NULL ? 1 : 0;
}
//==========================================================================
// Purpose: close emuhandle safely
// Changed Globals: emuhandle
//==========================================================================
void MEM_Quit(void)
{
	if(emuhandle != NULL)
		CloseHandle(emuhandle);
}


void MEM_UpdateEmuoffset(void)
{
	ReadProcessMemory(emuhandle, (LPVOID)(modBaseAddr + EMU_PTR), &emuoffset, sizeof(emuoffset), NULL);;
}

static void MEM_ByteSwap32(uint32_t* input)
{
	const uint8_t* inputarray = ((uint8_t*)input); // set byte array to input
	*input = (uint32_t)((inputarray[0] << 24) | (inputarray[1] << 16) | (inputarray[2] << 8) | (inputarray[3])); // reassign input to swapped value
}


int32_t MEM_ReadInt(const uint32_t addr)
{
	int32_t output;
	ReadProcessMemory(emuhandle, (LPVOID)(emuoffset + addr), &output, sizeof(output), NULL);
	return output;
}


float MEM_ReadFloat(const uint32_t addr)
{
	float output;
	ReadProcessMemory(emuhandle, (LPVOID)(emuoffset + addr), &output, sizeof(output), NULL);
	return output;
}


void MEM_WriteInt(const uint32_t addr, uint32_t value)
{
	WriteProcessMemory(emuhandle, (LPVOID)(emuoffset + addr), &value, sizeof(value), NULL);
}

void MEM_PatchWord(const uint32_t addr, uint32_t value)
{
	//MEM_ByteSwap32(&value);
	WriteProcessMemory(emuhandle, (LPVOID)(emuoffset + addr), &value, sizeof(value), NULL);
}


void MEM_WriteFloat(const uint32_t addr, float value)
{
	WriteProcessMemory(emuhandle, (LPVOID)(emuoffset + addr), &value, sizeof(value), NULL);
}

void MEM_WriteByte(const uint32_t addr, uint8_t value)
{
	WriteProcessMemory(emuhandle, (LPVOID)(emuoffset + addr), &value, sizeof(value), NULL);
}

uint8_t MEM_ReadByte(const uint32_t addr)
{
	uint8_t output;
	ReadProcessMemory(emuhandle, (LPVOID)((emuoffset) + addr), &output, sizeof(output), NULL);
	return output;
}
