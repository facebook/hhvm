#include "manager.h"
#include "parse.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

void parse_test(char *spec)
{
	int i;
	mongo_con_manager *manager;
	mongo_servers *servers;
	char *error_message;

	manager = mongo_init();
	manager->log_function = mongo_log_printf;

	servers = mongo_parse_init();
	if (mongo_parse_server_spec(manager, servers, spec, (char **) &error_message)) {
		printf("error_message: %s\n", error_message);
		free(error_message);
	}

	for (i = 0; i < servers->count; i++) {
		char *tmp_hash;

		tmp_hash = mongo_server_create_hash(servers->server[i]);
		printf("HASH: %s\n", tmp_hash);
		free(tmp_hash);
	}
	mongo_servers_dump(manager, servers);

	mongo_servers_dtor(servers);
	mongo_deinit(manager);
}

int main(void)
{
	parse_test("host1:123");
	parse_test("host1:123,host2:123");
	parse_test("mongodb://host1:123,host2:123");
	parse_test("mongodb://derick:test@host1:123");
	parse_test("mongodb://derick:test@host1:123,host2:123");
	parse_test("mongodb://derick:test@host1:123,host2:123/database");
	parse_test("mongodb://derick:test@host1:123,host2/database");
	parse_test("mongodb://derick:test@host1,host2:123/database");
	parse_test("mongodb://host1,host2:123/database");
	/* Specifying options */
	parse_test("mongodb://derick:test@host1,host2:123/database?readPreference=secondary_preferred");
	parse_test("mongodb://derick:test@host1,host2:123/?readPreference=secondary");
	parse_test("mongodb://derick:test@host1,host2:123?timeout=4");
	parse_test("mongodb://derick:test@host1,host2:123/?timeout=4");
	parse_test("mongodb://derick:test@host1,host2:123/?readPreference");
	parse_test("mongodb://derick:test@host1,host2:123/?readPreference=primary;timeout=baz");
	parse_test("mongodb://derick:test@host1,host2:123/?readPreference=primary&timeout=baz");
	parse_test("mongodb://derick:test@host1,host2:123/?readPreference=primary;timeout");
	parse_test("mongodb://derick:test@host1,host2:123/?readPreference;timeout=baz");
	/* Specific options */
	parse_test("mongodb://host1/?replicaSet=testset");
	parse_test("mongodb://foo:bar@primary:14000/database?replicaSet=seta");
	parse_test("mongodb://foo:bar@primary:14000/database/?replicaSet=seta");
	/* Unix Domain Sockets */
	parse_test("mongodb:///tmp/mongodb-27017.sock");
	parse_test("mongodb:///tmp/mongodb-27017.sock/");
	parse_test("mongodb:///tmp/mongodb-27017.sock/?timeout=4");
	parse_test("mongodb:///tmp/mongodb-27017.sock/database");
	parse_test("mongodb:///tmp/mongodb-27017.sock/database?timeout=4");
	parse_test("mongodb://derick:test@/tmp/mongodb-27017.sock");
	parse_test("mongodb://derick:test@/tmp/mongodb-27017.sock/");
	parse_test("mongodb://derick:test@/tmp/mongodb-27017.sock/?timeout=4");
	parse_test("mongodb://derick:test@/tmp/mongodb-27017.sock/database");
	parse_test("mongodb://derick:test@/tmp/mongodb-27017.sock/database?timeout=4");


	return 0;
}
