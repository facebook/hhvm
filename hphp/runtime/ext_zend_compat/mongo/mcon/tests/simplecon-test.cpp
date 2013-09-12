#include "parse.h"
#include "manager.h"

int main(void)
{
	mongo_servers *servers;
	mongo_con_manager *manager;
	char *error_message;

	manager = mongo_init();
	manager->log_function = mongo_log_printf;

	servers = mongo_parse_init();
	mongo_parse_server_spec(manager, servers, "mongodb://127.0.0.1:27017", (char**) &error_message);
	mongo_servers_dump(manager, servers);
	mongo_get_read_write_connection(manager, servers, MONGO_CON_FLAG_READ, (char**) &error_message);
	mongo_servers_dtor(servers);

	servers = mongo_parse_init();
	mongo_parse_server_spec(manager, servers, "mongodb://127.0.0.2:27017", (char**) &error_message);
	mongo_servers_dump(manager, servers);
	mongo_get_read_write_connection(manager, servers, MONGO_CON_FLAG_READ, (char**) &error_message);
	mongo_servers_dtor(servers);

	servers = mongo_parse_init();
	mongo_parse_server_spec(manager, servers, "mongodb://127.0.0.1:27017", (char**) &error_message);
	mongo_servers_dump(manager, servers);
	mongo_get_read_write_connection(manager, servers, MONGO_CON_FLAG_READ, (char**) &error_message);
	mongo_servers_dtor(servers);

	mongo_deinit(manager);

	return 0;
}
