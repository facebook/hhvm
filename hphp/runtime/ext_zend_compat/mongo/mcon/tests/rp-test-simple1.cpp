#include "manager.h"
#include "collection.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int last_socket = 4;

mongo_connection *create_con(mongo_con_manager *manager, int type, int ping_ms, char *hash)
{
	mongo_connection *con;

	con = malloc(sizeof(mongo_connection));
	con->connection_type = type;
	con->socket = ++last_socket;
	con->ping_ms = ping_ms;
	con->hash = strdup(hash);
	mongo_manager_connection_register(manager, con);

	return con;
}

int main(void)
{
	mongo_con_manager *manager;
	mongo_read_preference rp;
	mongo_connection *con;
	mcon_collection *collection;

	manager = mongo_init();

	/* Register connections */
	con = create_con(manager, MONGO_NODE_SECONDARY, 13, "whisky:13001;X;10120");
	con = create_con(manager, MONGO_NODE_PRIMARY, 8, "whisky:13000;X;10120");
	con = create_con(manager, MONGO_NODE_SECONDARY, 19, "whisky:13002;X;10120");
	con = create_con(manager, MONGO_NODE_SECONDARY, 843, "whisky:13003;X;10120");

	/* Configure RP */
	rp.type = MONGO_RP_PRIMARY_PREFERRED;
	collection = mongo_find_candidate_servers(manager, &rp, NULL);
	if (collection->count) {
		collection = mongo_sort_servers(manager, collection, &rp);
		printf("collection size: %d\n", collection->count);
		collection = mongo_select_nearest_servers(manager, collection, &rp);
		printf("collection size: %d\n", collection->count);
		con = mongo_pick_server_from_set(manager, collection, &rp);
		mongo_print_connection_iterate_wrapper(manager, con);
	} else {
		printf("no servers found\n");
	}

	/* Cleaning up */
	mcon_collection_free(collection);	
	mongo_deinit(manager);

	return 0;
}
