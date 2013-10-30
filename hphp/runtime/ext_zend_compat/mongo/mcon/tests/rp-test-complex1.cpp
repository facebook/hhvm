#include "manager.h"
#include "collection.h"
#include "types.h"
#include "read_preference.h"
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
	con->tag_count = 0;
	con->tags = NULL;
	mongo_manager_connection_register(manager, con);

	return con;
}

void add_tag(mongo_connection *con, char *tag)
{
	con->tags = realloc(con->tags, (con->tag_count + 1) * sizeof(char*));
	con->tags[con->tag_count] = strdup(tag);
	con->tag_count++;
}

void add_rp_tag0(mongo_read_preference *rp)
{
	mongo_read_preference_tagset *tmp_ts = calloc(1, sizeof(mongo_read_preference_tagset));
	mongo_read_preference_add_tagset(rp, tmp_ts);
}

void add_rp_tag(mongo_read_preference *rp, char *name1, char *value1)
{
	mongo_read_preference_tagset *tmp_ts = calloc(1, sizeof(mongo_read_preference_tagset));
	mongo_read_preference_add_tag(tmp_ts, name1, value1);
	mongo_read_preference_add_tagset(rp, tmp_ts);
}

void add_rp_tag2(mongo_read_preference *rp, char *name1, char *value1, char *name2, char *value2)
{
	mongo_read_preference_tagset *tmp_ts = calloc(1, sizeof(mongo_read_preference_tagset));
	mongo_read_preference_add_tag(tmp_ts, name1, value1);
	mongo_read_preference_add_tag(tmp_ts, name2, value2);
	mongo_read_preference_add_tagset(rp, tmp_ts);
}

int main(void)
{
	mongo_con_manager *manager;
	mongo_read_preference rp = { MONGO_RP_PRIMARY, 0 };
	mongo_connection *con;
	mcon_collection *collection;

	manager = mongo_init();
	manager->log_function = mongo_log_printf;

	/* Register connections */
	con = create_con(manager, MONGO_NODE_PRIMARY, 8, "whisky:13000;X;10120");
	add_tag(con, "dc:east"); add_tag(con, "use:reporting");
	con = create_con(manager, MONGO_NODE_SECONDARY, 13, "whisky:13001;X;10120");
	con = create_con(manager, MONGO_NODE_SECONDARY, 19, "whisky:13002;X;10120");
	add_tag(con, "dc:east"); add_tag(con, "use:accounting");
	con = create_con(manager, MONGO_NODE_SECONDARY, 843, "whisky:13003;X;10120");
	add_tag(con, "dc:east"); add_tag(con, "use:accounting");

	/* Configure RP */
	rp.type = MONGO_RP_SECONDARY_PREFERRED;
	add_rp_tag2(&rp, "dc", "west", "use", "reporting");
	add_rp_tag(&rp, "use", "documenting");
	add_rp_tag0(&rp);
	collection = mongo_find_candidate_servers(manager, &rp, NULL);
	if (collection && collection->count) {
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
	if (collection) {
		mcon_collection_free(collection);
	}
	mongo_deinit(manager);

	return 0;
}
