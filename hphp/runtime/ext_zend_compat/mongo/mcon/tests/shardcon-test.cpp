#include "parse.h"
#include "manager.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	mongo_servers *servers;
	mongo_con_manager *manager;
	char *error_message = NULL;

	manager = mongo_init();
	manager->log_function = mongo_log_printf;

	servers = mongo_parse_init();
	mongo_parse_server_spec(manager, servers, "mongodb://whisky:13200,whisky:13201/phpunit", (char **) &error_message);
	mongo_servers_dump(manager, servers);
	servers->read_pref.type = MONGO_RP_PRIMARY_PREFERRED;
	mongo_get_read_write_connection(manager, servers, MONGO_CON_FLAG_READ, (char**) &error_message);
	if (error_message) {
		printf("ERROR: %s\n", error_message);
		free(error_message);
	}
	mongo_servers_dtor(servers);

	mongo_deinit(manager);

	return 0;
}
