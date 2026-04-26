#define PIKA_OMIT_PREFIX

#include <stdbool.h>
#include <stdio.h>
#include "pika.h"

static const schema_t schema = {
	BOOL(false, .Name = "verbose", .Desc = "Enable verbose output", .Alias = ALIASES("v")),
	BOOL(false, .Name = "dryRun",  .Desc = "Simulate without making changes", .Alias = ALIASES("d", "dry")),
	CSTR("localhost", .Name = "host",     .Desc = "Host to connect to", .Alias = ALIASES("H")),
	U64(8080,      .Name = "port",     .Desc = "Port to connect on", .Alias = ALIASES("p")),
	CSTR("root",      .Name = "username", .Desc = "Username for auth",  .Alias = ALIASES("u", "n")),
	HELP,
};

int main(int argc, char** argv) {
	res_arena_t arena = {0};
	size_t schemaSize = PIKA_SCHEMA_SIZE(schema);

	LoadSchema(argc, argv, schema, schemaSize, &arena);

	bool verbose   = GetBool("verbose",  &arena);
	bool  dryRun   = GetBool("dryRun",   &arena);
	char* host     = GetCStr("host",     &arena);
	uint64_t port  = GetU64("port",      &arena);
	char* username = GetCStr("username", &arena);

	PIKA_INF("verbose:  %d", verbose);
	PIKA_INF("dryRun:   %d", dryRun);
	PIKA_INF("host:     %s", host);
	PIKA_INF("port:     %lu", port);
	PIKA_INF("username: %s", username);
}
