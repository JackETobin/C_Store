#include "c_store.h"

static uint32 unresolvedCollisions = 0;

static uint32 globalNumKeyRef;
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


static inline uint32 __NameLen(const char* name)
{
	uint32 nameLen = 0;
	while (*(name + nameLen) != '\0')
		nameLen++;
	return nameLen;
}


static inline ret_code __NameCheck(char* pName, char* pKeyName)
{
	uint8 i = 0;
	ret_code retCode = RET_DUPLICATE_NAME_FND;
	retCode += (!*pName || !*pKeyName) * (RET_NAMEMSSING_FAILURE - retCode);

	do
		retCode += (*(pName + i) != *(pKeyName + i)) * (RET_NAMEMSSING_FAILURE - retCode);
	while (*(pName + i++) != '\0' && retCode == RET_DUPLICATE_NAME_FND);

	return retCode;
}


static inline uint64 __Pow(uint32 input, uint32 pow)
{
	uint64 output = 1;
	for (uint32 i = 0; i < pow; i++)
		output *= input;
	return output;
}


uint8 static inline __NumDigits(uint64 input)
{
	uint8 numDigits = 0;
	while (input)
	{
		input /= 10;
		numDigits++;
	}
	return numDigits;
}


uint32 NumKeys()		//REMINDER: CANNOT INLINE EXTERNALLY CALLED FUNCTION, DON'T DO IT!!
{
	if (!activeKeyring)
		return RET_KYRNGMSSNG_FAILURE;

	void* keysEnd = (void*)activeKeyring->pActiveStash - (activeKeyring->numStashes - 1);
	uint32 numKeys = ((void*)keysEnd - (void*)activeKeyring->keys) / sizeof(key);
	return numKeys;
}


static ret_code INDEX_REDUCE(uint64 indexSeed, uint64* pIndex)
{
	uint32 index = indexSeed;
	uint8 numDigits = __NumDigits(indexSeed);
	uint32 numKeys = globalNumKeyRef;
	while (index > numKeys || ((activeKeyring->keys + index)->data && numDigits >= 2))
	{
		index = (numDigits > 2) * (index % __Pow(10, numDigits - 1)) / 10;
		numDigits -= 2;
	}
	*pIndex = index;
	return (((activeKeyring->keys + index)->data > 0) * RET_COLLISION_DETECTED) + (((activeKeyring->keys + index)->data == 0) * RET_SUCCESS);
}


static ret_code INDEX_REBUILD(uint64 indexSeed, uint64* pNewIndex, uint8 iteration)
{
	if (iteration < REFRESH_ATTEMPT_MAX)
	{
		ret_code retCode = RET_SUCCESS;

		uint64 newIndex = 0;
		indexSeed += 486;
		indexSeed *= indexSeed;
		retCode = INDEX_REDUCE(indexSeed, &newIndex);
		if (retCode == RET_COLLISION_DETECTED)
			retCode = INDEX_REBUILD(indexSeed, &newIndex, ++iteration);

		*pNewIndex = newIndex;
		return retCode;
	}
	return RET_MAXITERLIM_FAILURE;
}


static ret_code INDEX_PRIME(uint64 indexSeed, uint64* pIndex)
{
	uint64 index = indexSeed;
	uint8 numDigits = __NumDigits(indexSeed);
	uint32 numKeys = globalNumKeyRef;
	while (index > numKeys)
	{
		index = (index % __Pow(10, numDigits - 1)) / 10;
		numDigits -= 2;
	}
	*pIndex = index;
	return RET_SUCCESS;
}


static ret_code INDEX_SEED(char* pName, uint64* pIndexSeed)
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


#define IND_REBUILD 1
#define IND_LINEAR 2
ret_code __Success(uint64 noIn, uint64* noOut, uint8 noIter) { return RET_SUCCESS; }
ret_code __INDEX_LINEAR(uint64 noIn, uint64* indexOut, uint8 noIter)
{
	ret_code retCode = RET_MAXITERLIM_FAILURE;
	uint64 index = globalNumKeyRef;
	while (retCode != RET_SUCCESS && index--)
		retCode += (!((key*)activeKeyring->keys + index)->data) * (RET_SUCCESS - retCode);

	*indexOut = index;
	return retCode;
}


static ret_code(*indexFuncPtrs[3])(uint64, uint64*, uint8) =
{ __Success, INDEX_REBUILD, __INDEX_LINEAR };
static ret_code __Index__Get(char* pName, uint64 indexSeed, key** pKey)
{
	uint64 index = 0;

	ret_code retCode = INDEX_REDUCE(indexSeed, &index);
	retCode = indexFuncPtrs[(retCode != RET_SUCCESS) * IND_REBUILD](indexSeed, &index, 0);
	retCode = indexFuncPtrs[(retCode != RET_SUCCESS) * IND_LINEAR](indexSeed, &index, 0);

	*pKey = (key*)activeKeyring->keys + index;
	return retCode += (retCode != RET_SUCCESS) * (RET_KEYSTORAGE_FAILURE - retCode);
}


static void __Keyring__Clear()
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


static ret_code __Keyring__Build(uint64 keyringSize)
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


static ret_code __Keyring__AddStash(stash_handle stash)
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
	globalNumKeyRef = NumKeys() - 1;
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


ret_code __AllClear(char* pName)
{
	ret_code retCode = RET_SUCCESS;
	retCode += (!pName)
		* (RET_NAMENOTFND_FAILURE - retCode);												//Check for name ptr

	retCode += (retCode == RET_SUCCESS)
		* ((!activeKeyring) * RET_KYRNGMSSNG_FAILURE);										//Check for keyring ptr

	retCode += (retCode == RET_SUCCESS)
		* ((!*(stash_handle*)activeKeyring->pActiveStash) * RET_ASTASHMSNG_FAILURE);		//Check for stash ptr

	char* validName[] = { pName, "" };	//Array for case of null name ptr, indexed by conditional statement
	retCode += (retCode == RET_SUCCESS)
		* ((__NameLen(validName[(!pName)]) >= MAX_NAME_LENGTH) * RET_NMAXLENGTH_FAILURE);	//Check name length

	return retCode;
}


static ret_code __DuplicateCheck(char* pName, uint64 indexPrime, key** pRetKey)
{
	ret_code retCode = RET_SUCCESS;

	key* keySet = NULL;
	key* keyLinkIterator = NULL;

	keyLinkIterator = activeKeyring->keys + indexPrime;
	while (keyLinkIterator)
	{
		keySet = keyLinkIterator;
		retCode = __NameCheck(pName, keySet->name);
		if (retCode == RET_DUPLICATE_NAME_FND)
			break;
		keyLinkIterator = keyLinkIterator->keyLink;
	}
	*pRetKey = keySet;
	return retCode;
}


static ret_code __StoreKey(char* pName, data_type type)
{
	uint64 indexSeed = 0;
	uint64 indexPrime = 0;
	key* pKeySet = NULL;
	key* pKeyActual = NULL;

	ret_code retCode = INDEX_SEED(pName, &indexSeed);
	retCode = INDEX_PRIME(indexSeed, &indexPrime);

	retCode = __DuplicateCheck(pName, indexPrime, &pKeySet);
	if (retCode == RET_DUPLICATE_NAME_FND)
		return retCode;

	retCode = __Index__Get(pName, indexSeed, &pKeyActual);
	if (retCode != RET_SUCCESS)
		return retCode;

	key* toStore = pKeyActual;
	key* linkChoice[] = { NULL, toStore };
	pKeySet->keyLink = linkChoice[(pKeySet != toStore)];
	uint8 i;
	for (i = 0; *(pName + i) != '\0'; i++)
		toStore->name[i] = *(pName + i);
	toStore->name[i + 1] = '\0';
	toStore->primeIndex = indexPrime;
	toStore->type = type;
	toStore->data = (*(stash_handle*)activeKeyring->pActiveStash)->next + POINTER_SIZE;

	*(void**)(*(stash_handle*)activeKeyring->pActiveStash)->next = &toStore->data;
	(*(stash_handle*)activeKeyring->pActiveStash)->next += POINTER_SIZE;

	return retCode;
}


static ret_code __GetKey(char* pName, key** pRetKey)
{
	ret_code retCode = RET_SUCCESS;

	uint64 indexSeed = 0;
	uint64 indexPrime = 0;

	retCode = INDEX_SEED(pName, &indexSeed);
	retCode = INDEX_PRIME(indexSeed, &indexPrime);

	__DuplicateCheck(pName, indexPrime, pRetKey);
	return retCode;
}


ret_code __Store(char* name, data_type type, uint64 size, void* input)
{
	if (type != T_PPCHAR && type > T_PCHAR && type < T_UNKNOWN)
		return RET_NONPTRONLY_FAILURE;
	
	ret_code retCode = __StoreKey(name, type);
	activeKeyring->usedKeySlots += (retCode == RET_SUCCESS);

	uint64 sW = (retCode == RET_SUCCESS) * size;
	while(sW--)
		*((char*)(*(stash_handle*)activeKeyring->pActiveStash)->next + sW) = *((char*)input + sW);

	(*(stash_handle*)activeKeyring->pActiveStash)->next += (retCode == RET_SUCCESS) * size;
	return retCode;
}


ret_code __Reserve(char* pName, data_type type, uint64 size, void* null)
{
	if (type < T_PCHAR)
		return RET_RESPTRONLY_FAILURE;

	ret_code retCode = __StoreKey(pName, type);
	if (retCode == RET_SUCCESS)
	{
		*(uint32*)(*(stash_handle*)activeKeyring->pActiveStash)->next = T_RESERVED;
		*(uint64*)((*(stash_handle*)activeKeyring->pActiveStash)->next + INT32_SIZE) = size;
		**(void***)((*(stash_handle*)activeKeyring->pActiveStash)->next - POINTER_SIZE) = 
			(*(stash_handle*)activeKeyring->pActiveStash)->next + INT32_SIZE + INT64_SIZE;
		(*(stash_handle*)activeKeyring->pActiveStash)->next += INT32_SIZE + INT64_SIZE + size;
	}
	return retCode;
}


ret_code __Fill(char* pName, data_type type, uint64 noSize, void* input)
{
	key* pKey;
	__GetKey(pName, &pKey);

	if (*(uint32*)(pKey->data - INT64_SIZE - INT32_SIZE) != T_RESERVED)
		return RET_NORESERVED_FAILURE;
	if (type != pKey->type)
		return RET_TYPEMATCHF_FAILURE;

	uint64 availableSize = *(uint64*)(pKey->data - INT64_SIZE);
	for(uint64 i = 0; i < availableSize; i++)
		*(char*)(pKey->data + i) = *(char*)(input + i);

	return RET_SUCCESS;
}


ret_code __Get(char* pName, data_type type, uint64 size, void* output)
{
	key* pKey;
	__GetKey(pName, &pKey);

	if (type != pKey->type)
		return RET_TYPEMATCHG_FAILURE;

	if(type <= T_LONGDOUBLE)
		for (uint64 i = 0; i < size; i++)
			*((char*)output + i) = *((char*)pKey->data + i);

	if (type > T_LONGDOUBLE && type < T_RESERVED)
	{
		if (type == T_PCHAR || type == T_PPCHAR)
			*(char**)output = (char*)pKey->data;
		else
			*(void**)output = (void*)pKey->data;
	}
	return RET_SUCCESS;
}


ret_code __Remove(char* pName, data_type noType, uint64 noSize, void* null)
{
	key* pKey = NULL;
	ret_code retCode = __GetKey(pName, &pKey);
	if (retCode != RET_DUPLICATE_NAME_FND)
		return retCode;

	void* dataAddr = pKey->data;
	data_type dataType = pKey->type;
	uint64 dataSize =	(dataType < T_SHORT) * INT8_SIZE +
						(dataType >= T_SHORT && dataType < T_INT) * INT16_SIZE +
						(dataType >= T_INT && dataType < T_LONG)* INT32_SIZE +
						(dataType >= T_LONG && dataType < T_LONGLONG) * LONG_SIZE +
						(dataType >= T_LONGLONG && dataType < T_LONGDOUBLE) * INT64_SIZE +
						(dataType == T_LONGDOUBLE) * LDOUB_SIZE +
						(dataType > T_LONGDOUBLE) * (*(uint64*)(pKey->data - INT64_SIZE));

	while (dataSize--)
		*((uint8*)dataAddr + dataSize) = 0;

	for (uint32 i = 0; i < KEY_SIZE; i++)
		*((uint8*)pKey + i) = 0;

	retCode = RET_SUCCESS;
	return retCode;
}


ret_code __FailFunc(char* pName, data_type noType, uint64 noSize, void* null) { return RET_ACTIONREQU_FAILURE; }	//Failure mode in case of undefined action request.
static ret_code(*responsePtrArray[6])(char*, data_type, uint64, void*) =	//Storing pointers to functions once to reduce overhead.
{ __FailFunc, __Store, __Reserve, __Fill, __Get, __Remove };				//Functions stored.
ret_code __MemRequest(char* pName, data_type type, void* container, uint64 sizeItem, uint64 reserveSize, action reqAction)
{
	sizeItem += (reqAction == REQ_RESERVE) * (reserveSize - sizeItem);	//Branchless if(reqAction = REQ_RESERVE) sizeItem = reserveSize;
	ret_code retCode = __AllClear(pName);
	retCode = (responsePtrArray[(retCode == RET_SUCCESS) * reqAction])(pName, type, sizeItem, container);	//Call to response function with respect to action request type via function pointer array.
	
	return retCode;
}
