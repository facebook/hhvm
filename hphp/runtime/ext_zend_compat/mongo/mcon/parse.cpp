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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "parse.h"
#include "utils.h"
#include "manager.h"
#include "read_preference.h"

/* Forward declarations */
void static mongo_add_parsed_server_addr(mongo_con_manager *manager, mongo_servers *servers, char *host_start, char *host_end, char *port_start);
int static mongo_parse_options(mongo_con_manager *manager, mongo_servers *servers, char *options_string, char **error_message);

/* Parsing routine */
mongo_servers* mongo_parse_init(void)
{
	mongo_servers *servers;

	/* Create tmp server definitions */
	servers = (mongo_servers*) malloc(sizeof(mongo_servers));
	memset(servers, 0, sizeof(mongo_servers));
	servers->count = 0;
	servers->options.repl_set_name = NULL;
	servers->options.con_type = MONGO_CON_TYPE_STANDALONE;

	servers->options.connectTimeoutMS = 0;
	servers->options.socketTimeoutMS = -1;
	servers->options.default_w = -1;
	servers->options.default_wstring = NULL;
	servers->options.default_wtimeout = -1;
	servers->options.default_fsync = 0;
	servers->options.default_journal = 0;
	servers->options.ssl = MONGO_SSL_DISABLE;
	servers->options.ctx = NULL;

	return servers;
}

int mongo_parse_server_spec(mongo_con_manager *manager, mongo_servers *servers, char *spec, char **error_message)
{
	char          *pos; /* Pointer to current parsing position */
	char          *tmp_user = NULL, *tmp_pass = NULL, *tmp_database = NULL; /* Stores parsed user/password/database to be copied to each server struct */
	char          *host_start, *host_end, *port_start, *db_start, *db_end, *last_slash;
	int            i;

	/* Initialisation */
	pos = spec;
	mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "Parsing %s", spec);

	if (strstr(spec, "mongodb://") == spec) {
		char *at, *colon;

		/* mongodb://user:pass@host:port,host:port
		 *           ^                             */
		pos += 10;

		/* mongodb://user:pass@host:port,host:port
		 *                    ^                    */
		at = strchr(pos, '@');

		/* mongodb://user:pass@host:port,host:port
		 *               ^                         */
		colon = strchr(pos, ':');

		/* check for username:password */
		if (at && colon && at - colon > 0) {
			tmp_user = mcon_strndup(pos, colon - pos);
			tmp_pass = mcon_strndup(colon + 1, at - (colon + 1));

			/* move current
			 * mongodb://user:pass@host:port,host:port
			 *                     ^                   */
			pos = at + 1;
			mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found user '%s' and a password", tmp_user);
		}
	}

	host_start = pos;
	host_end   = NULL;
	port_start = NULL;
	last_slash = NULL;

	/* Now we parse the host part - there are two cases:
	 * 1: mongodb://user:pass@host:port,host:port/database?opt=1 -- TCP/IP
	 *                        ^
	 * 2: mongodb://user:pass@/tmp/mongo.sock/database?opt=1 -- Unix Domain sockets
	 *                        ^                                                     */
	if (*pos != '/') {
		/* TCP/IP:
		 * mongodb://user:pass@host:port,host:port/database?opt=1 -- TCP/IP
		 *                     ^                                            */
		do {
			if (*pos == ':') {
				host_end = pos;
				port_start = pos + 1;
			}
			if (*pos == ',') {
				if (!host_end) {
					host_end = pos;
				}

				mongo_add_parsed_server_addr(manager, servers, host_start, host_end, port_start);

				host_start = pos + 1;
				host_end = port_start = NULL;
			}
			if (*pos == '/') {
				if (!host_end) {
					host_end = pos;
				}
				break;
			}
			pos++;
		} while (*pos != '\0');

		/* We are now either at the end of the string, or at / where the dbname
		 * starts.  We still have to add the last parser host/port combination
		 * though: */
		mongo_add_parsed_server_addr(manager, servers, host_start, host_end ? host_end : pos, port_start);
	} else if (*pos == '/') {
		host_start = pos;
		port_start = "0";

		/* Unix Domain Socket
		 * mongodb://user:pass@/tmp/mongo.sock
		 * mongodb://user:pass@/tmp/mongo.sock/?opt=1
		 * mongodb://user:pass@/tmp/mongo.sock/database?opt=1
		 */
		last_slash = strrchr(pos, '/');

		/* The last component of the path *could* be a database name.  The rule
		 * is; if the last component has a dot, we use the full string since
		 * "host_start" as host */
		if (strchr(last_slash, '.')) {
			host_end = host_start + strlen(host_start);
		} else {
			host_end = last_slash;
		}
		pos = host_end;
		mongo_add_parsed_server_addr(manager, servers, host_start, host_end, port_start);
	}

	/* Set the default connection type, we might change this if we encounter
	 * the replicaSet option later */
	if (servers->count == 1) {
		servers->options.con_type = MONGO_CON_TYPE_STANDALONE;
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Connection type: STANDALONE");
	} else {
		servers->options.con_type = MONGO_CON_TYPE_MULTIPLE;
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Connection type: MULTIPLE");
	}

	/* Check for dbname
	 * mongodb://user:pass@host:port,host:port/dbname?foo=bar
	 *                                        ^ */
	db_start = NULL;
	db_end = spec + strlen(spec);
	if (*pos == '/') {
		char *question;

		question = strchr(pos, '?');
		if (question) {
			if (pos + 1 == question) {
				db_start = NULL;
			} else {
				db_start = pos + 1;
				db_end = question;
			}
		} else {
			db_start = pos + 1;
			db_end = spec + strlen(spec);
		}

		/* Check for options
		 * mongodb://user:pass@host:port,host:port/dbname?foo=bar
		 *                                               ^ */
		if (question) {
			int retval = -1;
			retval = mongo_parse_options(manager, servers, question + 1, error_message);
			if (retval > 0) {
				free(tmp_user);
				free(tmp_pass);
				free(tmp_database);

				return retval;
			}
		}
	}

	/* Handling database name */
	if (db_start && (db_end != db_start)) {
		tmp_database = mcon_strndup(db_start, db_end - db_start);
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found database name '%s'", tmp_database);
	} else if (tmp_user && tmp_pass) {
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- No database name found for an authenticated connection. Using 'admin' as default database");
		tmp_database = strdup("admin");
	}

	/* Update all servers with user, password and dbname */
	for (i = 0; i < servers->count; i++) {
		servers->server[i]->username = tmp_user ? strdup(tmp_user) : NULL;
		servers->server[i]->password = tmp_pass ? strdup(tmp_pass) : NULL;
		servers->server[i]->db       = tmp_database ? strdup(tmp_database) : NULL;
	}

	free(tmp_user);
	free(tmp_pass);
	free(tmp_database);

	return 0;
}

/* Helpers */
void static mongo_add_parsed_server_addr(mongo_con_manager *manager, mongo_servers *servers, char *host_start, char *host_end, char *port_start)
{
	mongo_server_def *tmp;

	tmp = (mongo_server_def*) malloc(sizeof(mongo_server_def));
	memset(tmp, 0, sizeof(mongo_server_def));
	tmp->username = tmp->password = tmp->db = tmp->authdb = NULL;
	tmp->mechanism = MONGO_AUTH_MECHANISM_MONGODB_CR; /* MONGODB-CR is the default authentication mechanism */
	tmp->port = 27017;

	tmp->host = mcon_strndup(host_start, host_end - host_start);
	if (port_start) {
		tmp->port = atoi(port_start);
	}
	servers->server[servers->count] = tmp;
	servers->count++;
	mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found node: %s:%d", tmp->host, tmp->port);
}

/* Processes a single option/value pair.
 * Returns:
 * -1 if it worked, but the option really shouldn't be used
 * 0 if it worked
 * 1 if either name or value was missing
 * 2 if the option didn't exist
 * 3 on logic errors */
int static mongo_process_option(mongo_con_manager *manager, mongo_servers *servers, char *name, char *value, char *pos, char **error_message)
{
	char *tmp_name;
	char *tmp_value;
	int   retval = 0;

	if (!name || strcmp(name, "") == 0 || (name + 1 == value)) {
		*error_message = strdup("- Found an empty option name");
		mongo_manager_log(manager, MLOG_PARSE, MLOG_WARN, "- Found an empty option name");
		return 1;
	}
	if (!value) {
		*error_message = strdup("- Found an empty option value");
		mongo_manager_log(manager, MLOG_PARSE, MLOG_WARN, "- Found an empty option value");
		return 1;
	}

	tmp_name = mcon_strndup(name, value - name - 1);
	tmp_value = mcon_strndup(value, pos - value);

	retval = mongo_store_option(manager, servers, tmp_name, tmp_value, error_message);

	free(tmp_name);
	free(tmp_value);

	return retval;
}

/* Option parser helpers */
static int parse_read_preference_tags(mongo_con_manager *manager, mongo_servers *servers, char *value, char **error_message)
{
	mongo_read_preference_tagset *tmp_ts = (mongo_read_preference_tagset*) calloc(1, sizeof(mongo_read_preference_tagset));

	/* format = dc:ny,rack:1 - empty is allowed! */
	if (strlen(value) == 0) {
		mongo_read_preference_add_tagset(&servers->read_pref, tmp_ts);
	} else {
		char *start, *end, *colon, *tmp_name, *tmp_value;

		start = value;

		while (1) {
			end = strchr(start, ',');
			colon = strchr(start, ':');
			if (!colon) {
				int len = strlen(start) + sizeof("Error while trying to parse tags: No separator for ''");

				*error_message = (char*) malloc(len + 1);
				snprintf(*error_message, len, "Error while trying to parse tags: No separator for '%s'", start);
				mongo_read_preference_tagset_dtor(tmp_ts);
				return 3;
			}
			tmp_name = mcon_strndup(start, colon - start);
			if (end) {
				tmp_value = mcon_strndup(colon + 1, end - colon - 1);
				start = end + 1;
				mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found tag '%s': '%s'", tmp_name, tmp_value);
				mongo_read_preference_add_tag(tmp_ts, tmp_name, tmp_value);
				free(tmp_value);
				free(tmp_name);
			} else {
				mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found tag '%s': '%s'", tmp_name, colon + 1);
				mongo_read_preference_add_tag(tmp_ts, tmp_name, colon + 1);
				free(tmp_name);
				break;
			}
		}
		mongo_read_preference_add_tagset(&servers->read_pref, tmp_ts);
	}
	return 0;
}

/* Sets server options.
 * Returns:
 * 0 if it worked
 * 2 if the option didn't exist
 * 3 on logical errors.
 *
 * On logical errors, the error_message will be populated with the reason. */
int mongo_store_option(mongo_con_manager *manager, mongo_servers *servers, char *option_name, char *option_value, char **error_message)
{
	int i;

	if (strcasecmp(option_name, "authMechanism") == 0) {
		int mechanism;

		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'authMechanism': '%s'", option_value);
		if (strcasecmp(option_value, "MONGODB-CR") == 0) {
			mechanism = MONGO_AUTH_MECHANISM_MONGODB_CR;
		} else if (strcasecmp(option_value, "GSSAPI") == 0) {
			/* FIXME: GSSAPI isn't implemented yet */
			mechanism = MONGO_AUTH_MECHANISM_GSSAPI;
			*error_message = strdup("The authMechanism 'GSSAPI' is currently not supported. Only MONGODB-CR is available.");
			return 3;
		} else {
			int len = strlen(option_value) + sizeof("The authMechanism '' does not exist.");

			*error_message = (char*) malloc(len + 1);
			snprintf(*error_message, len, "The authMechanism '%s' does not exist.", option_value);
			return 3;
		}
		for (i = 0; i < servers->count; i++) {
			servers->server[i]->mechanism = mechanism;
		}
		return 0;
	}

	if (strcasecmp(option_name, "authSource") == 0) {
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'authSource': '%s'", option_value);
		for (i = 0; i < servers->count; i++) {
			if (servers->server[i]->authdb) {
				free(servers->server[i]->authdb);
			}
			servers->server[i]->authdb = strdup(option_value);
		}
		return 0;
	}

	if (strcasecmp(option_name, "connectTimeoutMS") == 0) {
		int value = atoi(option_value);

		if (servers->options.connectTimeoutMS) {
			mongo_manager_log(manager, MLOG_PARSE, MLOG_WARN, "- Replacing previously set value for 'connectTimeoutMS' (%d)", servers->options.connectTimeoutMS);
		}
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'connectTimeoutMS': %d", value);
		servers->options.connectTimeoutMS = value;
		return 0;
	}

	if (strcasecmp(option_name, "db") == 0) {
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'db': '%s'", option_value);
		for (i = 0; i < servers->count; i++) {
			if (servers->server[i]->db) {
				free(servers->server[i]->db);
				/* Free the authdb too as it defaulted to 'admin' when no db was passed as the connection string */
				free(servers->server[i]->authdb);
				servers->server[i]->authdb = NULL;
			}
			servers->server[i]->db = strdup(option_value);
		}
		return 0;
	}

	if (strcasecmp(option_name, "fsync") == 0) {
		if (strcasecmp(option_value, "true") == 0 || strcmp(option_value, "1") == 0) {
			servers->options.default_fsync = 1;
		} else {
			servers->options.default_fsync = 0;
		}
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'fsync': %d", servers->options.default_fsync);
		return 0;
	}

	if (strcasecmp(option_name, "journal") == 0) {
		if (strcasecmp(option_value, "true") == 0 || strcmp(option_value, "1") == 0) {
			servers->options.default_journal = 1;
		} else {
			servers->options.default_journal = 0;
		}
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'journal': %d", servers->options.default_journal);
		return 0;
	}

	if (strcasecmp(option_name, "password") == 0) {
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'password': '%s'", option_value);
		for (i = 0; i < servers->count; i++) {
			if (servers->server[i]->password) {
				free(servers->server[i]->password);
			}
			servers->server[i]->password = strdup(option_value);
		}
		return 0;
	}

	if (strcasecmp(option_name, "readPreference") == 0) {
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'readPreference': '%s'", option_value);
		if (strcasecmp(option_value, "primary") == 0) {
			servers->read_pref.type = MONGO_RP_PRIMARY;
		} else if (strcasecmp(option_value, "primaryPreferred") == 0) {
			servers->read_pref.type = MONGO_RP_PRIMARY_PREFERRED;
		} else if (strcasecmp(option_value, "secondary") == 0) {
			servers->read_pref.type = MONGO_RP_SECONDARY;
		} else if (strcasecmp(option_value, "secondaryPreferred") == 0) {
			servers->read_pref.type = MONGO_RP_SECONDARY_PREFERRED;
		} else if (strcasecmp(option_value, "nearest") == 0) {
			servers->read_pref.type = MONGO_RP_NEAREST;
		} else {
			int len = strlen(option_value) + sizeof("The readPreference value '' is not supported.");

			*error_message = (char*) malloc(len + 1);
			snprintf(*error_message, len, "The readPreference value '%s' is not supported.", option_value);
			return 3;
		}
		return 0;
	}

	if (strcasecmp(option_name, "readPreferenceTags") == 0) {
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'readPreferenceTags': '%s'", option_value);
		return parse_read_preference_tags(manager, servers, option_value, error_message);
	}

	if (strcasecmp(option_name, "replicaSet") == 0) {
		if (servers->options.repl_set_name) {
			/* Free the already existing one */
			free(servers->options.repl_set_name);
			servers->options.repl_set_name = NULL; /* We reset it as not all options set a string as replset name */
		}

		if (option_value && *option_value) {
			/* We explicitly check for the stringified version of "true" here,
			 * as "true" has a special meaning. It does not mean that the
			 * replicaSet name is "1". */
			if (strcmp(option_value, "1") != 0) {
				servers->options.repl_set_name = strdup(option_value);
				mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'replicaSet': '%s'", option_value);

				/* Associate the given replica set name with all the server
				 * definitions from the seed */
				for (i = 0; i < servers->count; i++) {
					if (servers->server[i]->repl_set_name) {
						free(servers->server[i]->repl_set_name);
					}
					servers->server[i]->repl_set_name = strdup(option_value);
				}
			} else {
				mongo_manager_log(manager, MLOG_PARSE, MLOG_WARN, "- Found option 'replicaSet': true - Expected the name of the replica set");
			}
			servers->options.con_type = MONGO_CON_TYPE_REPLSET;
			mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Switching connection type: REPLSET");
		}
		return 0;
	}

	if (strcasecmp(option_name, "slaveOkay") == 0) {
		if (strcasecmp(option_value, "true") == 0 || strcmp(option_value, "1") == 0) {
			mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'slaveOkay': true");
			if (servers->read_pref.type != MONGO_RP_PRIMARY || servers->read_pref.tagset_count) {
				/* The server already has read preferences configured, but
				 * we're still trying to set slave okay. The spec says that's
				 * an error */
				*error_message = strdup("You can not use both slaveOkay and read-preferences. Please switch to read-preferences.");
				return 3;
			} else {
				/* Old style option, that needs to be removed. For now, spec
				 * dictates it needs to be ReadPreference=SECONDARY_PREFERRED */
				servers->read_pref.type = MONGO_RP_SECONDARY_PREFERRED;
			}
			return -1;
		}

		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'slaveOkay': false");
		return -1;
	}

	if (strcasecmp(option_name, "socketTimeoutMS") == 0) {
		int value = atoi(option_value);

		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'socketTimeoutMS': %d", value);
		servers->options.socketTimeoutMS = value;
		return 0;
	}

	if (strcasecmp(option_name, "ssl") == 0) {
		int value = 0;
		if (strcasecmp(option_value, "true") == 0 || strcmp(option_value, "1") == 0) {
			value = MONGO_SSL_ENABLE;
			mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'ssl': true");
		} else if (strcasecmp(option_value, "false") == 0 || strcmp(option_value, "0") == 0) {
			value = MONGO_SSL_DISABLE;
			mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'ssl': false");
		} else if (strcasecmp(option_value, "prefer") == 0 || atoi(option_value) == MONGO_SSL_PREFER) {
			/* FIXME: MongoDB doesn't support "connection promotion" to SSL at
			 * the moment, so we can't support this option properly */
			value = MONGO_SSL_PREFER;
			mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'ssl': prefer");
			*error_message = strdup("SSL=prefer is currently not supported by mongod");
			return 3;
		} else {
			mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'ssl': '%s'", option_name);
			*error_message = strdup("SSL can only be 'true' or 'false'");
			return 3;
		}

		servers->options.ssl = value;
		return 0;
	}

	if (strcasecmp(option_name, "timeout") == 0) {
		int value = atoi(option_value);

		if (servers->options.connectTimeoutMS) {
			mongo_manager_log(manager, MLOG_PARSE, MLOG_WARN, "- Replacing previously set value for 'connectTimeoutMS' (%d)", servers->options.connectTimeoutMS);
		}
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'timeout' ('connectTimeoutMS'): %d", value);
		servers->options.connectTimeoutMS = value;
		return -1;
	}

	if (strcasecmp(option_name, "username") == 0) {
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'username': '%s'", option_value);
		for (i = 0; i < servers->count; i++) {
			if (servers->server[i]->username) {
				free(servers->server[i]->username);
			}
			servers->server[i]->username = strdup(option_value);
			/* Use "admin" as the default db if none selected yet. It is okay
			 * if it is set in a later option, as we first always free the
			 * value before setting it anyway. */
			if (!servers->server[i]->db) {
				servers->server[i]->db = strdup("admin");
				/* Admin users always authenticate on the admin db, even when
				 * using other databases */
				servers->server[i]->authdb = strdup("admin");
			}
		}
		return 0;
	}

	if (strcasecmp(option_name, "w") == 0) {
		/* Rough check to see whether this is a numeric string or not */
		char *endptr;
		long tmp_value;
		
		tmp_value = strtol(option_value, &endptr, 10);
		/* If no invalid character is found (endptr == 0), we consider the
		 * option value as a number */
		if (!*endptr) {
			servers->options.default_w = tmp_value;
			mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'w': %d", servers->options.default_w);
			if (servers->options.default_w < 0) {
				*error_message = strdup("The value of 'w' needs to be 0 or higher (or a string).");
				return 3;
			}
		} else {
			servers->options.default_w = 1;
			servers->options.default_wstring = strdup(option_value);
			mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'w': '%s'", servers->options.default_wstring);
		}
		return 0;
	}

	if (strcasecmp(option_name, "wTimeout") == 0) {
		int value = atoi(option_value);

		if (servers->options.default_wtimeout != -1) {
			mongo_manager_log(manager, MLOG_PARSE, MLOG_WARN, "- Replacing previously set value for 'wTimeoutMS' (%d)", servers->options.default_wtimeout);
		}
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'wTimeout' ('wTimeoutMS'): %d", value);
		servers->options.default_wtimeout = value;
		return 0;
	}

	if (strcasecmp(option_name, "wTimeoutMS") == 0) {
		int value = atoi(option_value);

		if (servers->options.default_wtimeout != -1) {
			mongo_manager_log(manager, MLOG_PARSE, MLOG_WARN, "- Replacing previously set value for 'wTimeoutMS' (%d)", servers->options.default_wtimeout);
		}
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- Found option 'wTimeoutMS': %d", value);
		servers->options.default_wtimeout = value;
		return 0;
	}

	*error_message = (char*) malloc(256);
	snprintf(*error_message, 256, "- Found unknown connection string option '%s' with value '%s'", option_name, option_value);
	mongo_manager_log(manager, MLOG_PARSE, MLOG_WARN, "- Found unknown connection string option '%s' with value '%s'", option_name, option_value);
	return 2;
}


/* Returns 0 if all options were processed without errors.
 * On failure, returns 1 and populates error_message */
int static mongo_parse_options(mongo_con_manager *manager, mongo_servers *servers, char *options_string, char **error_message)
{
	int retval = 0;
	char *name_start, *value_start = NULL, *pos;

	name_start = pos = options_string;

	do {
		if (*pos == '=') {
			value_start = pos + 1;
		}
		if (*pos == ';' || *pos == '&') {
			retval = mongo_process_option(manager, servers, name_start, value_start, pos, error_message);

			if (retval > 0) {
				return retval;
			}
			name_start = pos + 1;
			value_start = NULL;
		}
		pos++;
	} while (*pos != '\0');
	retval = mongo_process_option(manager, servers, name_start, value_start, pos, error_message);

	return retval;
}

void static mongo_server_def_dump(mongo_con_manager *manager, mongo_server_def *server_def)
{
	mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO,
		"- host: %s; port: %d; username: %s, password: %s, database: %s, auth source: %s, mechanism: %d",
		server_def->host, server_def->port, server_def->username, server_def->password, server_def->db, server_def->authdb, server_def->mechanism);
}

void mongo_servers_dump(mongo_con_manager *manager, mongo_servers *servers)
{
	int i;

	mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "Seeds:");
	for (i = 0; i < servers->count; i++) {
		mongo_server_def_dump(manager, servers->server[i]);
	}
	mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "");

	mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "Options:");
	mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- repl_set_name: %s", servers->options.repl_set_name);
	mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- readPreference: %s", mongo_read_preference_type_to_name(servers->read_pref.type));
	for (i = 0; i < servers->read_pref.tagset_count; i++) {
		char *tmp = mongo_read_preference_squash_tagset(servers->read_pref.tagsets[i]);
		mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "- tagset: %s", tmp);
		free(tmp);
	}
	mongo_manager_log(manager, MLOG_PARSE, MLOG_INFO, "\n");
}

/* Cloning */
static void mongo_server_def_copy(mongo_server_def *to, mongo_server_def *from, int flags)
{
	to->host = to->repl_set_name = to->db = to->authdb = to->username = to->password = NULL;
	to->mechanism = MONGO_AUTH_MECHANISM_MONGODB_CR;
	if (from->host) {
		to->host = strdup(from->host);
	}
	to->port = from->port;
	if (from->repl_set_name) {
		to->repl_set_name = strdup(from->repl_set_name);
	}

	if (flags & MONGO_SERVER_COPY_CREDENTIALS) {
		if (from->db) {
			to->db = strdup(from->db);
		}
		if (from->authdb) {
			to->authdb = strdup(from->authdb);
		}
		if (from->username) {
			to->username = strdup(from->username);
		}
		if (from->password) {
			to->password = strdup(from->password);
		}
		to->mechanism = from->mechanism;
	}
}

void mongo_servers_copy(mongo_servers *to, mongo_servers *from, int flags)
{
	int i;

	to->count = from->count;
	for (i = 0; i < from->count; i++) {
		to->server[i] = (mongo_server_def*) calloc(1, sizeof(mongo_server_def));
		mongo_server_def_copy(to->server[i], from->server[i], flags);
	}

	to->options.con_type = from->options.con_type;

	if (from->options.repl_set_name) {
		to->options.repl_set_name = strdup(from->options.repl_set_name);
	}

	to->options.connectTimeoutMS = from->options.connectTimeoutMS;

	to->options.default_w = from->options.default_w;
	to->options.default_wtimeout = from->options.default_wtimeout;
	if (from->options.default_wstring) {
		to->options.default_wstring = strdup(from->options.default_wstring);
	}
	to->options.default_fsync = from->options.default_fsync;
	to->options.default_journal = from->options.default_journal;

	to->options.ssl = from->options.ssl;

	if (from->options.ctx) {
		memcpy(to->options.ctx, from->options.ctx, sizeof(void *));
	}

	mongo_read_preference_copy(&from->read_pref, &to->read_pref);
}

/* Cleanup */
void mongo_server_def_dtor(mongo_server_def *server_def)
{
	if (server_def->host) {
		free(server_def->host);
	}
	if (server_def->repl_set_name) {
		free(server_def->repl_set_name);
	}
	if (server_def->db) {
		free(server_def->db);
	}
	if (server_def->authdb) {
		free(server_def->authdb);
	}
	if (server_def->username) {
		free(server_def->username);
	}
	if (server_def->password) {
		free(server_def->password);
	}
	free(server_def);
}

void mongo_servers_dtor(mongo_servers *servers)
{
	int i;

	for (i = 0; i < servers->count; i++) {
		mongo_server_def_dtor(servers->server[i]);
	}
	if (servers->options.repl_set_name) {
		free(servers->options.repl_set_name);
	}
	if (servers->options.default_wstring) {
		free(servers->options.default_wstring);
	}
	for (i = 0; i < servers->read_pref.tagset_count; i++) {
		mongo_read_preference_tagset_dtor(servers->read_pref.tagsets[i]);
	}
	if (servers->read_pref.tagsets) {
		free(servers->read_pref.tagsets);
	}
	free(servers);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
