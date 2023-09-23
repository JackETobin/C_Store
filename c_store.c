#include "c_store.h"

static uint32 unresolvedCollisions = 0;

static keyring_handle activeKeyring;
static void (*pCallbackFunc)(errpack errorPackage);


void __ErrorCatch(ret_code code, uint32 line, const char* file, void* optional)
{
	if (pCallbackFunc && code != RET_SUCCESS)
	{
		errpack errorPackage = { 0 };
		errorPackage.code = code / 10;
		errorPackage.severity = code % 10;
		errorPackage.rawErr = code;
		errorPackage.line = line;
		errorPackage.file = file;
		errorPackage.pKeyring = activeKeyring;
		if (optional)
			errorPackage.optional = optional;

		pCallbackFunc(errorPackage);
	}
}


typedef enum _allocation_style
{
	MEM_ALL_TOSTASH = 0x0,
	MEM_ALL_ENCOMPASING
}alloc_style;
#define DEFAULT_ALLOC_STYLE MEM_ALL_TOSTASH
static uint8 firstAllocStyle = DEFAULT_ALLOC_STYLE;
ret_code __SetMemContained(void)
{
	firstAllocStyle = MEM_ALL_ENCOMPASING;
	return RET_MEMSTYLE_CONTAIN;
}


uint32 __NameLen(const char* name)
{
	uint32 nameLen = 0;
	while (*(name + nameLen) != '\0')
		nameLen++;
	return nameLen;
}


uint64 __NameSum(char* pName)
{
	uint64 retSum = 0;
	while (*(pName) != '\0')
		retSum += *(pName++);
	return retSum;
}


uint64 __ChunkProd(char* pName)
{
	uint64 retProd = 1;
	while (*(pName) != '\0')
		retProd *= *(pName++);
	return retProd;
}


inline ret_code __DuplicateCheck(char* pName, char* pKeyName)
{
	if(!*pName || !*pKeyName)
		return RET_NAMEMSSING_FAILURE;
	
	if (__NameLen(pName) != __NameLen(pKeyName))
		return RET_NAMEMSSING_FAILURE;

	for (uint8 i = 0; *(pName + i) != '\0'; i++)
		if (*(pName + i) != *(pKeyName + i))
			return RET_NAMEMSSING_FAILURE;

	return RET_DUPLICATE_NAME_FND;
}


uint64 __Pow(uint32 input, uint32 pow)
{
	uint64 output = 1;
	for (uint32 i = 0; i < pow; i++)
		output *= input;
	return output;
}


uint8 __NumDigits(uint64 input)
{
	uint8 numDigits = 0;
	while (input)
	{
		input /= 10;
		numDigits++;
	}
	return numDigits;
}


uint32 NumKeys()
{
	if (!activeKeyring)
		return RET_KYRNGMSSNG_FAILURE;

	void* keysEnd = (void*)activeKeyring->pActiveStash - (activeKeyring->numStashes - 1);
	uint32 numKeys = ((void*)keysEnd - (void*)activeKeyring->keys) / sizeof(key);
	return numKeys;
}


ret_code INDEX_REDUCE(char* pName, uint64 indexSeed, uint64* pIndex, action reqType)
{
	data_type duplicateCheck;
	uint32 index = indexSeed;
	uint8 numDigits = __NumDigits(indexSeed);
	uint32 numKeys = NumKeys();
	while (index > numKeys || ((activeKeyring->keys + index)->data && numDigits >= 2))
	{
		if (index < numKeys && (activeKeyring->keys + index)->data)
			if (__DuplicateCheck(pName, ((key*)activeKeyring->keys + index)->name) == RET_DUPLICATE_NAME_FND)
			{
				*pIndex = index;
				return RET_DUPLICATE_NAME_FND;
			}
		index = (numDigits > 2) * (index % __Pow(10, numDigits - 1)) / 10;
		numDigits -= 2;
	}
	*pIndex = index;
	return (((activeKeyring->keys + index)->data > 0) * RET_COLLISION_DETECTED) + (((activeKeyring->keys + index)->data == 0) * RET_SUCCESS);
}


ret_code INDEX_REBUILD(char* pName, uint64 indexSeed, uint64* pNewIndex, uint8 iteration, action reqType)
{
	if (iteration < REFRESH_ATTEMPT_MAX)
	{
		ret_code retCode = RET_SUCCESS;

		uint64 newIndex = 0;
		indexSeed += 486;
		indexSeed *= indexSeed;
		retCode = INDEX_REDUCE(pName, indexSeed, &newIndex, reqType);
		if (retCode == RET_COLLISION_DETECTED)
			retCode = INDEX_REBUILD(pName, indexSeed, &newIndex, ++iteration, reqType);

		*pNewIndex = newIndex;
		return retCode;
	}
	return RET_MAXITERLIM_FAILURE;
}


ret_code INDEX_SEED(char* pName, uint64* pIndexSeed)
{
	char* c = pName;
	uint8 strLen = 0;
	uint64 indexSeed = 0;
	while (*(c + strLen) != '\0')
		indexSeed += *(c + strLen++) - CHAR_VAL_OFFSET;
	indexSeed = (indexSeed * strLen) + ((strLen == 1) * ((MAX_NAME_LENGTH - 1) * ((*c) - CHAR_VAL_OFFSET)));
	uint8 i, j;
	int8 numFullReps = (MAX_NAME_LENGTH - strLen) / ((strLen - 1) + (strLen == 1));
	int8 numPartReps = (MAX_NAME_LENGTH - strLen) - ((strLen - 1) * numFullReps);
	for (i = 0, j = i + 1; *(pName + j) != '\0'; i++, j++)
	{
		indexSeed +=		//BRANCHLESS IF STATEMENT FOR OVERLAP DISTRIBUTION
			(numPartReps > 0) * (numFullReps + 1) * ((*(pName + i) > *(pName + j)) * (*(pName + i) - *(pName + j)) + (*(pName + i) < *(pName + j)) * (*(pName + j) - *(pName + i))) +
			(numPartReps <= 0) * (numFullReps) * ((*(pName + i) > *(pName + j)) * (*(pName + i) - *(pName + j)) + (*(pName + i) < *(pName + j)) * (*(pName + j) - *(pName + i)));
		numPartReps--;
	}

	*pIndexSeed = indexSeed;
	return RET_SUCCESS;
}


ret_code __Index__Get(char* pName, uint32* pIndex, action reqType)
{
	*pIndex = -1;
	uint64 index = 0;
	uint64 indexSeed = 0;
	ret_code retCode = RET_SUCCESS;
	retCode = INDEX_SEED(pName, &indexSeed);
	retCode = INDEX_REDUCE(pName, indexSeed, &index, reqType);

	if (retCode == RET_DUPLICATE_NAME_FND || retCode == RET_SUCCESS)
	{
		*pIndex = index;
		return retCode;
	}
	
	retCode = INDEX_REBUILD(pName, indexSeed, &index, 0, reqType);
	if (retCode == RET_DUPLICATE_NAME_FND || retCode == RET_SUCCESS)
	{
		*pIndex = index;
		return retCode;
	}

	unresolvedCollisions += (reqType == REQ_STORE);
	index = NumKeys() - 1;
	for (; index >= 0; index--)
	{
		if (reqType == REQ_STORE && !((key*)activeKeyring->keys + index)->data)
		{
			*pIndex = index;
			return RET_SUCCESS;
		}
		retCode = __DuplicateCheck(pName, ((key*)activeKeyring->keys + index)->name);
		if (retCode == RET_DUPLICATE_NAME_FND)
		{
			*pIndex = index;
			return retCode;
		}
	}
	return ((reqType == REQ_STORE) * RET_KEYSTORAGE_FAILURE) + ((reqType == REQ_GET) * RET_NAMEMSSING_FAILURE);
}


void __Keyring__Clear()
{
	if (activeKeyring)
		for (uint8 i = 0; i < activeKeyring->numStashes; i++)
			if (*((stash_handle*)activeKeyring->pActiveStash - i))
			{
				free(*((stash_handle*)activeKeyring->pActiveStash - i));
				*((stash_handle*)activeKeyring->pActiveStash - i) = NULL;
			}

	free(activeKeyring);
}


void __PrintKeys(uint32 line, const char* file)
{
	action printReq = REQ_PRINTKEYS;
	__ErrorCatch(0, line, file, &printReq);
	return;
}


ret_code __Keyring__Build(uint64 keyringSize)
{
	ret_code retCode = RET_SUCCESS;
	if (!keyringSize)
		return retCode;

	activeKeyring = malloc(keyringSize);
	if (!activeKeyring)
		return RET_KYRNGALLOC_FAILURE;
	activeKeyring->pActiveStash = (void*)activeKeyring + keyringSize - POINTER_SIZE;
	activeKeyring->numStashes = STARTING_STASH_COUNT;
	activeKeyring->usedKeySlots = 0;
	activeKeyring->keys = (void*)&activeKeyring->keys + sizeof(key*);

	char* keyringEnd = (char*)activeKeyring + keyringSize;
	while (keyringEnd-- > (char*)activeKeyring->keys)
		*keyringEnd = 0x00;

	atexit(__Keyring__Clear);
	return retCode;
}


ret_code __Keyring__AddStash(stash_handle stash)
{
	ret_code retCode = RET_SUCCESS;
	stash_handle lastActive = *(stash_handle*)activeKeyring->pActiveStash;

	uint8 numStashes = activeKeyring->numStashes;
	while (numStashes--)
		*((stash_handle*)activeKeyring->pActiveStash - numStashes) = *((stash_handle*)activeKeyring->pActiveStash - numStashes + 1);
	*(stash_handle*)activeKeyring->pActiveStash = stash;

	if (*((stash_handle*)activeKeyring->pActiveStash - 1) != lastActive)
		retCode = RET_KRSTASHADD_FAILURE;

	return retCode;
}


ret_code __Stash__SysInit(uint64 totalAvailableMB, uint64* stashReserve)
{
	ret_code retCode = RET_SUCCESS;

	uint64 keyringSize = 0;
	if (firstAllocStyle == MEM_ALL_ENCOMPASING)
	{
		//TODO
	}
	else
	{
		keyringSize = sizeof(struct _keyring) + (STARTING_KEY_COUNT * sizeof(key)) + (STARTING_STASH_COUNT * sizeof(stash_handle));
		*stashReserve = 1024 * 1024;
	}
	retCode = __Keyring__Build(keyringSize);
	return retCode;
}


ret_code __Stash__Make(uint64 sizeMB)
{
	ret_code retCode = RET_SUCCESS;
	uint64 sizeBytes = 0;
	if (!activeKeyring)
		retCode = __Stash__SysInit(sizeMB, &sizeBytes);
	else
		sizeBytes = sizeMB * 1024 * 1024;
	stash_handle stash = malloc(sizeBytes);
	if (!stash)
		retCode = RET_STASHALLOC_FAILURE;

	stash->end = (char*)stash + sizeBytes;
	stash->next = (void*)&stash->isFull + sizeof(uint8);
	stash->isFull = 0;

	retCode = __Keyring__AddStash(stash);
	return retCode;
}


ret_code __Stash__Destroy()
{
	if (!activeKeyring)
		return RET_KYRNGMSSNG_FAILURE;
	if (!*(stash_handle*)activeKeyring->pActiveStash)
		return RET_STASHMSSNG_FAILURE;

	for (uint8 i = 0; i < activeKeyring->numStashes; i++)
		if (*((stash_handle*)activeKeyring->pActiveStash - i))
		{
			free(*((stash_handle*)activeKeyring->pActiveStash - i));
			*((stash_handle*)activeKeyring->pActiveStash - i) = NULL;
		}
	for (uint8 i = 0; i < activeKeyring->numStashes; i++)
		if (*((stash_handle*)activeKeyring->pActiveStash - i))
			return  RET_STASHDESTR_FAILURE;

	return RET_SUCCESS;
}


ret_code __Stash__Action(uint64 sizeMB, action reqAction)
{
	ret_code retCode;
	switch (reqAction)
	{
	case REQ_MAKE:
		if (activeKeyring)
		{
			retCode = RET_STASHEXIST_FAILURE;
			break;
		}
		retCode = __Stash__Make(sizeMB);
		break;
	case REQ_DESTROY:
		retCode = __Stash__Destroy();
		break;
	default:
		retCode = RET_ACTIONREQU_FAILURE;
	}
	return retCode;
}

/*
ret_code __NameCheck(char* pName)
{
	char* keyName;
	uint32 numKeys = NumKeys();
	uint32 usedKeys = activeKeyring->usedKeySlots;
	for (uint32 i = 0; i < numKeys && usedKeys > 0; i++)
	{
		if (*((key*)activeKeyring->keys + i)->name)
		{
			if (__DuplicateCheck(pName, ((key*)activeKeyring->keys + i)->name) == RET_DUPLICATE_NAME_FND)
				return RET_DUPLICATE_NAME_FND;
			usedKeys--;
		}
	}
	return RET_SUCCESS;
}
*/

ret_code __AllClear(char* name)
{
	if (!name)
		return RET_NAMENOTFND_FAILURE;
	
	if (!activeKeyring)
		return RET_KYRNGMSSNG_FAILURE;

	if (!*(stash_handle*)activeKeyring->pActiveStash)
		return RET_ASTASHMSNG_FAILURE;

	if (__NameLen(name) >= MAX_NAME_LENGTH)
		return RET_NMAXLENGTH_FAILURE;

	return RET_SUCCESS;
}


ret_code __StoreKey(char* name, data_type type, uint32* indexRet)
{
	uint32 index = 0;
	ret_code retCode = __Index__Get(name, &index, REQ_STORE);
	if (retCode != RET_SUCCESS)
		return retCode;

	if (indexRet)
		*indexRet = index;

	key* toStore = (key*)activeKeyring->keys + index;
	uint8 i;
	for (i = 0; *(name + i) != '\0'; i++)
		toStore->name[i] = *(name + i);
	toStore->name[i + 1] = '\0';
	toStore->type = type;
	toStore->data = (*(stash_handle*)activeKeyring->pActiveStash)->next;

	return retCode;
}

ret_code __Store(char* name, data_type type, uint64 size, void* input)
{
	if (type != T_PPCHAR && type > T_PCHAR && type < T_UNKNOWN)
		return RET_NONPTRONLY_FAILURE;
	
	ret_code retCode = __AllClear(name);
	if (retCode == RET_SUCCESS)
		retCode = __StoreKey(name, type, NULL);

	if (retCode == RET_SUCCESS)
	{
		activeKeyring->usedKeySlots++;
		for (uint64 i = 0; i < size; i++)
			*((char*)(*(stash_handle*)activeKeyring->pActiveStash)->next + i) = *((char*)input + i);

		(*(stash_handle*)activeKeyring->pActiveStash)->next += size;
	}
	
	return retCode;
}


ret_code __Reserve(char* name, data_type type, uint64 size)
{
	if (type < T_PCHAR)
		return RET_RESPTRONLY_FAILURE;

	ret_code retCode = __AllClear(name);
	if (retCode == RET_SUCCESS)
	{
		uint32 index;
		retCode = __StoreKey(name, type, &index);
		if (retCode == RET_SUCCESS)
		{
			*(uint32*)(*(stash_handle*)activeKeyring->pActiveStash)->next = T_RESERVED;
			*(uint64*)((*(stash_handle*)activeKeyring->pActiveStash)->next + INT32_SIZE) = size;
			((key*)activeKeyring->keys + index)->data = (*(stash_handle*)activeKeyring->pActiveStash)->next + INT32_SIZE + INT64_SIZE;
			(*(stash_handle*)activeKeyring->pActiveStash)->next += INT32_SIZE + INT64_SIZE + size;
		}
	}
	return retCode;
}


ret_code __Fill(char* name, data_type type, void* input)
{
	ret_code retCode = __AllClear(name);
	if (retCode != RET_SUCCESS)
		return retCode;

	uint32 index = 0;
	retCode = __Index__Get(name, &index, REQ_GET);
	if (retCode != RET_DUPLICATE_NAME_FND)
		return retCode;

	if (*(uint32*)(((key*)activeKeyring->keys + index)->data - INT64_SIZE - INT32_SIZE) != T_RESERVED)
		return RET_NORESERVED_FAILURE;
	if (type != ((key*)activeKeyring->keys + index)->type)
		return RET_TYPEMATCHF_FAILURE;

	uint64 availableSize = *(uint64*)(((key*)activeKeyring->keys + index)->data - INT64_SIZE);
	for(uint64 i = 0; i < availableSize; i++)
		*(char*)(((key*)activeKeyring->keys + index)->data + i) = *(char*)(input + i);

	retCode = RET_SUCCESS;
	return retCode;
}


ret_code __Get(char* pName, data_type type, uint64 size, void* output)
{
	ret_code retCode = __AllClear(pName);
	if (retCode != RET_SUCCESS)
		return retCode;

	uint32 index = 0;
	retCode = __Index__Get(pName, &index, REQ_GET);
	if (retCode != RET_DUPLICATE_NAME_FND)
		return retCode;

	if (type != ((key*)activeKeyring->keys + index)->type)
		return RET_TYPEMATCHG_FAILURE;

	if(type <= T_LONGDOUBLE)
		for (uint64 i = 0; i < size; i++)
			*((char*)output + i) = *((char*)((key*)activeKeyring->keys + index)->data + i);

	if (type > T_LONGDOUBLE && type < T_RESERVED)
	{
		if (type == T_PCHAR || type == T_PPCHAR)
			*(char**)output = (char*)((key*)activeKeyring->keys + index)->data;
		else
			*(void**)output = (void*)((key*)activeKeyring->keys + index)->data;
	}
	retCode = RET_SUCCESS;
	return retCode;
}


ret_code __Remove(char* pName)
{
	ret_code retCode = __AllClear(pName);
	if (retCode != RET_SUCCESS)
		return retCode;

	uint32 index = 0;
	retCode = __Index__Get(pName, &index, REQ_GET);
	if (retCode != RET_DUPLICATE_NAME_FND)
		return retCode;

	//key activeKey = *(activeKeyring->keys + index);
	void* dataAddr = (activeKeyring->keys + index)->data;
	data_type dataType = (activeKeyring->keys + index)->type;
	uint64 dataSize =	(dataType < T_SHORT) * INT8_SIZE +
						(dataType >= T_SHORT && dataType < T_INT) * INT16_SIZE +
						(dataType >= T_INT && dataType < T_LONG)* INT32_SIZE +
						(dataType >= T_LONG && dataType < T_LONGLONG) * LONG_SIZE +
						(dataType >= T_LONGLONG && dataType < T_LONGDOUBLE) * INT64_SIZE +
						(dataType == T_LONGDOUBLE) * LDOUB_SIZE +
						(dataType > T_LONGDOUBLE) * (*(uint64*)(((key*)activeKeyring->keys + index)->data - INT64_SIZE));

	while (dataSize--)
		*((uint8*)dataAddr + dataSize) = 0;

	for (uint32 i = 0; i < KEY_SIZE; i++)
		*((uint8*)(activeKeyring->keys + index) + i) = 0;

	retCode = RET_SUCCESS;
	return retCode;
}


ret_code __MemRequest(char* pName, data_type type, void* container, uint64 sizeItem, uint64 numItems, action reqAction)
{
	ret_code retCode;
	switch (reqAction)
	{
	case REQ_STORE:
		retCode = __Store(pName, type, sizeItem, container);
		break;
	case REQ_RESERVE:
		retCode = __Reserve(pName, type, numItems);
		break;
	case REQ_FILL:
		retCode = __Fill(pName, type, container);
		break;
	case REQ_GET:
		retCode = __Get(pName, type, sizeItem, container);
		break;
	case REQ_REMOVE:
		retCode = __Remove(pName);
		break;
	default:
		retCode = RET_ACTIONREQU_FAILURE;
	}
	return retCode;
}