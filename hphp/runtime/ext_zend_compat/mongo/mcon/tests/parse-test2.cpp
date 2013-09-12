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
	/* Specific options */
	parse_test("mongodb://host1/?replicaSet=testset&readPreference=secondary&readPreferenceTags=");
	parse_test("mongodb://host1/?replicaSet=testset&readPreference=secondary&readPreferenceTags=dc:east");
	parse_test("mongodb://host1/?replicaSet=testset&readPreference=secondary&readPreferenceTags=dc:east,rack:d1");
	parse_test("mongodb://host1/?replicaSet=testset&readPreference=secondary&readPreferenceTags=dc:east,rack:d1&readPreferenceTags=dc:west");
	parse_test("mongodb://host1/?replicaSet=testset&readPreference=secondary&readPreferenceTags=dc:east,rack:d1&readPreferenceTags=");
	parse_test("mongodb://host1/?replicaSet=testset&readPreference=secondary&readPreferenceTags=dc:east,rack:d1&readPreferenceTags=foo");

	return 0;
}
