#pragma once

#include <windows.h>

// INTERNAL USE ONLY
// TYPE and NAME fileds should be variable-length, but TYPE is fixed as DWORD size 
// for dialog resources: 0xffff0005
// NAME can be ordinal (WORD size, with 0xffff at high-word) or a UNICODE string
typedef struct res_hdr {
	DWORD DataSize;
	DWORD HeaderSize;
	DWORD TYPE; // 0xffff0005
	DWORD NAME; // 0xffff???? or UNICODE string
	DWORD DataVersion;
	WORD  MemoryFlags; // MOVABLE|PURE|DISCARDABLE
	WORD  LanguageId;
	DWORD Version;
	DWORD Characteristics;

	// internal data fields
	PBYTE data; // DataSize field will work with this field
	struct res_hdr* next;
} RESOURCE_HEADER;
