#include "parse.h"
#include "manager.h"
#include "connections.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	mongo_servers *servers;
	mongo_con_manager *manager;
	char *error_message, *nonce;
	mongo_connection *con;

	manager = mongo_init();
	manager->log_function = mongo_log_printf;

	servers = mongo_parse_init();
	mongo_parse_server_spec(manager, servers, "mongodb://127.0.0.1:27017", (char**) &error_message);
	mongo_servers_dump(manager, servers);

	con = mongo_get_read_write_connection(manager, servers, MONGO_CON_FLAG_READ, (char**) &error_message);
	nonce = mongo_connection_getnonce(manager, con, (char**) &error_message);
	printf("The nonce is: %s\n", nonce);
	if (!mongo_connection_authenticate(manager, con, "phpunit", "admin", "wrong!", nonce, (char**) &error_message)) {
		printf("ERROR: %s\n", error_message);
		free(error_message);
		mongo_manager_connection_deregister(manager, con);
	}

	con = mongo_get_read_write_connection(manager, servers, MONGO_CON_FLAG_READ, (char**) &error_message);
	nonce = mongo_connection_getnonce(manager, con, (char**) &error_message);
	printf("The nonce is: %s\n", nonce);
	if (!mongo_connection_authenticate(manager, con, "phpunit", "admin", "admin", nonce, (char**) &error_message)) {
		printf("ERROR: %s\n", error_message);
		free(error_message);
	}

	free(nonce);
	mongo_servers_dtor(servers);

	mongo_deinit(manager);

	return 0;
}
