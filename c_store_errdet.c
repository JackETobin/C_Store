#include "c_store.h"

#ifdef _DEBUG

#include <stdio.h>

static const char* errorCode[] =
{
	"RET_ERCALBKINIT_SUCESS",
	"RET_SUCCESS = 10",
	"RET_MEMSTYLE_CONTAIN",
	"RET_MEMSTYLE_TOSTASH",
	"RET_KYRNGMSSNG_FAILURE",
	"RET_KYRNGALLOC_FAILURE",
	"RET_STASHALLOC_FAILURE",
	"RET_KRSTASHADD_FAILURE",
	"RET_STASHEXIST_FAILURE",
	"RET_STASHEXPAN_FAILURE",
	"RET_ASTASHMSNG_FAILURE",
	"RET_STASHMSSNG_FAILURE",
	"RET_STASHDESTR_FAILURE",
	"RET_ACTIONREQU_FAILURE",
	"RET_TYPEMATCHG_FAILURE",
	"RET_TYPEMATCHF_FAILURE",
	"RET_DUPLICATE_NAME_FND",
	"RET_NMAXLENGTH_FAILURE",
	"RET_NAMEMSSING_FAILURE",
	"RET_KEYSTORAGE_FAILURE",
	"RET_COLLISION_DETECTED",
	"RET_NAMENOTFND_FAILURE",
	"RET_RESPTRONLY_FAILURE",
	"RET_NORESERVED_FAILURE",
	"RET_NONPTRONLY_FAILURE",
	"RET_MAXITERLIM_FAILURE"
};
static const char* callbackMessage[] =
{
	"c_store: Callback system initialized...",
	"c_store: Good Job!",
	"c_store: First allocation style set to contain.",
	"c_store: First allocation style set to default.",
	"c_store: No active key storage system -> call MakeStash(sizeMB).",
	"c_store: Failed to allocate key storage system",
	"c_store: Failed to allocate stash.",
	"c_store: Failed to add new stash to keyring.",
	"c_store: Duplicate call to MakeStash(sizeMB) -> stash already exists.",
	"c_store: Failed to expand stash.",
	"c_store: No active stash -> call MakeStash(sizeMB).",
	"c_store: No stash present to destroy.",
	"c_store: Failed to destroy stash.",
	"c_store: Failed to send memory access request.",
	"c_store: Type missmatch on call to Get(name, output) -> itemContainer wrong type.",
	"c_store: Type missmatch on call to Fill(name, pInput) -> itemContainer wrong type.",
	"c_store: Duplicate item name detected.",
	"c_store: Name length is larger than maximum allowed length.",
	"c_store: No key by that name is present in the keyring.",
	"c_store: Failed to find open slot in the keyring.",
	"c_store: Collision detected, adjusting the index...",
	"c_store: Valid name not proveded.",
	"c_store: Non-pointer passed to Reserve(name, size, preserved) -> please pass a pointer.",
	"c_store: Space not reserved, please call reserve space first.",
	"c_store: Pointer passed to Store(name, input) -> please pass a non-pointer.",
	"c_store: Maximum allowed recursion depth hit."
};

uint32 __namelen(const char* name)
{
	uint32 nameLen = 0;
		while (*(name + nameLen) != '\0')
			nameLen++;
	return nameLen;
}


uint8 __lenComp(const char* pName, const char* pKeyName)
{
	uint8 diff = 0;
	uint8 nameLen = __namelen(pName);
	uint8 keyNameLen = __namelen(pKeyName);

	if (nameLen > keyNameLen)
		diff = nameLen - keyNameLen;
	else
		diff = keyNameLen - nameLen;

	return diff;
}


uint32 __filenamestart(const char* path)
{
	char c = 'a';
	int pathLen = 0;
	while (c != '\0')
		c = *(path + pathLen++);
	while (c != '\\')
		c = *(path + pathLen--);
	pathLen += 2;

	return pathLen;
}


uint8 __detectsimilar(char* pName, char* pKeyName)
{
	uint8 similar = 0;
	uint8 missThreshold = 1;
	if (__lenComp(pName, pKeyName) > (missThreshold * 2))
		return similar;

	uint8 missCount = 0;
	for (uint8 i = 0; *(pName + i) != '\0'; i++)
		if (*(pName + i) != *(pKeyName + i) && 
			*(pName + i) != *(pKeyName + i + 1) &&
			*(pName + i) != *(pKeyName + i - 2) &&
			*(pName + i) != *(pKeyName + i - 1))
			missCount++;
	if (missCount <= missThreshold)
		similar = 1;

	return similar;
}


void __printsimilar(errpack* pErrorPackage)
{
	uint8 similarCount = 0;
	uint32 numKeys = NumKeys();
	keyring_handle pKeyring = pErrorPackage->pKeyring;
	for (uint32 i = 0; i < numKeys; i++)
	{
		if (((key*)pKeyring->keys + i)->data && __detectsimilar((char*) pErrorPackage->optional, ((key*)pKeyring->keys + i)->name))
		{
			if (!similarCount)
			{
				printf("\t Similar keys detected: \"%s\"\n", (char*)((key*)pKeyring->keys + i)->name);
				similarCount++;
				continue;
			}
			printf("\t\t\t\t %s\n", (char*)((key*)pKeyring->keys + i)->name);
		}
	}
}


void __printkeys(errpack* pErrorPackage)
{
	if (!pErrorPackage->pKeyring)
	{
		__ErrorCatch(RET_KYRNGMSSNG_FAILURE, pErrorPackage->line, pErrorPackage->file, NULL);
		return;
	}

	char* keyName;
	uint32 numKeys = NumKeys();
	for (uint32 i = 0; i < numKeys; i++)
	{
		if (*((key*)((keyring_handle)pErrorPackage->pKeyring)->keys + i)->name)
			keyName = ((key*)((keyring_handle)pErrorPackage->pKeyring)->keys + i)->name;
		else
			keyName = "----EMPTY----";
		printf("%i:\t\"%s\"\n", i, keyName);
	}
}


void __error__callback(errpack errorPackage)
{
	if (errorPackage.optional)
		if (*(action*)errorPackage.optional == REQ_PRINTKEYS)
		{
			__printkeys(&errorPackage);
			return;
		}

	char* severityLevel[] =
	{
		"info",
		"warning",
		"error",
		"fatal"
	};
	const char* file = errorPackage.file + __filenamestart(errorPackage.file);
	printf("%s\n", callbackMessage[errorPackage.code]);
	if (errorPackage.optional)
		switch (errorPackage.rawErr)
		{
		case RET_MEMSTYLE_CONTAIN:
			printf("\t All allications will be fit into requested mem size, resulting stash will be smaller than size requested.\n");
			break;
		case RET_TYPEMATCHG_FAILURE:
			printf("\t Failed to retrieve: \"%s\"\n", (char*)errorPackage.optional);
			break;
		case RET_TYPEMATCHF_FAILURE:
			printf("\t Failed to fill: \"%s\"\n", (char*)errorPackage.optional);
			break;
		case RET_NORESERVED_FAILURE:
			printf("\t Call order: Reserve(\"%s\", reserveSize, pReserved) -> -> -> Fill(\"%s\", pInput)\n", (char*)errorPackage.optional, (char*)errorPackage.optional);
			break;
		case RET_DUPLICATE_NAME_FND:
			printf("\t An item with the name \"%s\" already exists, please choose a different name.\n", (char*)errorPackage.optional);
			break;
		case RET_NMAXLENGTH_FAILURE:
			printf("\t Name provided: \"%s\"\tLength: %i\tMax name length: %i\n", (char*)errorPackage.optional, __namelen(errorPackage.optional), MAX_NAME_LENGTH);
			break;
		case RET_NAMEMSSING_FAILURE:
			printf("\t Key tried: \"%s\"\t\t\tResult: Unable to be located.\n", (char*)errorPackage.optional);
			__printsimilar(&errorPackage);
			break;
		case RET_KEYSTORAGE_FAILURE:
			printf("\t Key tried: \"%s\".\n", (char*)errorPackage.optional);
			break;
		}
	printf("\t Code: %s\t\tSeverity: %s\n\t File: %s\t\t\tLine: %i\n\n", errorCode[errorPackage.code], severityLevel[errorPackage.severity], file, errorPackage.line);
	if (errorPackage.severity > ES_ERROR)
		exit(errorPackage.rawErr);
}

#endif

void __InitErrorCallback(void (*pErrFunc)(errpack errorPackage), uint32 line, const char* file)
{
#ifdef _DEBUG
	if (pErrFunc)
		pCallbackFunc = pErrFunc;
	else
		pCallbackFunc = *__error__callback;

	__ErrorCatch(RET_ERCALBKINIT_SUCESS, line, file, NULL);
#endif
}
