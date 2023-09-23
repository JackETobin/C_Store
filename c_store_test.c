#include "c_store.h"
#include <stdio.h>
#include <Windows.h>

char* randomWords[] = 
{
"a",
"abilit",
"able",
"about",
"above",
"accept",
"accordng",
"accoun",
"across",
"act",
"action",
"activiy",
"actualy",
"add",
"addres",
"adminitraion",
"admit",
"adult",
"affect",
"after",
"again",
"agains",
"age",
"agency",
"agent",
"ago",
"agree",
"agreemnt",
"ahead",
"air",
"all",
"allow",
"almost",
"alone",
"along",
"alread",
"also",
"althouh",
"always",
"Americn",
"among",
"amount",
"analyss",
"and",
"animal",
"anothe",
"answer",
"any",
"anyone",
"anythig",
"appear",
"apply",
"approah",
"area",
"argue",
"arm",
"around",
"arrive",
"art",
"articl",
"artist",
"as	",
"ask",
"assume",
"at	",
"attack",
"attenton",
"attorny",
"audiene",
"author",
"authorty",
"availale",
"avoid",
"away",
"baby",
"back",
"bad",
"bag",
"ball",
"bank",
"bar",
"base",
"be	",
"beat",
"beautiul",
"becaus",
"become",
"bed",
"before",
"begin",
"behavir",
"behind",
"believ",
"benefi",
"best",
"better",
"betwee",
"beyond",
"big",
"bill",
"billio",
"bit",
"black",
"blood",
"blue",
"board",
"body",
"book",
"born",
"both",
"box",
"boy",
"break",
"bring",
"brothe",
"budget",
"build",
"buildig",
"busines",
"but",
"buy",
"by	",
"call",
"camera",
"campain",
"can",
"cancer",
"candidte",
"capita",
"car",
"card",
"care",
"career",
"carry",
"case",
"catch",
"cause",
"cell",
"center",
"centra",
"centur",
"certai",
"certaily",
"chair",
"challege",
"chance",
"change",
"characer",
"charge",
"check",
"child",
"choice",
"choose",
"church",
"citize",
"city",
"civil",
"claim",
"class",
"clear",
"clearl",
"close",
"coach",
"cold",
"collecion",
"colleg",
"color",
"come",
"commerial",
"common",
"communty",
"compan",
"compar",
"computr",
"concer",
"conditon",
"confernce",
"Congres",
"considr",
"consumr",
"contai",
"contine",
"contro",
"cost",
"could",
"countr",
"couple",
"course",
"court",
"cover",
"create",
"crime",
"culturl",
"cultur",
"cup",
"curren",
"customr",
"cut",
"dark",
"data",
"daughtr",
"day",
"dead",
"deal",
"death",
"debate",
"decade",
"decide",
"decisin",
"deep",
"defens",
"degree",
"Democrt",
"democrtic",
"descrie",
"design",
"despit",
"detail",
"determne",
"develo",
"develomen",
"die",
"differnce",
"differnt",
"difficlt",
"dinner",
"directon",
"directr",
"discovr",
"discus",
"discusion",
"diseas",
"do	",
"doctor",
"dog",
"door",
"down",
"draw",
"dream",
"drive",
"drop",
"drug",
"during",
"each",
"early",
"east",
"easy",
"eat",
"economc",
"econom",
"edge",
"educaton",
"effect",
"effort",
"eight",
"either",
"electin",
"else",
"employe",
"end",
"energy",
"enjoy",
"enough",
"enter",
"entire",
"enviromen",
"enviromenal",
"especilly",
"establsh",
"even",
"evenin",
"event",
"ever",
"every",
"everybdy",
"everyoe",
"everyting",
"evidene",
"exactl",
"exampl",
"executve",
"exist",
"expect",
"experince",
"expert",
"explai",
"eye",
"face",
"fact",
"factor",
"fail",
"fall",
"family",
"far",
"fast",
"father",
"fear",
"federa",
"feel",
"feelin",
"few",
"field",
"fight",
"figure",
"fill",
"film",
"final",
"finall",
"financal",
"find",
"fine",
"finger",
"finish",
"fire",
"firm",
"first",
"fish",
"five",
"floor",
"fly",
"focus",
"follow",
"food",
"foot",
"for",
"force",
"foreig",
"forget",
"form",
"former",
"forwar",
"four",
"free",
"friend",
"from",
"front",
"full",
"fund",
"future",
"game",
"garden",
"gas",
"genera",
"generaion",
"get",
"girl",
"give",
"glass",
"go	",
"goal",
"good",
"governent",
"great",
"green",
"ground",
"group",
"grow",
"growth",
"guess",
"gun",
"guy",
"hair",
"half",
"hand",
"hang",
"happen",
"happy",
"hard",
"have",
"he	",
"head",
"health",
"hear",
"heart",
"heat",
"heavy",
"help",
"her",
"here",
"hersel",
"high",
"him",
"himsel",
"his",
"histor",
"hit",
"hold",
"home",
"hope",
"hospitl",
"hot",
"hotel",
"hour",
"house",
"how",
"howeve",
"huge",
"human",
"hundre",
"husban",
"I	",
"idea",
"identiy",
"if	",
"image",
"imagin",
"impact",
"importnt",
"improv",
"in	",
"includ",
"includng",
"increae",
"indeed",
"indicae",
"indiviual",
"industy",
"informtio",
"inside",
"instea",
"instittio",
"interet",
"interetin",
"interntioal",
"intervew",
"into",
"investent",
"involv",
"issue",
"it	",
"item",
"its",
"itself",
"job",
"join",
"just",
"keep",
"key",
"kid",
"kill",
"kind",
"kitche",
"know",
"knowlege",
"land",
"languae",
"large",
"last",
"late",
"later",
"laugh",
"law",
"lawyer",
"lay",
"lead",
"leader",
"learn",
"least",
"leave",
"left",
"leg",
"legal",
"less",
"let",
"letter",
"level",
"lie",
"life",
"light",
"like",
"likely",
"line",
"list",
"listen",
"little",
"live",
"local",
"long",
"look",
"lose",
"loss",
"lot",
"love",
"low",
"machin",
"magazie",
"main",
"maintan",
"major",
"majoriy",
"make",
"man",
"manage",
"management",
"manager",
"many",
"market",
"marriae",
"materil",
"matter",
"may",
"maybe",
"me	",
"mean",
"measur",
"media",
"medica",
"meet",
"meetin",
"member",
"memory",
"mentio",
"messag",
"method",
"middle",
"might",
"militay",
"millio",
"mind",
"minute",
"miss",
"missio",
"model",
"modern",
"moment",
"money",
"month",
"more",
"mornin",
"most",
"mother",
"mouth",
"move",
"movemet",
"movie",
"Mr	",
"Mrs",
"much",
"music",
"must",
"my	",
"myself",
"name",
"nation",
"nationl",
"natura",
"nature",
"near",
"nearly",
"necessry",
"need",
"networ",
"never",
"new",
"news",
"newspaer",
"next",
"nice",
"night",
"no	",
"none",
"nor",
"north",
"not",
"note",
"nothin",
"notice",
"now",
"n't",
"number",
"occur",
"of	",
"off",
"offer",
"office",
"officer",
"official",
"often",
"oh	",
"oil",
"ok	",
"old",
"on	",
"once",
"one",
"only",
"onto",
"open",
"operaton",
"opportnit",
"option",
"or	",
"order",
"organiatin",
"other",
"others",
"our",
"out",
"outsid",
"over",
"own",
"owner",
"page",
"pain",
"paintig",
"paper",
"parent",
"part",
"particpan",
"particlar",
"particlary",
"partne",
"party",
"pass",
"past",
"patien",
"patter",
"pay",
"peace",
"people",
"per",
"perfor",
"perforanc",
"perhap",
"period",
"person",
"personl",
"phone",
"physical",
"pick",
"picture",
"piece",
"place",
"plan",
"plant",
"play",
"player",
"PM	",
"point",
"police",
"policy",
"political",
"politics",
"poor",
"popular",
"population",
"position",
"positive",
"possible",
"power",
"practice",
"prepare",
"present",
"president",
"pressure",
"pretty",
"prevent",
"price",
"private",
"probably",
"problem",
"process",
"produce",
"product",
"production",
"professional",
"professor",
"program",
"project",
"property",
"protec",
"prove",
"provide",
"public",
"pull",
"purpos",
"push",
"put",
"quality",
"question",
"quickly",
"quite",
"race",
"radio",
"raise",
"range",
"rate",
"rather",
"reach",
"read",
"ready",
"real",
"reality",
"realize",
"really",
"reason",
"receive",
"recent",
"recently",
"recognze",
"record",
"red",
"reduce",
"reflect",
"region",
"relate",
"relationship",
"religious",
"remain",
"remember",
"remove",
"report",
"represent",
"Republican",
"require",
"research",
"resource",
"respond",
"response",
"responsibiity",
"rest",
"result",
"return",
"reveal",
"rich",
"right",
"rise",
"risk",
"road",
"rock",
"role",
"room",
"rule",
"run",
"safe",
"same",
"save",
"say",
"scene",
"school",
"science",
"scientist",
"score",
"sea",
"season",
"seat",
"second",
"section",
"security",
"see",
"seek",
"seem",
"sell",
"send",
"senior",
"sense",
"series",
"serious",
"serve",
"service",
"set",
"seven",
"several",
"sex",
"sexual",
"shake",
"share",
"she",
"shoot",
"short",
"shot",
"should",
"shoulder",
"show",
"side",
"sign",
"significant",
"simila",
"simple",
"simply",
"since",
"sing",
"single",
"sister",
"sit",
"site",
"situation",
"six",
"size",
"skill",
"skin",
"small",
"smile",
"so",
"social",
"society",
"soldier",
"some",
"somebody",
"someone",
"something",
"sometimes",
"son",
"song",
"soon",
"sort",
"sound",
"source",
"south",
"southern",
"space",
"speak",
"special",
"specific",
"speech",
"spend",
"sport",
"spring",
"staff",
"stage",
"stand",
"standard",
"star",
"start",
"state",
"statement",
"statio",
"stay",
"step",
"still",
"stock",
"stop",
"store",
"story",
"strategy",
"street",
"strong",
"structure",
"studen",
"study",
"stuff",
"style",
"subjec",
"succes",
"successful",
"such",
"suddenly",
"suffer",
"sugges",
"summer",
"suppor",
"sure",
"surface",
"system",
"table",
"take",
"talk",
"task",
"tax",
"teach",
"teache",
"team",
"technoogy",
"televiion",
"tell",
"ten",
"tend",
"term",
"test",
"than",
"thank",
"that",
"the",
"their",
"them",
"themseves",
"then",
"theory",
"there",
"these",
"they",
"thing",
"think",
"third",
"this",
"those",
"though",
"thought",
"thousnad",
"threat",
"three",
"through",
"throughout",
"throw",
"thus",
"time",
"to	",
"today",
"together",
"tonight",
"too",
"top",
"total",
"tough",
"toward",
"town",
"trade",
"traditonal",
"training",
"travel",
"treat",
"treatment",
"tree",
"trial",
"trip",
"trouble",
"true",
"truth",
"try",
"turn",
"TV	",
"two",
"type",
"under",
"understand",
"unit",
"until",
"up	",
"upon",
"us	",
"use",
"usually",
"value",
"various",
"very",
"victim",
"view",
"violence",
"visit",
"voice",
"vote",
"wait",
"walk",
"wall",
"want",
"war",
"watch",
"water",
"way",
"we	",
"weapon",
"wear",
"week",
"weight",
"well",
"west",
"western",
"what",
"whatever",
"when",
"where",
"whether",
"which",
"while",
"white",
"who",
"whole",
"whom",
"whose",
"why",
"wide",
"wife",
"will",
"win",
"wind",
"window",
"wish",
"with",
"within",
"without",
"woman",
"wonder",
"word",
"work",
"worker",
"world",
"worry",
"would",
"write",
"writer",
"wrong",
"yard",
"yeah",
"year",
"yes",
"yet",
"you",
"young",
"your",
"yourself"
};

int NameStart(char* path)
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

void OperationCheck(char* path)
{
	char name[48];
	int nameStart = NameStart(path);

	for (int i = 0; *(path + nameStart) != '.'; i++)
		name[i] = *(path + nameStart++);

	printf("%s operational...\n\n", name);
}


int main(int numPassed, char* passed[])
{
	OperationCheck(passed[0]);
	InitErrorCallback(NULL);
	//TODO: SetMemAllContained();

	/*
	uint64 index = 0;
	uint64 indexSeed = 0;
	char* name = "some odd name here";
	INDEX_SEED(name, &indexSeed);
	INDEX_REDUCE(name, indexSeed, &index);
	INDEX_REBUILD(name, indexSeed, &index, 0);
	*/
	
	MakeStash(3);

	
	uint32 typeInt = 14;
	uint32 intContainer = 0;
	uint32* arrayPtr;
	uint32 intArray[] = { 45, 46, 47, 48 };

	//Fill("somename", intArray);

	Reserve("somename", 53, intArray);
	Remove("somename");
	Fill("somename", intArray);
	//Fill("somename", typeInt);
	//Get("somename", arrayPtr);

	//char* aString;
	//Store("aString", "Some string to store.");
	//Get("aString", aString);

	//uint32* arrayPtrPtr;
	//Store("arrayPtr", arrayPtr);
	//Get("arrayPtr", arrayPtrPtr);
	//Fill("aString", intArray);
	

	uint32 len = 1000;
	uint8 data = 1;
	
	LARGE_INTEGER frequency;
	LARGE_INTEGER t1, t2;
	double elapsedTimeS;
	double elapsedTimeG;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&t1);
	
	for (uint32 i = 0; i < len; i++)
		Store(randomWords[i], data);

	QueryPerformanceCounter(&t2);
	elapsedTimeS = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	printf("\nStore time:\t\t\t%f ms.\n", elapsedTimeS);

	uint8 buff = 0;
	uint32 tally = 0;
	
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&t1);
	
	for (uint32 i = 0; i < len; i++)
	{
		Get(randomWords[i], buff);
		tally += buff;
	}
	
	QueryPerformanceCounter(&t2);
	elapsedTimeG = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	printf("Get time:\t\t\t%f ms.\n", elapsedTimeG);
	printf("Objects stored and retrieved:\t%i\n", tally);
	printf("Total time elapsed:\t\t%f ms.\n\n", elapsedTimeG + elapsedTimeS);
	printf("Unresolved collisions: %i\n\n", unresolvedCollisions);

	//PrintKeys();
	return 0;
}