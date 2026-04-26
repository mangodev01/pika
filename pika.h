#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// DONT unprefix this macro - VERSION is VERY ambiguous
#define PIKA_VERSION "0.0.1"

#define PIKA_RESET        "\033[0m"
#define PIKA_BLACK        "\033[30m"
#define PIKA_RED          "\033[31m"
#define PIKA_GREEN        "\033[32m"
#define PIKA_YELLOW       "\033[33m"
#define PIKA_BLUE         "\033[34m"
#define PIKA_MAGENTA      "\033[35m"
#define PIKA_CYAN         "\033[36m"
#define PIKA_WHITE        "\033[37m"

#define PIKA_BRIGHT_BLACK   "\033[90m"
#define PIKA_BRIGHT_RED     "\033[91m"
#define PIKA_BRIGHT_GREEN   "\033[92m"
#define PIKA_BRIGHT_YELLOW  "\033[93m"
#define PIKA_BRIGHT_BLUE    "\033[94m"
#define PIKA_BRIGHT_MAGENTA "\033[95m"
#define PIKA_BRIGHT_CYAN    "\033[96m"
#define PIKA_BRIGHT_WHITE   "\033[97m"

#define PIKA_BOLD        "\033[1m"
#define PIKA_DIM         "\033[2m"
#define PIKA_ITALIC      "\033[3m"
#define PIKA_UNDERLINE   "\033[4m"

typedef enum {
	PikaType_Bool = 0,
	PikaType_Help = 1,
	PikaType_CStr = 2,
	PikaType_U32 = 3,
	PikaType_I32 = 4,
	PikaType_U64 = 5,
	PikaType_I64 = 6,
} pika_type_t;

static inline const char* const PikaTypeToString(pika_type_t ty) {
	switch (ty) {
		case PikaType_Bool: return "boolean";
		case PikaType_Help: return "boolean";
		case PikaType_CStr: return "string";
		case PikaType_U32:  return "integer (u32)";
		case PikaType_I32:  return "integer (i32)";
		case PikaType_U64:  return "integer (u64)";
		case PikaType_I64:  return "integer (i64)";
		default: return "unknown";
	}
}


typedef struct {
	const char* const* Aliases;
	size_t Count;
} pika_aliases_t;

typedef struct {
	const pika_type_t Type;
	const char* Name;
	const char* Desc;
	pika_aliases_t Alias;
} pika_arg_descriptor_t;

typedef struct {
	const void* Default;
	const pika_arg_descriptor_t Descriptor;
} pika_arg_t;

typedef struct pika_res_t {
	const char* Key;
	void* Val;
	pika_type_t Type;
} pika_res_t;

typedef struct {
    pika_res_t nodes[64];
    size_t count;
} pika_res_arena_t;

typedef const pika_arg_t pika_schema_t[];

#define PIKA_BOOL(defaultValue, ...) \
    _PikaArg((const void*)(intptr_t)(defaultValue), ((pika_arg_descriptor_t){ \
        .Type = PikaType_Bool, \
        __VA_ARGS__ \
    }))

#define PIKA_CSTR(defaultValue, ...) \
    _PikaArg((const void*)(defaultValue), ((pika_arg_descriptor_t){ \
        .Type = PikaType_CStr, \
        __VA_ARGS__ \
    }))

#define PIKA_I32(defaultValue, ...) \
    _PikaArg((const void*)(defaultValue), ((pika_arg_descriptor_t){ \
        .Type = PikaType_I32, \
        __VA_ARGS__ \
    }))


#define PIKA_U32(defaultValue, ...) \
    _PikaArg((const void*)(defaultValue), ((pika_arg_descriptor_t){ \
        .Type = PikaType_U32, \
        __VA_ARGS__ \
    }))

#define PIKA_I64(defaultValue, ...) \
    _PikaArg((const void*)(defaultValue), ((pika_arg_descriptor_t){ \
        .Type = PikaType_I64, \
        __VA_ARGS__ \
    }))


#define PIKA_U64(defaultValue, ...) \
    _PikaArg((const void*)(defaultValue), ((pika_arg_descriptor_t){ \
        .Type = PikaType_U64, \
        __VA_ARGS__ \
    }))

#define PIKA_HELP \
    _PikaArg(false, ((pika_arg_descriptor_t){ \
        .Type = PikaType_Help, \
		.Name = "help", \
		.Desc = "Show this help message and exit.", \
		.Alias = PIKA_ALIASES("h"), \
    }))

#define PIKA_HELP_CUSTOM(defaultValue, ...) \
    _PikaArg((const void*)(defaultValue), ((pika_arg_descriptor_t){ \
        .Type = PikaType_Help, \
		__VA_ARGS__ \
    }))


#define PIKA_ALIAS_ARRAY(...) ((const char*[]){ __VA_ARGS__ })

#define PIKA_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#define PIKA_ALIASES(...) (pika_aliases_t){                 \
	.Aliases = PIKA_ALIAS_ARRAY(__VA_ARGS__),               \
	.Count = PIKA_COUNT(PIKA_ALIAS_ARRAY(__VA_ARGS__))      \
}

#define _PikaArg(defaultValue, descriptor) ((pika_arg_t){ .Default = (defaultValue), .Descriptor = descriptor })

#define PIKA_ERR(fmt, ...) \
    fprintf(stderr, PIKA_BRIGHT_RED fmt PIKA_RESET "\n", ##__VA_ARGS__)

#define PIKA_INF(fmt, ...) \
    printf(PIKA_BRIGHT_CYAN fmt PIKA_RESET "\n", ##__VA_ARGS__)

static inline const pika_arg_descriptor_t* PikaSchemaHasAlias(pika_schema_t schema, size_t schemaSize, const char* userAlias) {
	for (size_t i = 0; i < schemaSize; i++) {
		pika_aliases_t aliases = schema[i].Descriptor.Alias;
		for (size_t a = 0; a < aliases.Count; a++) {
			const char* alias = aliases.Aliases[a];

			if (strcmp(alias, userAlias) == 0)  {
				return &schema[i].Descriptor;
			}
		}
	}

	return 0;
}

static inline const pika_arg_descriptor_t* PikaSchemaHasName(pika_schema_t schema, size_t schemaSize, const char* userName) {
	for (size_t i = 0; i < schemaSize; i++) {
		if (strcmp(schema[i].Descriptor.Name, userName) == 0)  {
			return &schema[i].Descriptor;
		}
	}

	return 0;
}

static inline size_t _PikaMax(size_t a, size_t b) {
	return a > b ? a : b;
}

static inline void PikaLogSchema(pika_schema_t schema, size_t schemaSize) {

#define PIKA_BOX_MAX 256

    for (size_t i = 0; i < schemaSize; i++) {

        const pika_arg_descriptor_t* d = &schema[i].Descriptor;

        const size_t w1 = strlen("Aliases"); // 7, widest label

        char typeBuf[32];
        snprintf(typeBuf, sizeof(typeBuf), "%s", PikaTypeToString(d->Type));

        char aliasBuf[PIKA_BOX_MAX];
        aliasBuf[0] = '\0';
        for (size_t a = 0; a < d->Alias.Count; a++) {
            if (a > 0) strncat(aliasBuf, ", ", sizeof(aliasBuf) - strlen(aliasBuf) - 1);
            strncat(aliasBuf, d->Alias.Aliases[a], sizeof(aliasBuf) - strlen(aliasBuf) - 1);
        }

        size_t w2 = 0;
        w2 = _PikaMax(w2, strlen(d->Desc));
        w2 = _PikaMax(w2, strlen(d->Name));
        w2 = _PikaMax(w2, strlen(typeBuf));
        w2 = _PikaMax(w2, strlen(aliasBuf));

        // inner width: " " + w1 + " │ " + w2 + " "
        //               1  + w1 +  3  + w2 + 1  = w1 + w2 + 5
        // but │ is 3 bytes displaying as 1, so actual char width = w1 + w2 + 3
        size_t innerWidth = w1 + w2 + 5;

        // top bar: (w1+2) dashes + ┬ + (w2+2) dashes
        char topBar[PIKA_BOX_MAX * 3 + 1];
        size_t topLen = 0;
        for (size_t b = 0; b < w1 + 2 && topLen + 3 < sizeof(topBar); b++) { memcpy(topBar + topLen, "─", 3); topLen += 3; }
        memcpy(topBar + topLen, "┬", 3); topLen += 3;
        for (size_t b = 0; b < w2 + 2 && topLen + 3 < sizeof(topBar); b++) { memcpy(topBar + topLen, "─", 3); topLen += 3; }
        topBar[topLen] = '\0';

        // bot bar: (w1+2) dashes + ┴ + (w2+2) dashes
        char botBar[PIKA_BOX_MAX * 3 + 1];
        size_t botLen = 0;
        for (size_t b = 0; b < w1 + 2 && botLen + 3 < sizeof(botBar); b++) { memcpy(botBar + botLen, "─", 3); botLen += 3; }
        memcpy(botBar + botLen, "┴", 3); botLen += 3;
        for (size_t b = 0; b < w2 + 2 && botLen + 3 < sizeof(botBar); b++) { memcpy(botBar + botLen, "─", 3); botLen += 3; }
        botBar[botLen] = '\0';

        // _written includes 2 extra bytes for the │ separator (3 bytes, 1 display width)
#define PIKA_BOX_LINE(fmt, ...) do { \
    char _line[PIKA_BOX_MAX]; \
    int _written = snprintf(_line, sizeof(_line), fmt, ##__VA_ARGS__); \
    if (_written < 0) _written = 0; \
    int _pad = (int)innerWidth - (_written - 2); \
    if (_pad < 0) _pad = 0; \
    PIKA_INF("│%s%*s│", _line, _pad, ""); \
} while(0)

        PIKA_INF("╭%s╮", topBar);
        PIKA_BOX_LINE(" %-*s │ %s", (int)w1, "Desc",    d->Desc);
        PIKA_BOX_LINE(" %-*s │ %s", (int)w1, "Name",    d->Name);
        PIKA_BOX_LINE(" %-*s │ %s", (int)w1, "Type",    typeBuf);
        PIKA_BOX_LINE(" %-*s │ %s", (int)w1, "Aliases", aliasBuf);
        PIKA_INF("╰%s╯", botBar);

#undef PIKA_BOX_LINE
    }

#undef PIKA_BOX_MAX
}

#define PIKA_ALIAS_ERR(alias)                      \
	PIKA_ERR("╭─────────────────────╮");           \
	PIKA_ERR("│ invalid alias: '%s'  │", alias);   \
	PIKA_ERR("╰─────────────────────╯");

#define PIKA_OPT_ERR(opt) do {                                     \
    const char* _opt = (opt);                                      \
    size_t _len = strlen(_opt);                                    \
    size_t _inner = 20 + _len;                                     \
                                                                   \
    char _top[512] = {0};                                          \
    char _bot[512] = {0};                                          \
                                                                   \
    size_t pos = 0;                                                \
                                                                   \
    pos += snprintf(_top + pos, sizeof(_top) - pos, "╭");          \
    for (size_t i = 0; i < _inner; i++)                            \
        pos += snprintf(_top + pos, sizeof(_top) - pos, "─");      \
    snprintf(_top + pos, sizeof(_top) - pos, "╮");                 \
                                                                   \
    pos = 0;                                                       \
                                                                   \
    pos += snprintf(_bot + pos, sizeof(_bot) - pos, "╰");          \
    for (size_t i = 0; i < _inner; i++)                            \
        pos += snprintf(_bot + pos, sizeof(_bot) - pos, "─");      \
    snprintf(_bot + pos, sizeof(_bot) - pos, "╯");                 \
                                                                   \
    PIKA_ERR("%s", _top);                                          \
    PIKA_ERR("│ invalid option: '%s' │", _opt);                    \
    PIKA_ERR("%s", _bot);                                          \
} while (0)

#define PIKA_NOT_VALID(val, type) do {                                          \
    const char* _val = (val);                                                   \
    const char* _type = (type);                                                 \
    size_t _len = strlen("value '") + strlen(_val) + strlen("' isn't of type ") + strlen(_type); \
    size_t _inner = 2 + _len;                                                   \
                                                                                \
    char _top[512] = {0};                                                       \
    char _bot[512] = {0};                                                       \
                                                                                \
    size_t pos = 0;                                                             \
                                                                                \
    pos += snprintf(_top + pos, sizeof(_top) - pos, "╭");                       \
    for (size_t i = 0; i < _inner; i++)                                         \
        pos += snprintf(_top + pos, sizeof(_top) - pos, "─");                   \
    snprintf(_top + pos, sizeof(_top) - pos, "╮");                              \
                                                                                \
    pos = 0;                                                                    \
                                                                                \
    pos += snprintf(_bot + pos, sizeof(_bot) - pos, "╰");                       \
    for (size_t i = 0; i < _inner; i++)                                         \
        pos += snprintf(_bot + pos, sizeof(_bot) - pos, "─");                   \
    snprintf(_bot + pos, sizeof(_bot) - pos, "╯");                              \
                                                                                \
    PIKA_ERR("%s", _top);                                                       \
    PIKA_ERR("│ value '%s' isn't of type %s │", _val, _type);                   \
    PIKA_ERR("%s", _bot);                                                       \
} while (0)

#define PIKA_EMPTY()                               \
	PIKA_ERR("╭─────────────────────╮");           \
	PIKA_ERR("│   option is empty   │");           \
	PIKA_ERR("╰─────────────────────╯");

#define PIKA_ALIAS_UNEXPECTED()                    \
	PIKA_ERR("╭────────────────────────────╮");    \
	PIKA_ERR("│ expected value, got option │");    \
	PIKA_ERR("╰────────────────────────────╯");

typedef enum {
	PikaArgType_Option = 0,
	PikaArgType_Value = 1,
	// PikaArgType_Subcommand = 2,
} pika_arg_type_t;

static inline void _PikaPush(pika_res_arena_t* arena,
                            const char* key,
                            void* val,
                            const pika_type_t type)
{
    arena->nodes[arena->count++] = (pika_res_t){ .Key = key, .Val = val, .Type = type };
}

static inline void _PikaHandleHelp(const pika_arg_descriptor_t* curArg, pika_schema_t schema, size_t schemaSize) {
	if (curArg->Type == PikaType_Help) {
		PikaLogSchema(schema, schemaSize);
		exit(0);
	}
}


static inline void _PikaParseOpt(
	size_t i,
	char** argv,
	pika_schema_t schema,
	size_t schemaSize,
	pika_res_arena_t* pika,
	const pika_arg_descriptor_t* curArg,
	pika_arg_type_t* nextArgType
) {
	_PikaHandleHelp(curArg, schema, schemaSize);

	if (curArg->Type != PikaType_Bool)
		*nextArgType = PikaArgType_Value;
	else {
		_PikaPush(pika, curArg->Name, (void*)(intptr_t)true, PikaType_Bool);
	}
}

static inline uint32_t PikaParseU32(const char* val) {
    char* end;
    errno = 0;
    unsigned long result = strtoul(val, &end, 0);
    if (errno != 0 || end == val || *end != '\0' || result > UINT32_MAX) {
        PIKA_NOT_VALID(val, "u32");
        exit(-1);
    }
    return (uint32_t)result;
}

static inline int32_t PikaParseI32(const char* val) {
    char* end;
    errno = 0;
    long result = strtol(val, &end, 0);
    if (errno != 0 || end == val || *end != '\0' || result > INT32_MAX || result < INT32_MIN) {
        PIKA_NOT_VALID(val, "i32");
        exit(-1);
    }
    return (int32_t)result;
}

static inline uint64_t PikaParseU64(const char* val) {
    char* end;
    errno = 0;
    uint64_t result = strtoull(val, &end, 0);
    if (errno != 0 || end == val || *end != '\0') {
        PIKA_NOT_VALID(val, "u64");
        exit(-1);
    }
    return result;
}

static inline int64_t PikaParseI64(const char* val) {
    char* end;
    errno = 0;
    int64_t result = strtoll(val, &end, 0);
    if (errno != 0 || end == val || *end != '\0') {
        PIKA_NOT_VALID(val, "i64");
        exit(-1);
    }
    return result;
}



static inline void PikaParseVal(
	size_t i,
	char** argv,
	pika_schema_t schema,
	size_t schemaSize,
	pika_res_arena_t* pika,
	const pika_arg_descriptor_t* curArg,
	pika_arg_type_t* nextArgType
) {
	switch (curArg->Type) {
		case PikaType_Bool: {
			// should've already been handled by PikaParseOpt, ignoring
			return;
		}
		case PikaType_Help: {
			// should've ALSO already been handled by PikaParseOpt, ignoring
			return;
		}
		case PikaType_CStr: {
			_PikaPush(pika, curArg->Name, (void*)argv[i], curArg->Type);
			break;
		}
		case PikaType_U32:  {
			const char* ty = PikaTypeToString(curArg->Type);
			uint32_t val = PikaParseU32(argv[i]);
			_PikaPush(pika, curArg->Name, (void*)(uintptr_t)val, curArg->Type);
			break;
		}
		case PikaType_I32:  {
			const char* ty = PikaTypeToString(curArg->Type);
			int32_t val = PikaParseI32(argv[i]);
			_PikaPush(pika, curArg->Name, (void*)(intptr_t)val, curArg->Type);
			break;
		}
		case PikaType_U64:  {
			const char* ty = PikaTypeToString(curArg->Type);
			uint64_t val = PikaParseU64(argv[i]);
			_PikaPush(pika, curArg->Name, (void*)(uintptr_t)val, curArg->Type);
			break;
		}
		case PikaType_I64:  {
			const char* ty = PikaTypeToString(curArg->Type);
			int64_t val = PikaParseI64(argv[i]);
			_PikaPush(pika, curArg->Name, (void*)(intptr_t)val, curArg->Type);
			break;
		}
	}

	*nextArgType = PikaArgType_Option;
}

static inline const pika_arg_descriptor_t* _PikaParseAlias(
	size_t i,
	char** argv,
	pika_schema_t schema,
	size_t schemaSize,
	pika_res_arena_t* pika,
	const pika_arg_descriptor_t* curArg,
	pika_arg_type_t* nextArgType
) {
	if (*nextArgType != PikaArgType_Option) {
		PIKA_ALIAS_UNEXPECTED();
		return curArg;
	}

	const pika_arg_descriptor_t* res = curArg;

	if (strlen(argv[i]) > 1) {
		char* p = argv[i];
		// multi-alias: -abc → check 'a', 'b', 'c' individually
		while (*p) {
			char alias[2] = { *p++, '\0' };
			res = PikaSchemaHasAlias(schema, schemaSize, alias);
			if (!res) {
				PIKA_ALIAS_ERR(alias);
				exit(-1);
			}
			_PikaParseOpt(i, argv, schema, schemaSize, pika, res, nextArgType);
		}
	} else {
		if (!*argv[i]) {
			PIKA_EMPTY();
			exit(-1);
		} else {
			res = PikaSchemaHasAlias(schema, schemaSize, argv[i]);
			if (!res) {
				PIKA_ALIAS_ERR(argv[i]);
				exit(-1);
			}
			_PikaParseOpt(i, argv, schema, schemaSize, pika, res, nextArgType);
		}
	}

	return res;
}

static inline void* PikaGetArg(const char* name, pika_res_arena_t* arena) {
    for (size_t i = 0; i < arena->count; i++) {
        if (strcmp(arena->nodes[i].Key, name) == 0)
            return arena->nodes[i].Val;
    }
    return 0;
}

#define PikaGetBool(name, args) ((bool)(intptr_t)PikaGetArg(name, args))
#define PikaGetCStr(name, args) ((char*)PikaGetArg(name, args))
#define PikaGetU32(name, args)  ((uint32_t)(uintptr_t)PikaGetArg(name, args))
#define PikaGetI32(name, args)  ((int32_t)(intptr_t)PikaGetArg(name, args))
#define PikaGetU64(name, args)  ((uint64_t)(uintptr_t)PikaGetArg(name, args))
#define PikaGetI64(name, args)  ((int64_t)(intptr_t)PikaGetArg(name, args))

static inline void PikaLoadSchema(
	int userArgc,
	char** userArgv,
	pika_schema_t schema,
	size_t schemaSize,
	pika_res_arena_t* pika
) {
	pika_arg_type_t nextArgType = PikaArgType_Option;

	char** argv = userArgv + 1;
	int argc = userArgc - 1;

	const pika_arg_descriptor_t* curArg;

	for (size_t i = 0; i < argc; i++) {
		if (nextArgType == PikaArgType_Value) {
			PikaParseVal(i, argv, schema, schemaSize, pika, curArg, &nextArgType);
			continue;
		}

		if (argv[i][0] == '-' && argv[i][1] == '-') {
			argv[i] += 2;

			// PIKA_INF("Argument type: fullopt(%s)\n", argv[i]);

			if (!*argv[i]) {
				PIKA_EMPTY();
				exit(-1);
				return;
			}

			curArg = PikaSchemaHasName(schema, schemaSize, argv[i]);
			if (!curArg) {
				PIKA_OPT_ERR(argv[i]);
				exit(-1);
			}

			_PikaParseOpt(i, argv, schema, schemaSize, pika, curArg, &nextArgType);
		} else if (argv[i][0] == '-') {
			argv[i] += 1;


			// PIKA_INF("Argument type: aliasopt(%s)\n", argv[i]);

			curArg = _PikaParseAlias(i, argv, schema, schemaSize, pika, curArg, &nextArgType);
		}
#if 0
		// subcommands aren't yet supported in the schema
		else {
			argv[i] += 0;

			printf("Argument type: subcmd(%s)\n", argv[i]);
		}
#endif

		// printf("Arg %d: %s\n\n", i, argv[i]);
	}

	// walk SECOND TIME to supply default values
	for (size_t i = 0; i < schemaSize; i++) {
		pika_arg_t arg = schema[i];

		// if arg isn't specified by user, use default
		if (PikaGetArg(arg.Descriptor.Name, pika) == 0) {
			_PikaPush(pika, arg.Descriptor.Name, (void*)arg.Default, arg.Descriptor.Type);
		}
	}
}

static inline void PikaHello() {
	printf("Hello world from pika v0.0.1!\n");
}

#ifdef PIKA_OMIT_PREFIX
typedef pika_type_t type_t;
typedef pika_arg_t arg_t;
typedef pika_schema_t schema_t;
typedef pika_res_t res_t;
typedef pika_res_arena_t res_arena_t;
typedef pika_arg_descriptor_t arg_descriptor_t;
typedef pika_aliases_t aliases_t;
typedef pika_arg_type_t arg_type_t;

#define COUNT PIKA_COUNT

#define BOOL PIKA_BOOL
#define CSTR PIKA_CSTR
#define U32 PIKA_U32
#define I32 PIKA_I32

#define U64 PIKA_U64
#define I64 PIKA_I64
#define HELP PIKA_HELP
#define HELP_CUSTOM PIKA_HELP_CUSTOM

#define ALIASES PIKA_ALIASES
#define ALIAS_ARRAY PIKA_ALIAS_ARRAY
#define PIKA_SCHEMA_SIZE(schema) PIKA_COUNT(schema)

#define RESET PIKA_RESET
#define BLACK PIKA_BLACK
#define RED PIKA_RED
#define GREEN PIKA_GREEN
#define YELLOW PIKA_YELLOW
#define BLUE PIKA_BLUE
#define MAGENTA PIKA_MAGENTA
#define CYAN PIKA_CYAN
#define WHITE PIKA_WHITE

#define BRIGHT_BLACK PIKA_BRIGHT_BLACK
#define BRIGHT_RED PIKA_BRIGHT_RED
#define BRIGHT_GREEN PIKA_BRIGHT_GREEN
#define BRIGHT_YELLOW PIKA_BRIGHT_YELLOW
#define BRIGHT_BLUE PIKA_BRIGHT_BLUE
#define BRIGHT_MAGENTA PIKA_BRIGHT_MAGENTA
#define BRIGHT_CYAN PIKA_BRIGHT_CYAN
#define BRIGHT_WHITE PIKA_BRIGHT_WHITE

#define BOLD PIKA_BOLD
#define DIM PIKA_DIM
#define ITALIC PIKA_ITALIC
#define UNDERLINE PIKA_UNDERLINE

#define GetArg    PikaGetArg
#define GetBool   PikaGetBool
#define GetCStr   PikaGetCStr
#define GetU32    PikaGetU32
#define GetI32    PikaGetI32
#define GetU64    PikaGetU64
#define GetI64    PikaGetI64

#define LoadSchema  PikaLoadSchema
#define LogSchema   PikaLogSchema
#define Hello       PikaHello
#define Push        PikaPush
#define TypeToString PikaTypeToString
#define SchemaHasAlias PikaSchemaHasAlias
#define SchemaHasName  PikaSchemaHasName

#define SCHEMA_SIZE PIKA_SCHEMA_SIZE

#define ERR  PIKA_ERR
#define INF  PIKA_INF

#define ALIAS_ERR        PIKA_ALIAS_ERR
#define OPT_ERR          PIKA_OPT_ERR
#define NOT_VALID        PIKA_NOT_VALID
#define EMPTY            PIKA_EMPTY
#define ALIAS_UNEXPECTED PIKA_ALIAS_UNEXPECTED

#define GetBool PikaGetBool
#define GetCStr PikaGetCStr
#define GetU32  PikaGetU32
#define GetI32  PikaGetI32
#define GetU64  PikaGetU64
#define GetI64  PikaGetI64

#define _HandleHelp   _PikaHandleHelp
#define _ParseOpt     _PikaParseOpt
#define _ParseVal     _PikaParseVal
#define _ParseAlias   _PikaParseAlias
#define _Max          _PikaMax

#endif
