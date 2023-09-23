#ifndef C_STORE
#define C_STORE

#include <stdlib.h>

#define STARTING_STASH_COUNT 5
#define STARTING_KEY_COUNT 2500

#define CHUNK_SIZE 8
#define CHAR_VAL_OFFSET 31
#define MAX_NAME_LENGTH 24
#define REFRESH_ATTEMPT_MAX 3

typedef		char				int8;
typedef		unsigned char		uint8;
typedef		short				int16;
typedef		unsigned short		uint16;
typedef		int					int32;
typedef		unsigned int		uint32;
typedef		long long			int64;
typedef		unsigned long long	uint64; 
#define UINT64_NUM_DIGITS 20

#define INT8_SIZE sizeof(uint8)
#define LONG_SIZE sizeof(long)
#define INT16_SIZE sizeof(uint16)
#define INT32_SIZE sizeof(uint32)
#define INT64_SIZE sizeof(uint64)
#define LDOUB_SIZE sizeof(long double)
#define POINTER_SIZE sizeof(void*)

typedef enum _error_code
{
	RET_ERCALBKINIT_SUCESS = 00,
	RET_SUCCESS = 10,
	RET_MEMSTYLE_CONTAIN = 20,
	RET_MEMSTYLE_TOSTASH = 30,
	RET_KYRNGMSSNG_FAILURE = 42,
	RET_KYRNGALLOC_FAILURE = 53,
	RET_STASHALLOC_FAILURE = 63,
	RET_KRSTASHADD_FAILURE = 73,
	RET_STASHEXIST_FAILURE = 81,
	RET_STASHEXPAN_FAILURE = 92,
	RET_ASTASHMSNG_FAILURE = 102,
	RET_STASHMSSNG_FAILURE = 111,
	RET_STASHDESTR_FAILURE = 122,
	RET_ACTIONREQU_FAILURE = 132,
	RET_TYPEMATCHG_FAILURE = 142,
	RET_TYPEMATCHF_FAILURE = 152,
	RET_DUPLICATE_NAME_FND = 162,
	RET_NMAXLENGTH_FAILURE = 172,
	RET_NAMEMSSING_FAILURE = 182,
	RET_KEYSTORAGE_FAILURE = 192,
	RET_COLLISION_DETECTED = 200,
	RET_NAMENOTFND_FAILURE = 211,
	RET_RESPTRONLY_FAILURE = 222,
	RET_NORESERVED_FAILURE = 231,
	RET_NONPTRONLY_FAILURE = 242,
	RET_MAXITERLIM_FAILURE = 251
}ret_code;

enum _error_severity
{
	ES_INFO = 0,
	ES_WARNING,
	ES_ERROR,
	ES_FATAL
};

typedef struct _error_package
{
	uint8		code;
	uint8		severity;
	uint32		rawErr;
	uint32		line;
	const char* file;
	const char* message;
	void* pKeyring;
	void* optional;
}errpack;

typedef enum _type
{
	T_CHAR = 0x1,	T_UCHAR,
	T_SHORT,		T_USHORT,
	T_INT,			T_UINT,
	T_FLOAT,		T_LONG,
	T_ULONG,		T_LONGLONG,
	T_ULONGLONG,	T_DOUBLE,
	T_LONGDOUBLE,
	
	T_PCHAR,		T_PUCHAR,
	T_PSHORT,		T_PUSHORT,
	T_PINT,			T_PUINT,
	T_PLONG,		T_PULONG,
	T_PLONGLONG,	T_PULONGLONG,
	T_PFLOAT,		T_PDOUBLE,
	T_PLONGDOUBLE,	T_PVOID,

	T_PPCHAR,		T_PPUCHAR,
	T_PPSHORT,		T_PPUSHORT,
	T_PPINT,		T_PPUINT,
	T_PPLONG,		T_PPULONG,
	T_PPLONGLONG,	T_PPULONGLONG,
	T_PPFLOAT,		T_PPDOUBLE,
	T_PPLONGDOUBLE, T_PPVOID,

	T_RESERVED = 0XFE,
	T_UNKNOWN = 0xFF
}data_type;

typedef enum _request_action
{
	REQ_MAKE = 0x1,
	REQ_DESTROY,
	REQ_STORE,
	REQ_RESERVE,
	REQ_FILL,
	REQ_GET,
	REQ_REMOVE,
	REQ_CONDENSE,

	REQ_PRINTKEYS
}action;

typedef struct _key
{
	char		name[MAX_NAME_LENGTH];
	data_type	type;
	void*		data;
}key;
#define KEY_SIZE sizeof(key)

typedef struct _keyring
{
	void*			pActiveStash;
	uint64			numStashes;
	uint64			usedKeySlots;
	key*			keys;
}*keyring_handle;

typedef struct _stash
{
	void*		end;
	void*		next;
	uint8		isFull;
}stash, *stash_handle;

extern void (*pCallbackFunc)(errpack errorPackage);
void __InitErrorCallback(void (*pErrFunc)(errpack errorPackage), uint32 line, const char* file);
void __ErrorCatch(ret_code code, uint32 line, const char* file, void* optional);
void __PrintKeys(uint32 line, const char* file);
#define errcatch(EC, optional) __ErrorCatch(EC, __LINE__, __FILE__, optional)

ret_code __SetMemContained(void);
ret_code __Stash__Action(uint64 sizeMB, action reqAction);
ret_code __MemRequest(char* name, data_type type, void* container, uint64 sizeItem, uint64 numItems, action reqAction);


#define _get_type(container) _Generic((container),						\
		char: T_CHAR,				unsigned char: T_UCHAR,				\
		short: T_SHORT,				unsigned short: T_USHORT,			\
		int: T_INT,					unsigned int: T_UINT,				\
		long: T_LONG,				unsigned long: T_ULONG,				\
		long long: T_LONGLONG,		unsigned long long: T_ULONGLONG,	\
		float: T_FLOAT,				double: T_DOUBLE,					\
		long double: T_LONGDOUBLE,										\
																		\
		char*: T_PCHAR,				unsigned char*: T_PUCHAR,			\
		short*: T_PSHORT,			unsigned short*: T_PUSHORT,			\
		int*: T_PINT,				unsigned int*: T_PUINT,				\
		long*: T_PLONG,				unsigned long*: T_PULONG,			\
		long long*: T_PLONGLONG,	unsigned long long*:T_PULONGLONG,	\
		float*: T_PFLOAT,			double*: T_PDOUBLE,					\
		long double*:T_PLONGDOUBLE,										\
																		\
		char**: T_PPCHAR,			unsigned char**: T_PPUCHAR,			\
		short**: T_PPSHORT,			unsigned short**: T_PPUSHORT,		\
		int**: T_PPINT,				unsigned int**: T_PPUINT,			\
		long**: T_PPLONG,			unsigned long**: T_PPULONG,			\
		long long**: T_PPLONGLONG,	unsigned long long**:T_PPULONGLONG,	\
		float**: T_PPFLOAT,			double**: T_PPDOUBLE,				\
		long double**:T_PPLONGDOUBLE,									\
																		\
		void*: T_PVOID,				void**: T_PPVOID,					\
		default: T_UNKNOWN												)
#define _format_request(name, container, reserveSize, reqAction) __MemRequest(name, _get_type(container), &container, sizeof(container), reserveSize, reqAction)


#define InitErrorCallback(pFunc)					__InitErrorCallback(pFunc, __LINE__, __FILE__)


#define SetMemAllContained()						errcatch(__SetMemContained(), "MEM_SWITCH")


#define MakeStash(sizeMB)							errcatch(__Stash__Action(sizeMB, REQ_MAKE), NULL)


#define DestroyStash()								errcatch(__Stash__Action(0, REQ_DESTROY), NULL)


#define Store(name, input)							errcatch(_format_request(name, input, 0, REQ_STORE), name)


#define Reserve(name, reserveSize, pReserved)		errcatch(_format_request(name, pReserved, reserveSize, REQ_RESERVE), name)


#define Fill(name, pInput)							errcatch(_format_request(name, pInput, 0, REQ_FILL), name)


#define Get(name, output)							errcatch(_format_request(name, output, 0, REQ_GET), name)


#define Remove(name)								errcatch(__MemRequest(name, 0, NULL, 0, 0, REQ_REMOVE), name)


#define Condense()									errcatch(__MemRequest(NULL, 0, NULL, 0, 0, REQ_CONDENSE))


#define PrintKeys()									__PrintKeys(__LINE__, __FILE__)


uint32 NumKeys();





ret_code INDEX_SEED(char* pName, uint64* pIndex);
ret_code INDEX_REDUCE(char* pName, uint64 indexSeed, uint64* pIndexSeed, action reqType);
ret_code INDEX_REBUILD(char* pName, uint64 indexSeed, uint64* pNewIndex, uint8 iteration, action reqType);
extern uint32 unresolvedCollisions;
#endif