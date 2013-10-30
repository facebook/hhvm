#include "parse.h"
#include "manager.h"
#include <stdio.h>

int main(void)
{
	mongo_servers *servers;
	mongo_con_manager *manager;
	char *error_message;

	manager = mongo_init();
	manager->log_function = mongo_log_printf;

	servers = mongo_parse_init();
	mongo_parse_server_spec(manager, servers, "mongodb://whisky:13002,whisky:13000,whisky:13001/demo?replicaSet=seta", (char **) &error_message);
	mongo_servers_dump(manager, servers);
	servers->read_pref.type = MONGO_RP_PRIMARY_PREFERRED;
	mongo_get_read_write_connection(manager, servers, MONGO_CON_FLAG_READ, (char**) &error_message);
	if (error_message) {
		printf("ERROR: %s\n", error_message);
	}
	mongo_servers_dtor(servers);

	mongo_deinit(manager);

	return 0;
}
