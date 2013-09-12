/**
 *  Copyright 2009-2013 10gen, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "collection.h"
#include "types.h"
#include "read_preference.h"
#include "manager.h"
#include "str.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef WIN32
#include <unistd.h>
#endif

/* Helpers */
char *mongo_connection_type(int type)
{
	switch (type) {
		case MONGO_NODE_STANDALONE: return "STANDALONE";
		case MONGO_NODE_PRIMARY: return "PRIMARY";
		case MONGO_NODE_SECONDARY: return "SECONDARY";
		case MONGO_NODE_ARBITER: return "ARBITER";
		case MONGO_NODE_MONGOS: return "MONGOS";
		default:
			return "UNKNOWN?";
	}
}
static void mongo_print_connection_info(mongo_con_manager *manager, mongo_connection *con, int level)
{
	int i;

	mongo_manager_log(manager, MLOG_RS, level,
		"- connection: type: %s, socket: %d, ping: %d, hash: %s",
		mongo_connection_type(con->connection_type),
		42, /* FIXME: STREAMS: Maybe we do need a union here: con->socket, */
		con->ping_ms,
		con->hash
	);
	for (i = 0; i < con->tag_count; i++) {
		mongo_manager_log(manager, MLOG_RS, level,
			"  - tag: %s", con->tags[i]
		);
	}
}

void mongo_print_connection_iterate_wrapper(mongo_con_manager *manager, void *elem)
{
	mongo_connection *con = (mongo_connection*) elem;

	mongo_print_connection_info(manager, con, MLOG_FINE);
}

char *mongo_read_preference_type_to_name(int type)
{
	switch (type) {
		case MONGO_RP_PRIMARY:             return "primary";
		case MONGO_RP_PRIMARY_PREFERRED:   return "primaryPreferred";
		case MONGO_RP_SECONDARY:           return "secondary";
		case MONGO_RP_SECONDARY_PREFERRED: return "secondaryPreferred";
		case MONGO_RP_NEAREST:             return "nearest";
	}
	return "unknown";
}

/* Collecting the correct servers */
static mcon_collection *filter_connections(mongo_con_manager *manager, int types, mongo_read_preference *rp)
{
	mcon_collection *col;
	mongo_con_manager_item *ptr = manager->connections;
	int current_pid, connection_pid;

	current_pid = getpid();
	col = mcon_init_collection(sizeof(mongo_connection*));

	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "filter_connections: adding connections:");
	while (ptr) {
		mongo_connection *con = (mongo_connection *) ptr->data;
		connection_pid = mongo_server_hash_to_pid(con->hash);

		if (connection_pid != current_pid) {
			mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "filter_connections: skipping %s as it doesn't match the current pid (%d)", con->hash, current_pid);
			manager->forget(manager, con);
		} else if (con->connection_type & types) {
			mongo_print_connection_info(manager, con, MLOG_FINE);
			mcon_collection_add(col, con);
		}
		ptr = ptr->next;
	}
	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "filter_connections: done");

	return col;
}

/* Wrappers for the different collection types */
static mcon_collection *mongo_rp_collect_primary(mongo_con_manager *manager, mongo_read_preference *rp)
{
	return filter_connections(manager, MONGO_NODE_PRIMARY, rp);
}

static mcon_collection *mongo_rp_collect_primary_and_secondary(mongo_con_manager *manager, mongo_read_preference *rp)
{
	return filter_connections(manager, MONGO_NODE_PRIMARY | MONGO_NODE_SECONDARY, rp);
}

static mcon_collection *mongo_rp_collect_secondary(mongo_con_manager *manager, mongo_read_preference *rp)
{
	return filter_connections(manager, MONGO_NODE_SECONDARY, rp);
}

static mcon_collection *mongo_rp_collect_any(mongo_con_manager *manager, mongo_read_preference *rp)
{
	/* We add the MONGO_NODE_STANDALONE and MONGO_NODE_MONGOS here, because
	 * that's needed for the MULTIPLE connection type. Right now, that only is
	 * used for MONGO_RP_NEAREST, and MONGO_RP_NEAREST uses this function (see
	 * below in mongo_find_all_candidate_servers(). */
	return filter_connections(manager, MONGO_NODE_STANDALONE | MONGO_NODE_PRIMARY | MONGO_NODE_SECONDARY | MONGO_NODE_MONGOS, rp);
}

static mcon_collection* mongo_find_all_candidate_servers(mongo_con_manager *manager, mongo_read_preference *rp)
{
	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "- all servers");
	/* Depending on read preference type, run the correct algorithm */
	switch (rp->type) {
		case MONGO_RP_PRIMARY:
			return mongo_rp_collect_primary(manager, rp);
			break;
		case MONGO_RP_PRIMARY_PREFERRED:
		case MONGO_RP_SECONDARY_PREFERRED:
			return mongo_rp_collect_primary_and_secondary(manager, rp);
			break;
		case MONGO_RP_SECONDARY:
			return mongo_rp_collect_secondary(manager, rp);
			break;
		case MONGO_RP_NEAREST:
			return mongo_rp_collect_any(manager, rp);
			break;
		default:
			return NULL;
	}
}

static int candidate_matches_tags(mongo_con_manager *manager, mongo_connection *con, mongo_read_preference_tagset *tagset)
{
	int i, j, found = 0;

	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "candidate_matches_tags: checking tags on %s", con->hash);
	for (i = 0; i < tagset->tag_count; i++) {
		for (j = 0; j < con->tag_count; j++) {
			if (strcmp(tagset->tags[i], con->tags[j]) == 0) {
				found++;
				mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "candidate_matches_tags: found %s", con->tags[j]);
			}
		}
	}
	if (found == tagset->tag_count) {
		mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "candidate_matches_tags: all tags matched for %s", con->hash);
		return 1;
	} else {
		mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "candidate_matches_tags: not all tags matched for %s", con->hash);
		return 0;
	}
}

static mcon_collection* mongo_filter_candidates_by_tagset(mongo_con_manager *manager, mcon_collection *candidates, mongo_read_preference_tagset *tagset, int rp_type)
{
	int              i;
	mcon_collection *tmp;

	tmp = mcon_init_collection(sizeof(mongo_connection*));
	for (i = 0; i < candidates->count; i++) {
		if (rp_type == MONGO_RP_PRIMARY_PREFERRED && (((mongo_connection *) candidates->data[i])->connection_type == MONGO_NODE_PRIMARY)) {
			mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "candidate_matches_tags: added primary regardless of tags: %s", ((mongo_connection *) candidates->data[i])->hash);
			mcon_collection_add(tmp, candidates->data[i]);
		} else if (candidate_matches_tags(manager, (mongo_connection *) candidates->data[i], tagset)) {
			mcon_collection_add(tmp, candidates->data[i]);
		}
	}
	return tmp;
}

char *mongo_read_preference_squash_tagset(mongo_read_preference_tagset *tagset)
{
	int    i;
	struct mcon_str str = { 0 };

	for (i = 0; i < tagset->tag_count; i++) {
		if (i) {
			mcon_str_addl(&str, ", ", 2, 0);
		}
		mcon_str_add(&str, tagset->tags[i], 0);
	}
	return str.d;
}

static mcon_collection *mongo_filter_candidates_by_replicaset_name(mongo_con_manager *manager, mcon_collection *candidates, mongo_servers *servers)
{
	int              i;
	mcon_collection *filtered;
	char            *candidate_hash;
	char            *candidate_replsetname;

	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "limiting to servers with same replicaset name");
	filtered = mcon_init_collection(sizeof(mongo_connection*));

	for (i = 0; i < candidates->count; i++) {
		candidate_hash = ((mongo_connection *) candidates->data[i])->hash;
		mongo_server_split_hash(candidate_hash, NULL, NULL, (char**) &candidate_replsetname, NULL, NULL, NULL, NULL);

		/* Filter out all servers that don't have the replicaset name the same
		 * as what we have in the server definition struct. But only when the
		 * repl_set_name in the server definition struct is actually *set*. If
		 * not, we allow all connections. This make sure we can sort of handle
		 * [ replicaset => true ], although it would not support one PHP worker
		 * process connecting to multiple replicasets correctly. */
		if (candidate_replsetname) {
			if (!servers->options.repl_set_name || strcmp(candidate_replsetname, servers->options.repl_set_name) == 0) {
				mongo_print_connection_info(manager, (mongo_connection *) candidates->data[i], MLOG_FINE);
				mcon_collection_add(filtered, (mongo_connection *) candidates->data[i]);
			}
			free(candidate_replsetname);
		}
	}

	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "limiting to servers with same replicaset name: done");

	return filtered;
}

static mcon_collection *mongo_filter_candidates_by_seed(mongo_con_manager *manager, mcon_collection *candidates, mongo_servers *servers)
{
	int              i, j;
	mcon_collection *filtered;
	char            *server_hash;

	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "limiting by seeded/discovered servers");
	filtered = mcon_init_collection(sizeof(mongo_connection*));

	for (j = 0; j < servers->count; j++) {
		server_hash = mongo_server_create_hash(servers->server[j]);
		for (i = 0; i < candidates->count; i++) {
			if (strcmp(((mongo_connection *) candidates->data[i])->hash, server_hash) == 0) {
				mongo_print_connection_info(manager, (mongo_connection *) candidates->data[i], MLOG_FINE);
				mcon_collection_add(filtered, (mongo_connection *) candidates->data[i]);
			}
		}
		free(server_hash);
	}

	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "limiting by seeded/discovered servers: done");

	return filtered;
}

static mcon_collection *mongo_filter_candidates_by_credentials(mongo_con_manager *manager, mcon_collection *candidates, mongo_servers *servers)
{
	int              i;
	char            *db, *username, *auth_hash, *hashed = NULL;
	mcon_collection *filtered;

	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "limiting by credentials");
	filtered = mcon_init_collection(sizeof(mongo_connection*));

	for (i = 0; i < candidates->count; i++) {
		db = username = auth_hash = hashed = NULL;
		mongo_server_split_hash(((mongo_connection *) candidates->data[i])->hash, NULL, NULL, NULL, &db, &username, &auth_hash, NULL);
		if (servers->server[0]->username && servers->server[0]->password && servers->server[0]->db) {
			if (strcmp(db, servers->server[0]->db) != 0) {
				mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "- skipping '%s', database didn't match ('%s' vs '%s')", ((mongo_connection *) candidates->data[i])->hash, db, servers->server[0]->db);
				goto skip;
			}
			if (strcmp(username, servers->server[0]->username) != 0) {
				mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "- skipping '%s', username didn't match ('%s' vs '%s')", ((mongo_connection *) candidates->data[i])->hash, username, servers->server[0]->username);
				goto skip;
			}
			hashed = mongo_server_create_hashed_password(username, servers->server[0]->password);
			if (strcmp(auth_hash, hashed) != 0) {
				mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "- skipping '%s', authentication hash didn't match ('%s' vs '%s')", ((mongo_connection *) candidates->data[i])->hash, auth_hash, hashed);
				goto skip;
			}
		}

		mcon_collection_add(filtered, (mongo_connection *) candidates->data[i]);
		mongo_print_connection_info(manager, (mongo_connection *) candidates->data[i], MLOG_FINE);
skip:
		if (db) {
			free(db);
		}
		if (username) {
			free(username);
		}
		if (auth_hash) {
			free(auth_hash);
		}
		if (hashed) {
			free(hashed);
		}
	}
	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "limiting by credentials: done");

	return filtered;
}

mcon_collection* mongo_find_candidate_servers(mongo_con_manager *manager, mongo_read_preference *rp, mongo_servers *servers)
{
	int              i;
	mcon_collection *all, *filtered;

	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "finding candidate servers");
	all = mongo_find_all_candidate_servers(manager, rp);

	if (servers->options.con_type == MONGO_CON_TYPE_REPLSET) {
		filtered = mongo_filter_candidates_by_replicaset_name(manager, all, servers);
	} else {
		filtered = mongo_filter_candidates_by_seed(manager, all, servers);
	}
	mcon_collection_free(all);
	all = filtered;

	filtered = mongo_filter_candidates_by_credentials(manager, all, servers);
	mcon_collection_free(all);
	all = filtered;

	if (rp->tagset_count != 0) {
		mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "limiting by tagsets");
		/* If we have tagsets configured for the replicaset then we need to do
		 * some more filtering */
		for (i = 0; i < rp->tagset_count; i++) {
			char *tmp_ts = mongo_read_preference_squash_tagset(rp->tagsets[i]);

			mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "checking tagset: %s", tmp_ts);
			filtered = mongo_filter_candidates_by_tagset(manager, all, rp->tagsets[i], rp->type);
			mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "tagset %s matched %d candidates", tmp_ts, filtered->count);
			free(tmp_ts);

			if (filtered->count > 0) {
				mcon_collection_free(all);
				return filtered;
			}
		}
		mcon_collection_free(filtered);
		mcon_collection_free(all);
		return NULL;
	} else {
		return all;
	}
}

/* Sorting the servers */
static int mongo_rp_sort_primary_preferred(const void* a, const void *b)
{
	mongo_connection *con_a = *(mongo_connection**) a;
	mongo_connection *con_b = *(mongo_connection**) b;

	/* First we prefer primary over secondary, and if the field type is the
	 * same, we sort on ping_ms again. *_SECONDARY is a higher constant value
	 * than *_PRIMARY, so we sort descendingly by connection_type */
	if (con_a->connection_type > con_b->connection_type) {
		return 1;
	} else if (con_a->connection_type < con_b->connection_type) {
		return -1;
	} else {
		if (con_a->ping_ms > con_b->ping_ms) {
			return 1;
		} else if (con_a->ping_ms < con_b->ping_ms) {
			return -1;
		}
	}
	return 0;
}

static int mongo_rp_sort_secondary_preferred(const void* a, const void *b)
{
	mongo_connection *con_a = *(mongo_connection**) a;
	mongo_connection *con_b = *(mongo_connection**) b;

	/* First we prefer secondary over primary, and if the field type is the
	 * same, we sort on ping_ms again. *_SECONDARY is a higher constant value
	 * than *_PRIMARY. */
	if (con_a->connection_type < con_b->connection_type) {
		return 1;
	} else if (con_a->connection_type > con_b->connection_type) {
		return -1;
	} else {
		if (con_a->ping_ms > con_b->ping_ms) {
			return 1;
		} else if (con_a->ping_ms < con_b->ping_ms) {
			return -1;
		}
	}
	return 0;
}

static int mongo_rp_sort_any(const void* a, const void *b)
{
	mongo_connection *con_a = *(mongo_connection**) a;
	mongo_connection *con_b = *(mongo_connection**) b;

	if (con_a->ping_ms > con_b->ping_ms) {
		return 1;
	} else if (con_a->ping_ms < con_b->ping_ms) {
		return -1;
	}
	return 0;
}

/* This method is the master for selecting the correct algorithm for the order
 * of servers in which to try the candidate servers that we've previously found */
mcon_collection *mongo_sort_servers(mongo_con_manager *manager, mcon_collection *col, mongo_read_preference *rp)
{
	mongo_connection_sort_t *sort_function;

	switch (rp->type) {
		case MONGO_RP_PRIMARY:
			/* Should not really have to do anything as there is only going to
			 * be one server */
			sort_function = mongo_rp_sort_any;
			break;

		case MONGO_RP_PRIMARY_PREFERRED:
			sort_function = mongo_rp_sort_primary_preferred;
			break;

		case MONGO_RP_SECONDARY:
			sort_function = mongo_rp_sort_any;
			break;

		case MONGO_RP_SECONDARY_PREFERRED:
			sort_function = mongo_rp_sort_secondary_preferred;
			break;

		case MONGO_RP_NEAREST:
			sort_function = mongo_rp_sort_any;
			break;

		default:
			return NULL;
	}
	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "sorting servers by priority and ping time");
	qsort(col->data, col->count, sizeof(mongo_connection*), sort_function);
	mcon_collection_iterate(manager, col, mongo_print_connection_iterate_wrapper);
	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "sorting servers: done");
	return col;
}

mcon_collection *mongo_select_nearest_servers(mongo_con_manager *manager, mcon_collection *col, mongo_read_preference *rp)
{
	mcon_collection *filtered;
	int              i, nearest_ping;

	filtered = mcon_init_collection(sizeof(mongo_connection*));

	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "selecting near servers");

	switch (rp->type) {
		case MONGO_RP_PRIMARY:
		case MONGO_RP_PRIMARY_PREFERRED:
		case MONGO_RP_SECONDARY:
		case MONGO_RP_SECONDARY_PREFERRED:
		case MONGO_RP_NEAREST:
			/* The nearest ping time is in the first element */
			nearest_ping = ((mongo_connection*)col->data[0])->ping_ms;
			mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "selecting near servers: nearest is %dms", nearest_ping);

			/* FIXME: Change to iterator later */
			for (i = 0; i < col->count; i++) {
				if (((mongo_connection*)col->data[i])->ping_ms <= nearest_ping + MONGO_RP_CUTOFF) {
					mcon_collection_add(filtered, col->data[i]);
				}
			}
			break;

		default:
			return NULL;
	}

	/* Clean up the old collection that we no longer need */
	mcon_collection_free(col);

	mcon_collection_iterate(manager, filtered, mongo_print_connection_iterate_wrapper);
	mongo_manager_log(manager, MLOG_RS, MLOG_FINE, "selecting near server: done");

	return filtered;
}

/* The algorithm works as follows: In case we have a read preference of
 * primary, secondary or nearest the set will always contain a set of all nodes
 * that should always be considered to be returned. With primary, there is only
 * going to be one node, with primary the set contains only secondaries and
 * with nearest we do not prefer a secondary over a primary or v.v.
 *
 * In case we have a read preference of primaryPreferred or
 * secondaryPreferred, we need to do a bit more logic for selecting the node
 * that we use. */
mongo_connection *mongo_pick_server_from_set(mongo_con_manager *manager, mcon_collection *col, mongo_read_preference *rp)
{
	mongo_connection *con = NULL;
	int entry;

	/* If we prefer the primary, we check whether the first node is a primary
	 * (which it should be if it's available and sorted according to primary >
	 * secondary). If the first node in the list is no primary, we fall back
	 * to picking a random node from the set. */
	if (rp->type == MONGO_RP_PRIMARY_PREFERRED) {
		if (((mongo_connection*)col->data[0])->connection_type == MONGO_NODE_PRIMARY) {
			mongo_manager_log(manager, MLOG_RS, MLOG_INFO, "pick server: the primary");
			con = (mongo_connection*)col->data[0];
			mongo_print_connection_info(manager, con, MLOG_INFO);
			return con;
		}
	}

	/* If we prefer a secondary, then we need to ignore the last item from the
	 * list, as this is the primary node - but only if there is more than one
	 * node in the list AND the last node in the list is a primary. */
	if (rp->type == MONGO_RP_SECONDARY_PREFERRED) {
		if (
			(col->count > 1) &&
			(((mongo_connection*)col->data[col->count - 1])->connection_type == MONGO_NODE_PRIMARY)
		) {
			entry = rand() % (col->count - 1);
			mongo_manager_log(manager, MLOG_RS, MLOG_INFO, "pick server: random element %d while ignoring the primary", entry);
			con = (mongo_connection*)col->data[entry];
			mongo_print_connection_info(manager, con, MLOG_INFO);
			return con;
		}
	}

	/* For now, we just pick a random server from the set */
	entry = rand() % col->count;
	mongo_manager_log(manager, MLOG_RS, MLOG_INFO, "pick server: random element %d", entry);
	con = (mongo_connection*)col->data[entry];
	mongo_print_connection_info(manager, con, MLOG_INFO);
	return con;
}

/* Tagset helpers */
void mongo_read_preference_add_tag(mongo_read_preference_tagset *tagset, char *name, char *value)
{
	tagset->tag_count++;
	tagset->tags = (char**) realloc(tagset->tags, tagset->tag_count * sizeof(char*));
	tagset->tags[tagset->tag_count - 1] = (char*) malloc(strlen(name) + strlen(value) + 2);
	snprintf(tagset->tags[tagset->tag_count - 1], strlen(name) + strlen(value) + 2, "%s:%s", name, value);
}

void mongo_read_preference_add_tagset(mongo_read_preference *rp, mongo_read_preference_tagset *tagset)
{
	rp->tagset_count++;
	rp->tagsets = (mongo_read_preference_tagset**) realloc(rp->tagsets, rp->tagset_count * sizeof(mongo_read_preference_tagset*));
	rp->tagsets[rp->tagset_count - 1] = tagset;
}

void mongo_read_preference_tagset_dtor(mongo_read_preference_tagset *tagset)
{
	int i;

	if (tagset->tag_count > 0) {
		for (i = 0; i < tagset->tag_count; i++) {
			free(tagset->tags[i]);
		}

		tagset->tag_count = 0;
		free(tagset->tags);
	}

	free(tagset);
}

void mongo_read_preference_dtor(mongo_read_preference *rp)
{
	int i;

	if (rp->tagset_count == 0) {
		return;
	}

	for (i = 0; i < rp->tagset_count; i++) {
		mongo_read_preference_tagset_dtor(rp->tagsets[i]);
	}
	rp->tagset_count = 0;
	free(rp->tagsets);
	/* We are not freeing *rp itself, as that's not always a pointer */
}

void mongo_read_preference_copy(mongo_read_preference *from, mongo_read_preference *to)
{
	int i, j;

	to->type = from->type;
	to->tagset_count = from->tagset_count;
	if (!from->tagset_count) {
		to->tagset_count = 0;
		to->tagsets = NULL;
		return;
	}
	to->tagsets = (mongo_read_preference_tagset**) calloc(1, to->tagset_count * sizeof(mongo_read_preference_tagset*));

	for (i = 0; i < from->tagset_count; i++) {
		to->tagsets[i] = (mongo_read_preference_tagset*) calloc(1, sizeof(mongo_read_preference_tagset));
		to->tagsets[i]->tag_count = from->tagsets[i]->tag_count;
		to->tagsets[i]->tags = (char**) calloc(1, to->tagsets[i]->tag_count * sizeof(char*));

		for (j = 0; j < from->tagsets[i]->tag_count; j++) {
			to->tagsets[i]->tags[j] = strdup(from->tagsets[i]->tags[j]);
		}
	}
}

void mongo_read_preference_replace(mongo_read_preference *from, mongo_read_preference *to)
{
	mongo_read_preference_dtor(to);
	mongo_read_preference_copy(from, to);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
