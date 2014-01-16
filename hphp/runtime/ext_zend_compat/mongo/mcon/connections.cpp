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
#include <string.h>
#include "types.h"
#include "utils.h"
#include "parse.h"
#include "manager.h"
#include "connections.h"
#include "io.h"
#include "str.h"
#include "bson_helpers.h"
#include "mini_bson.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define INT_32  4
#define FLAGS   0

#pragma GCC optimize "no-strict-aliasing"

#define MONGO_REPLY_FLAG_QUERY_FAILURE 0x02

static void mongo_close_socket(int socket, int why);

/* Helper functions */
int mongo_connection_get_reqid(mongo_connection *con)
{
	con->last_reqid++;
	return con->last_reqid;
}

static int mongo_util_connect__sockaddr(struct sockaddr *sa, int family, char *host, int port, char **errmsg)
{
#ifndef WIN32
	if (family == AF_UNIX) {
		struct sockaddr_un* su = (struct sockaddr_un*)(sa);
		su->sun_family = AF_UNIX;
		strncpy(su->sun_path, host, sizeof(su->sun_path));
	} else {
#endif
		struct hostent *hostinfo;
		struct sockaddr_in* si = (struct sockaddr_in*)(sa);

		si->sin_family = AF_INET;
		si->sin_port = htons(port);
		hostinfo = (struct hostent*)gethostbyname(host);

		if (hostinfo == NULL) {
			*errmsg = (char*) malloc(256);
			snprintf(*errmsg, 256, "Couldn't get host info for %s", host);
			return 0;
		}

#ifndef WIN32
		si->sin_addr = *((struct in_addr*)hostinfo->h_addr);
	}
#else
	si->sin_addr.s_addr = ((struct in_addr*)(hostinfo->h_addr))->s_addr;
#endif

	return 1;
}

/* This function does the actual connecting The results of this function are
 * stored in mongo_connection->socket, which is a void* to be able to store
 * various different backends (f.e. the PHP io_streams stores a php_stream*)
 *
 * Returns an integer (masquerading as a void*) on success, NULL on failure. */
void* mongo_connection_connect(mongo_con_manager *manager, mongo_server_def *server, mongo_server_options *options, char **error_message)
{
	struct sockaddr*   sa;
	struct sockaddr_in si;
	socklen_t          sn;
	int                family;
	struct timeval     tval;
	int                connected;
	int                status;
	int                tmp_socket;

#ifdef WIN32
	WORD       version;
	WSADATA    wsaData;
	int        size, error;
	u_long     no = 0;
	const char yes = 1;
#else
	struct sockaddr_un su;
	uint               size;
	int                yes = 1;
#endif

	*error_message = NULL;

#ifdef WIN32
	family = AF_INET;
	sa = (struct sockaddr*)(&si);
	sn = sizeof(si);

	version = MAKEWORD(2,2);
	error = WSAStartup(version, &wsaData);

	if (error != 0) {
		return NULL;
	}

	/* create socket */
	tmp_socket = socket(family, SOCK_STREAM, 0);
	if (tmp_socket == INVALID_SOCKET) {
		*error_message = strdup(strerror(errno));
		return NULL;
	}

#else
	/* domain socket */
	if (server->port == 0) {
		family = AF_UNIX;
		sa = (struct sockaddr*)(&su);
		sn = sizeof(su);
	} else {
		family = AF_INET;
		sa = (struct sockaddr*)(&si);
		sn = sizeof(si);
	}

	/* create socket */
	if ((tmp_socket = socket(family, SOCK_STREAM, 0)) == -1) {
		*error_message = strdup(strerror(errno));
		return NULL;
	}
#endif

	/* TODO: Move this to within the loop & use real timeout setting */
	/* connection timeout: set in ms (current default 1 sec) */
	tval.tv_sec = options->connectTimeoutMS <= 0 ? 1 : options->connectTimeoutMS / 1000;
	tval.tv_usec = options->connectTimeoutMS <= 0 ? 0 : (options->connectTimeoutMS % 1000) * 1000;

	/* get addresses */
	if (mongo_util_connect__sockaddr(sa, family, server->host, server->port, error_message) == 0) {
		goto error;
	}

	setsockopt(tmp_socket, SOL_SOCKET, SO_KEEPALIVE, &yes, INT_32);
	setsockopt(tmp_socket, IPPROTO_TCP, TCP_NODELAY, &yes, INT_32);

#ifdef WIN32
	ioctlsocket(tmp_socket, FIONBIO, (u_long*)&yes);
#else
	fcntl(tmp_socket, F_SETFL, FLAGS|O_NONBLOCK);
#endif

	/* connect */
	status = connect(tmp_socket, sa, sn);
	if (status < 0) {
#ifdef WIN32
		errno = WSAGetLastError();
		if (errno != WSAEINPROGRESS && errno != WSAEWOULDBLOCK) {
#else
		if (errno != EINPROGRESS) {
#endif
			*error_message = strdup(strerror(errno));
			goto error;
		}

		while (1) {
			fd_set rset, wset, eset;

			FD_ZERO(&rset);
			FD_SET(tmp_socket, &rset);
			FD_ZERO(&wset);
			FD_SET(tmp_socket, &wset);
			FD_ZERO(&eset);
			FD_SET(tmp_socket, &eset);

			if (select(tmp_socket+1, &rset, &wset, &eset, &tval) == 0) {
				*error_message = (char*) malloc(256);
				snprintf(*error_message, 256, "Timed out after %d ms", options->connectTimeoutMS);
				goto error;
			}

			/* if our descriptor has an error */
			if (FD_ISSET(tmp_socket, &eset)) {
				*error_message = strdup(strerror(errno));
				goto error;
			}

			/* if our descriptor is ready break out */
			if (FD_ISSET(tmp_socket, &wset) || FD_ISSET(tmp_socket, &rset)) {
				break;
			}
		}

		size = sn;

		connected = getpeername(tmp_socket, sa, &size);
		if (connected == -1) {
			*error_message = strdup(strerror(errno));
			goto error;
		}
	}

	/* reset flags */
#ifdef WIN32
	ioctlsocket(tmp_socket, FIONBIO, &no);
#else
	fcntl(tmp_socket, F_SETFL, FLAGS);
#endif

	return (void *) (long) tmp_socket;

error:
	mongo_close_socket(tmp_socket, MONGO_CLOSE_BROKEN);
	return NULL;
}

static void mongo_close_socket(int socket, int why)
{
#ifdef WIN32
	shutdown(socket, SD_BOTH);
	closesocket(socket);
	WSACleanup();
#else
	shutdown(socket, SHUT_RDWR);
	close(socket);
#endif
}

void mongo_connection_close(mongo_connection *con, int why)
{
	mongo_close_socket((int) (long) con->socket, why);
}

mongo_connection *mongo_connection_create(mongo_con_manager *manager, char *hash, mongo_server_def *server_def, mongo_server_options *options, char **error_message)
{
	mongo_connection *tmp;

	/* Init struct */
	tmp = (mongo_connection*) malloc(sizeof(mongo_connection));
	memset(tmp, 0, sizeof(mongo_connection));
	tmp->last_reqid = rand();
	tmp->connection_type = MONGO_NODE_STANDALONE;

	/* Store hash */
	tmp->hash = strdup(hash);

	/* Connect */
	mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "connection_create: creating new connection for %s:%d", server_def->host, server_def->port);
	tmp->socket = manager->connect(manager, server_def, options, error_message);
	if (!tmp->socket) {
		mongo_manager_log(manager, MLOG_CON, MLOG_WARN, "connection_create: error while creating connection for %s:%d: %s", server_def->host, server_def->port, *error_message);
		mongo_manager_blacklist_register(manager, tmp);
		free(tmp->hash);
		free(tmp);
		return NULL;
	}

	/* We call get_server_flags to the maxBsonObjectSize data */
	if (!mongo_connection_get_server_flags(manager, tmp, options, error_message)) {
		mongo_manager_log(manager, MLOG_CON, MLOG_WARN, "server_flags: error while getting the server configuration %s:%d: %s", server_def->host, server_def->port, *error_message);
		free(tmp);
		return NULL;
	}

	return tmp;
}

void mongo_connection_destroy(mongo_con_manager *manager, void *data, int why)
{
	int current_pid, connection_pid;
	int i;
	mongo_connection *con = (mongo_connection *)data;

	current_pid = getpid();
	connection_pid = mongo_server_hash_to_pid(con->hash);

	/* Only close the connection if it matches the current PID */
	if (current_pid == connection_pid) {
		mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "mongo_connection_destroy: Destroying connection object for %s", con->hash);

		if (con->socket) {
			mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "mongo_connection_destroy: Closing socket for %s.", con->hash);
			manager->close(con, why);
			con->socket = NULL;

			for (i = 0; i < con->tag_count; i++) {
				free(con->tags[i]);
			}
			free(con->tags);

			if (con->cleanup_list) {
				mongo_connection_deregister_callback *ptr = con->cleanup_list;
				mongo_connection_deregister_callback *prev;
				do {
					if (ptr->callback_data) {
						ptr->mongo_cleanup_cb(ptr->callback_data);
					}

					if (!ptr->next) {
						free(ptr);
						ptr = NULL;
						break;
					}
					prev = ptr;
					ptr = ptr->next;
					free(prev);
					prev = NULL;
				} while(1);
				con->cleanup_list = NULL;
			}
			free(con->hash);
			free(con);
		}
	} else {
		mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "mongo_connection_destroy: The process pid (%d) for %s doesn't match the connection pid (%d).", current_pid, con->hash, connection_pid);
	}
}

void mongo_connection_forget(mongo_con_manager *manager, mongo_connection *con)
{
	/* FIXME: Should we remove it from our connection registry ? */
}

#define MONGO_REPLY_HEADER_SIZE 36

/* Returns 1 if it worked, and 0 if it didn't. If 0 is returned, *error_message
 * is set and must be free()d. On success *data_buffer is set and must be free()d */
static int mongo_connect_send_packet(mongo_con_manager *manager, mongo_connection *con, mongo_server_options *options, mcon_str *packet, char **data_buffer, char **error_message)
{
	int            read;
	uint32_t       data_size;
	int             reply_buffer[MONGO_REPLY_HEADER_SIZE];
	uint32_t       flags; /* To check for query reply status */

	/* Send and wait for reply */
	if (manager->send(con, options, packet->d, packet->l, error_message) == -1) {
		mcon_str_ptr_dtor(packet);
		return 0;
	}
	mcon_str_ptr_dtor(packet);
	read = manager->recv_header(con, options, options->socketTimeoutMS, reply_buffer, MONGO_REPLY_HEADER_SIZE, error_message);
	if (read < 0) {
		/* Error already populated */
		return 0;
	}

	mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "send_packet: read from header: %d", read);
	if (read < MONGO_REPLY_HEADER_SIZE) {
		*error_message = (char*) malloc(256);
		snprintf(*error_message, 256, "send_package: the amount of bytes read (%d) is less than the header size (%d)", read, MONGO_REPLY_HEADER_SIZE);
		return 0;
	}

	/* Read result flags */
	flags = MONGO_32(*(int*)(reply_buffer + sizeof(int32_t) * 4));

	/* Read the rest of the data */
	data_size = MONGO_32(*(int*)(reply_buffer)) - MONGO_REPLY_HEADER_SIZE;
	mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "send_packet: data_size: %d", data_size);

	/* Check size limits */
	if (con->max_bson_size && data_size > con->max_bson_size) {
		*error_message = (char*) malloc(256);
		snprintf(*error_message, 256, "send_package: data corruption: the returned size of the reply (%d) is larger than the maximum allowed size (%d)", data_size, con->max_bson_size);
		return 0;
	}

	/* Read data */
	*data_buffer = (char*) malloc(data_size + 1);
	if (manager->recv_data(con, options, options->socketTimeoutMS, *data_buffer, data_size, error_message) <= 0) {
		free(*data_buffer);
		return 0;
	}

	/* Check for a query error */
	if (flags & MONGO_REPLY_FLAG_QUERY_FAILURE) {
		char *ptr = *data_buffer + sizeof(int32_t); /* Skip the length */
		char *err;
		int32_t code;

		/* Find the error */
		if (bson_find_field_as_string(ptr, "$err", &err)) {
			*error_message = (char*) malloc(256 + strlen(err));

			if (bson_find_field_as_int32(ptr, "code", &code)) {
				snprintf(*error_message, 256 + strlen(err), "send_package: the query returned a failure: %s (code: %d)", err, code);
			} else {
				snprintf(*error_message, 256 + strlen(err), "send_package: the query returned a failure: %s", err);
			}
		} else {
			*error_message = strdup("send_package: the query returned an unknown error");
		}

		free(*data_buffer);
		return 0;
	}

	return 1;
}

int mongo_connection_ping_check(mongo_con_manager *manager, int last_ping, struct timeval *start)
{
	gettimeofday(start, NULL);
	if ((last_ping + manager->ping_interval) > start->tv_sec) {
		mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "is_ping: skipping: last ran at %ld, now: %ld, time left: %ld", last_ping, start->tv_sec, last_ping + manager->ping_interval - start->tv_sec);
		return 0;
	}
	return 1;
}

/* Sends a ping command to the server and stores the result.
 *
 * Returns 1 when it worked, and 0 when an error was encountered. */
int mongo_connection_ping(mongo_con_manager *manager, mongo_connection *con, mongo_server_options *options, char **error_message)
{
	mcon_str      *packet;
	struct timeval start, end;
	char          *data_buffer;

	/* If we haven't hit the ping_interval yet, then there is no need to do a roundtrip to the server */
	if (!mongo_connection_ping_check(manager, con->last_ping, &start)) {
		return 1;
	}
	mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "is_ping: pinging %s", con->hash);
	packet = bson_create_ping_packet(con);
	if (!mongo_connect_send_packet(manager, con, options, packet, &data_buffer, error_message)) {
		return 0;
	}
	gettimeofday(&end, NULL);
	free(data_buffer);

	con->last_ping = end.tv_sec;
	con->ping_ms = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
	if (con->ping_ms < 0) { /* some clocks do weird stuff */
		con->ping_ms = 0;
	}

	mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "is_ping: last pinged at %ld; time: %dms", con->last_ping, con->ping_ms);

	return 1;
}

/* Sends an ismaster command to the server and returns an array of new
 * connectable nodes
 *
 * Returns:
 * 0: when an error occurred
 * 1: when is master was run and worked
 * 2: when is master wasn't run due to the time-out limit
 * 3: when it all worked, but we need to remove the seed host (due to its name
 *    not being what the server thought it is) - in that case, the server in
 *    the last argument is changed */
int mongo_connection_ismaster(mongo_con_manager *manager, mongo_connection *con, mongo_server_options *options, char **repl_set_name, int *nr_hosts, char ***found_hosts, char **error_message, mongo_server_def *server)
{
	mcon_str      *packet;
	char          *data_buffer;
	char          *set = NULL;      /* For replicaset in return */
	char          *hosts, *passives = NULL, *ptr, *string;
	char          *connected_name, *we_think_we_are;
	struct timeval now;
	int            retval = 1;

	gettimeofday(&now, NULL);
	if ((con->last_ismaster + manager->ismaster_interval) > now.tv_sec) {
		mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "ismaster: skipping: last ran at %ld, now: %ld, time left: %ld", con->last_ismaster, now.tv_sec, con->last_ismaster + manager->ismaster_interval - now.tv_sec);
		return 2;
	}

	mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "ismaster: start");
	packet = bson_create_ismaster_packet(con);

	if (!mongo_connect_send_packet(manager, con, options, packet, &data_buffer, error_message)) {
		return 0;
	}

	/* Find data fields */
	ptr = data_buffer + sizeof(int32_t); /* Skip the length */

	/* We find out whether the machine we connected too, is actually the
	 * one we thought we were connecting too */
	/* MongoDB 1.8.x doesn't have the "me" field.
	 * The replicaset verification is done next step (setName). */
	if (bson_find_field_as_string(ptr, "me", &connected_name)) {
		we_think_we_are = mongo_server_hash_to_server(con->hash);
		if (strcmp(connected_name, we_think_we_are) == 0) {
			mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "ismaster: the server name matches what we thought it'd be (%s).", we_think_we_are);
		} else {
			mongo_manager_log(manager, MLOG_CON, MLOG_WARN, "ismaster: the server name (%s) did not match with what we thought it'd be (%s).", connected_name, we_think_we_are);
			/* We reset the name as the server responded with a different name than
			 * what we thought it was */
			free(server->host);
			server->host = mcon_strndup(connected_name, strchr(connected_name, ':') - connected_name);
			server->port = atoi(strchr(connected_name, ':') + 1);
			retval = 3;
		}
		free(we_think_we_are);
	} else {
		mongo_manager_log(manager, MLOG_CON, MLOG_WARN, "Can't find 'me' in ismaster response, possibly not a replicaset (%s)", mongo_server_hash_to_server(con->hash));
	}

	/* Do replica set name test */
	bson_find_field_as_string(ptr, "setName", &set);
	if (!set) {
		char *errmsg = NULL;
		bson_find_field_as_string(ptr, "errmsg", &errmsg);
		if (errmsg) {
			*error_message = strdup(errmsg);
		} else {
			*error_message = strdup("Not a replicaset member");
		}
		free(data_buffer);
		return 0;
	} else if (*repl_set_name) {
		if (strcmp(set, *repl_set_name) != 0) {
			struct mcon_str *tmp;

			mcon_str_ptr_init(tmp);
			mcon_str_add(tmp, "Host does not match replicaset name. Expected: ", 0);
			mcon_str_add(tmp, *repl_set_name, 0);
			mcon_str_add(tmp, "; Found: ", 0);
			mcon_str_add(tmp, set, 0);

			*error_message = strdup(tmp->d);
			mcon_str_ptr_dtor(tmp);

			free(data_buffer);
			return 0;
		} else {
			mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "ismaster: the found replicaset name matches the expected one (%s).", set);
		}
	} else if (*repl_set_name == NULL) {
		/* This can happen, in case the replicaset name was not given, but just
		 * bool(true) (or string("1")) in the connection options. */
		mongo_manager_log(manager, MLOG_CON, MLOG_WARN, "ismaster: the replicaset name is not set, so we're using %s.", set);
		*repl_set_name = strdup(set);
	}

	/* If the server definition has not set the repl_set_name member yet, set it here */
	if (!server->repl_set_name) {
		server->repl_set_name = strdup(set);
	}

	/* Find all hosts */
	bson_find_field_as_array(ptr, "hosts", &hosts);
	bson_find_field_as_array(ptr, "passives", &passives);
	*nr_hosts = 0;

	/* Iterate over the "hosts" document */
	ptr = hosts;
	while (bson_array_find_next_string(&ptr, NULL, &string)) {
		(*nr_hosts)++;
		*found_hosts = (char**) realloc(*found_hosts, (*nr_hosts) * sizeof(char*));
		(*found_hosts)[*nr_hosts-1] = strdup(string);
		mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "found host: %s", string);
	}

	/* Iterate over the "passives" document (priority=0) */
	if (passives) {
		ptr = passives;
		while (bson_array_find_next_string(&ptr, NULL, &string)) {
			(*nr_hosts)++;
			*found_hosts = (char**) realloc(*found_hosts, (*nr_hosts) * sizeof(char*));
			(*found_hosts)[*nr_hosts-1] = strdup(string);
			mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "found host: %s (passive)", string);
		}
	}

	free(data_buffer);

	con->last_ismaster = now.tv_sec;
	mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "ismaster: last ran at %ld", con->last_ismaster);

	return retval;
}

/* Sends an ismaster command to the server to find server flags
 *
 * Returns 1 when it worked, and 0 when an error was encountered. */
int mongo_connection_get_server_flags(mongo_con_manager *manager, mongo_connection *con, mongo_server_options *options, char **error_message)
{
	mcon_str      *packet;
	int32_t        max_bson_size = 0, max_message_size = 0;
	char          *data_buffer;
	char          *ptr;
	char          *tags;
	char          *msg; /* If set and its value is "isdbgrid", it signals we connected to a mongos */
	char          *set = NULL;      /* check if it is a replicaset */
	unsigned char  ismaster = 0, secondary = 0, arbiter = 0;

	mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "get_server_flags: start");
	packet = bson_create_ismaster_packet(con);

	if (!mongo_connect_send_packet(manager, con, options, packet, &data_buffer, error_message)) {
		return 0;
	}

	/* Find data fields */
	ptr = data_buffer + sizeof(int32_t); /* Skip the length */

	/* Find max bson size */
	if (bson_find_field_as_int32(ptr, "maxBsonObjectSize", &max_bson_size)) {
		mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "get_server_flags: setting maxBsonObjectSize to %d", max_bson_size);
		con->max_bson_size = max_bson_size;
	} else {
		/* This seems to be a pre-1.8 MongoDB installation, where we need to
		 * default to 4MB */
		con->max_bson_size = 4194304;
		mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "get_server_flags: can't find maxBsonObjectSize, defaulting to %d", con->max_bson_size);
	}

	/* Find max message size */
	if (bson_find_field_as_int32(ptr, "maxMessageSizeBytes", &max_message_size)) {
		mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "get_server_flags: setting maxMessageSizeBytes to %d", max_message_size);
		con->max_message_size = max_message_size;
	} else {
		/* This seems to be a pre-2.4 MongoDB installation, where we need to
		 * default to two times the max BSON size */
		con->max_message_size = 2 * con->max_bson_size;
		mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "get_server_flags: can't find maxMessageSizeBytes, defaulting to %d", con->max_message_size);
	}

	/* Check for flags */
	bson_find_field_as_bool(ptr, "ismaster", &ismaster);
	bson_find_field_as_bool(ptr, "secondary", &secondary);
	bson_find_field_as_bool(ptr, "arbiterOnly", &arbiter);

	/* Set connection type depending on flags */
	if (ismaster) {
		/* Find msg and whether it contains "isdbgrid" */
		if (bson_find_field_as_string(ptr, "msg", (char**) &msg) && strcmp(msg, "isdbgrid") == 0) {
			mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "get_server_flags: msg contains 'isdbgrid' - we're connected to a mongos");
			con->connection_type = MONGO_NODE_MONGOS;
		} else if (bson_find_field_as_string(ptr, "setName", &set)) {
			mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "get_server_flags: got replicaset name: %s", set);
			con->connection_type = MONGO_NODE_PRIMARY;
		} else {
			con->connection_type = MONGO_NODE_STANDALONE;
		}
	} else if (secondary) {
		con->connection_type = MONGO_NODE_SECONDARY;
	} else if (arbiter) {
		con->connection_type = MONGO_NODE_ARBITER;
	} else {
		con->connection_type = MONGO_NODE_INVALID;
	}

	mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "get_server_flags: found server type: %s", mongo_connection_type(con->connection_type));

	/* Find read preferences tags */
	con->tag_count = 0;
	con->tags = NULL;
	if (bson_find_field_as_document(ptr, "tags", (char**) &tags)) {
		char *it, *name, *value;
		int   length;

		it = tags;

		while (bson_array_find_next_string(&it, &name, &value)) {
			con->tags = (char**) realloc(con->tags, (con->tag_count + 1) * sizeof(char*));
			length = strlen(name) + strlen(value) + 2;
			con->tags[con->tag_count] = (char*) malloc(length);
			snprintf(con->tags[con->tag_count], length, "%s:%s", name, value);
			free(name);
			mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "get_server_flags: added tag %s", con->tags[con->tag_count]);
			con->tag_count++;
		}
	}

	free(data_buffer);

	return 1;
}

/* Sends a getnonce command to the server for authentication
 *
 * Returns the nonsense when it worked, or NULL if it didn't. */
char *mongo_connection_getnonce(mongo_con_manager *manager, mongo_connection *con, mongo_server_options *options, char **error_message)
{
	mcon_str      *packet;
	char          *data_buffer;
	char          *ptr;
	char          *nonce;
	char          *retval = NULL;

	mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "getnonce: start");
	packet = bson_create_getnonce_packet(con);

	if (!mongo_connect_send_packet(manager, con, options, packet, &data_buffer, error_message)) {
		return NULL;
	}

	/* Find data fields */
	ptr = data_buffer + sizeof(int32_t); /* Skip the length */

	/* Find getnonce */
	if (bson_find_field_as_string(ptr, "nonce", &nonce)) {
		mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "getnonce: found nonce '%s'", nonce);
	} else {
		*error_message = strdup("Couldn't find the nonce field");
		free(data_buffer);
		return NULL;
	}

	retval = strdup(nonce);

	free(data_buffer);

	return retval;
}

/* Authenticates a connection
 *
 * Returns 1 when it worked, or 0 when it didn't - with the error_message set. */
int mongo_connection_authenticate(mongo_con_manager *manager, mongo_connection *con, mongo_server_options *options, char *database, char *username, char *password, char *nonce, char **error_message)
{
	mcon_str      *packet;
	char          *data_buffer, *errmsg;
	double         ok;
	char          *ptr;
	char          *salted;
	int            length;
	char          *hash, *key;

	mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "authenticate: start");

	/* Calculate hash=md5(${username}:mongo:${password}) */
	length = strlen(username) + 7 + strlen(password) + 1;
	salted = (char*) malloc(length);
	snprintf(salted, length, "%s:mongo:%s", username, password);
	hash = mongo_util_md5_hex(salted, length - 1); /* -1 to chop off \0 */
	free(salted);
	mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "authenticate: hash=md5(%s:mongo:%s) = %s", username, password, hash);

	/* Calculate key=md5(${nonce}${username}${hash}) */
	length = strlen(nonce) + strlen(username) + strlen(hash) + 1;
	salted = (char*) malloc(length);
	snprintf(salted, length, "%s%s%s", nonce, username, hash);
	key = mongo_util_md5_hex(salted, length - 1); /* -1 to chop off \0 */
	free(salted);
	mongo_manager_log(manager, MLOG_CON, MLOG_FINE, "authenticate: key=md5(%s%s%s) = %s", nonce, username, hash, key);

	packet = bson_create_authenticate_packet(con, database, username, nonce, key);

	free(hash);
	free(key);

	if (!mongo_connect_send_packet(manager, con, options, packet, &data_buffer, error_message)) {
		return 0;
	}

	/* Find data fields */
	ptr = data_buffer + sizeof(int32_t); /* Skip the length */

	/* Find errmsg */
	if (bson_find_field_as_double(ptr, "ok", &ok)) {
		if (ok > 0) {
			mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "authentication successful");
		} else {
			mongo_manager_log(manager, MLOG_CON, MLOG_WARN, "authentication failed");
		}
	}
	if (bson_find_field_as_string(ptr, "errmsg", &errmsg)) {
		*error_message = (char*) malloc(256);
		snprintf(*error_message, 256, "Authentication failed on database '%s' with username '%s': %s", database, username, errmsg);
		free(data_buffer);
		return 0;
	}

	free(data_buffer);

	return 1;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
