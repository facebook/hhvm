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
#include <string.h>
#include <errno.h>

#ifndef WIN32
# include <sys/types.h>
#endif

#include <php.h>
#include <zend_exceptions.h>
#include "ext/standard/php_smart_str.h"
#include "ext/standard/file.h"

#include "php_mongo.h"
#include "mongoclient.h"
#include "db.h"
#include "cursor.h"
#include "bson.h"

#include "util/log.h"
#include "util/pool.h"

#include "mcon/types.h"
#include "mcon/read_preference.h"
#include "mcon/parse.h"
#include "mcon/manager.h"
#include "mcon/utils.h"


static void php_mongoclient_free(void* TSRMLS_DC);
static void stringify_server(mongo_server_def *server, smart_str *str);
static int close_connection(mongo_con_manager *manager, mongo_connection *connection);

zend_object_handlers mongo_default_handlers;
zend_object_handlers mongoclient_handlers;

ZEND_EXTERN_MODULE_GLOBALS(mongo)

zend_class_entry *mongo_ce_MongoClient;

extern zend_class_entry *mongo_ce_DB, *mongo_ce_Cursor, *mongo_ce_Exception;
extern zend_class_entry *mongo_ce_ConnectionException;

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo___construct, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, server)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo___get, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_no_parameters, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_selectDB, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, database_name)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_selectCollection, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, database_name)
	ZEND_ARG_INFO(0, collection_name)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_setReadPreference, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, read_preference)
	ZEND_ARG_ARRAY_INFO(0, tags, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_dropDB, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, MongoDB_object_OR_database_name)
ZEND_END_ARG_INFO()

static zend_function_entry mongo_methods[] = {
	PHP_ME(MongoClient, __construct, arginfo___construct, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, getConnections, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(MongoClient, connect, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, __toString, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, selectDB, arginfo_selectDB, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, selectCollection, arginfo_selectCollection, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, getReadPreference, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, setReadPreference, arginfo_setReadPreference, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, dropDB, arginfo_dropDB, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, listDBs, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, getHosts, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoClient, close, arginfo_no_parameters, ZEND_ACC_PUBLIC)

	{ NULL, NULL, NULL }
};

/* {{{ php_mongoclient_free
 */
static void php_mongoclient_free(void *object TSRMLS_DC)
{
	mongoclient *link = (mongoclient*)object;

	/* already freed */
	if (!link) {
		return;
	}

	if (link->servers) {
		mongo_servers_dtor(link->servers);
	}

	zend_object_std_dtor(&link->std TSRMLS_CC);

	efree(link);
}
/* }}} */

#if PHP_VERSION_ID >= 50400
zval *mongo_read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC)
#else
zval *mongo_read_property(zval *object, zval *member, int type TSRMLS_DC)
#endif
{
	zval *retval;
	zval tmp_member;
	mongoclient *obj;

	if (Z_TYPE_P(member) != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	obj = (mongoclient *)zend_objects_get_address(object TSRMLS_CC);
	if (strcmp(Z_STRVAL_P(member), "connected") == 0) {
		char *error_message = NULL;
		mongo_connection *conn = mongo_get_read_write_connection(obj->manager, obj->servers, MONGO_CON_FLAG_READ|MONGO_CON_FLAG_DONT_CONNECT, (char**) &error_message);
		ALLOC_INIT_ZVAL(retval);
		Z_SET_REFCOUNT_P(retval, 0);
		ZVAL_BOOL(retval, conn ? 1 : 0);
		if (error_message) {
			free(error_message);
		}
		return retval;
	}

#if PHP_VERSION_ID >= 50400
	retval = (zend_get_std_object_handlers())->read_property(object, member, type, key TSRMLS_CC);
#else
	retval = (zend_get_std_object_handlers())->read_property(object, member, type TSRMLS_CC);
#endif
	if (member == &tmp_member) {
		zval_dtor(member);
	}
	return retval;
}

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
HashTable *mongo_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
	HashPosition pos;
	HashTable *props = zend_std_get_properties(object TSRMLS_CC);
	zval **entry;
	ulong num_key;

	zend_hash_internal_pointer_reset_ex(props, &pos);
	while (zend_hash_get_current_data_ex(props, (void **)&entry, &pos) == SUCCESS) {
		char *key;
		uint key_len;

		switch (zend_hash_get_current_key_ex(props, &key, &key_len, &num_key, 0, &pos)) {
			case HASH_KEY_IS_STRING: {
				/* Override the connected property like we do for the read_property handler */
				if (strcmp(key, "connected") == 0) {
					zval member;
					zval *tmp;
					INIT_ZVAL(member);
					ZVAL_STRINGL(&member, key, key_len, 0);

#if PHP_VERSION_ID >= 50400
					tmp = mongo_read_property(object, &member, BP_VAR_IS, NULL TSRMLS_CC);
#else
					tmp = mongo_read_property(object, &member, BP_VAR_IS TSRMLS_CC);
#endif
					convert_to_boolean_ex(entry);
					ZVAL_BOOL(*entry, Z_BVAL_P(tmp));
					/* the var is set to refcount = 0, need to set it to 1 so it'll get free()d */
					if (Z_REFCOUNT_P(tmp) == 0) {
						Z_SET_REFCOUNT_P(tmp, 1);
					}
					zval_ptr_dtor(&tmp);
				}
				break;
			}
			case HASH_KEY_IS_LONG:
			case HASH_KEY_NON_EXISTANT:
				break;
		}
		zend_hash_move_forward_ex(props, &pos);
	}

	*is_temp = 0;
	return props;
}
#endif


/* {{{ php_mongoclient_new
 */
zend_object_value php_mongoclient_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	mongoclient *intern;

	intern = (mongoclient*)emalloc(sizeof(mongoclient));
	memset(intern, 0, sizeof(mongoclient));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	init_properties(intern);

	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t) zend_objects_destroy_object, php_mongoclient_free, NULL TSRMLS_CC);
	retval.handlers = &mongoclient_handlers;

	return retval;
}
/* }}} */

void mongo_init_MongoClient(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoClient", mongo_methods);
	ce.create_object = php_mongoclient_new;
	mongo_ce_MongoClient = zend_register_internal_class(&ce TSRMLS_CC);

	/* make mongoclient object uncloneable, and with its own read_property */
	memcpy(&mongoclient_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	mongoclient_handlers.clone_obj = NULL;
	mongoclient_handlers.read_property = mongo_read_property;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	mongoclient_handlers.get_debug_info = mongo_get_debug_info;
#endif

	/* Mongo class constants */
	zend_declare_class_constant_string(mongo_ce_MongoClient, "DEFAULT_HOST", strlen("DEFAULT_HOST"), "localhost" TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_MongoClient, "DEFAULT_PORT", strlen("DEFAULT_PORT"), 27017 TSRMLS_CC);
	zend_declare_class_constant_string(mongo_ce_MongoClient, "VERSION", strlen("VERSION"), PHP_MONGO_VERSION TSRMLS_CC);

	/* Read preferences types */
	zend_declare_class_constant_string(mongo_ce_MongoClient, "RP_PRIMARY", strlen("RP_PRIMARY"), "primary" TSRMLS_CC);
	zend_declare_class_constant_string(mongo_ce_MongoClient, "RP_PRIMARY_PREFERRED", strlen("RP_PRIMARY_PREFERRED"), "primaryPreferred" TSRMLS_CC);
	zend_declare_class_constant_string(mongo_ce_MongoClient, "RP_SECONDARY", strlen("RP_SECONDARY"), "secondary" TSRMLS_CC);
	zend_declare_class_constant_string(mongo_ce_MongoClient, "RP_SECONDARY_PREFERRED", strlen("RP_SECONDARY_PREFERRED"), "secondaryPreferred" TSRMLS_CC);
	zend_declare_class_constant_string(mongo_ce_MongoClient, "RP_NEAREST", strlen("RP_NEAREST"), "nearest" TSRMLS_CC);

	/* Mongo fields */
	zend_declare_property_bool(mongo_ce_MongoClient, "connected", strlen("connected"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(mongo_ce_MongoClient, "status", strlen("status"), ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(mongo_ce_MongoClient, "server", strlen("server"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(mongo_ce_MongoClient, "persistent", strlen("persistent"), ZEND_ACC_PROTECTED TSRMLS_CC);
}

/* {{{ Helper for connecting the servers */
mongo_connection *php_mongo_connect(mongoclient *link, int flags TSRMLS_DC)
{
	mongo_connection *con;
	char *error_message = NULL;

	/* We don't care about the result so although we assign it to a var, we
	 * only do that to handle errors and return it so that the calling function
	 * knows whether a connection could be obtained or not. */
	con = mongo_get_read_write_connection(link->manager, link->servers, flags, (char **) &error_message);
	if (con) {
		return con;
	}

	if (error_message) {
		zend_throw_exception(mongo_ce_ConnectionException, error_message, 71 TSRMLS_CC);
		free(error_message);
	} else {
		zend_throw_exception(mongo_ce_ConnectionException, "Unknown error obtaining connection", 72 TSRMLS_CC);
	}
	return NULL;
}
/* }}} */

/* {{{ Helper for special options, that can't be represented by a simple key
 * value pair, or options that are not actually connection string options. */
int mongo_store_option_wrapper(mongo_con_manager *manager, mongo_servers *servers, char *option_name, zval **option_value, char **error_message)
{
	/* Special cases:
	 *  - "connect" isn't supported by the URL parsing
	 *  - "readPreferenceTags" is an array of tagsets we need to iterate over
	 */
	if (strcasecmp(option_name, "connect") == 0) {
		return 4;
	}
	if (strcasecmp(option_name, "readPreferenceTags") == 0) {
		int            error = 0;
		HashPosition   pos;
		zval         **opt_entry;

		convert_to_array_ex(option_value);
		for (
			zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(option_value), &pos);
			zend_hash_get_current_data_ex(Z_ARRVAL_PP(option_value), (void **) &opt_entry, &pos) == SUCCESS;
			zend_hash_move_forward_ex(Z_ARRVAL_PP(option_value), &pos)
		) {
			convert_to_string_ex(opt_entry);
			error = mongo_store_option(manager, servers, option_name, Z_STRVAL_PP(opt_entry), error_message);
			if (error) {
				return error;
			}
		}
		return error;
	}
	convert_to_string_ex(option_value);

	return mongo_store_option(manager, servers, option_name, Z_STRVAL_PP(option_value), error_message);
}
/* }}} */

/* {{{ proto MongoClient MongoClient->__construct([string connection_string [, array mongo_options [, array driver_options]]])
   Creates a new MongoClient object for mongo[d|s]. mongo_options are the same
   options as the connection_string, while driver_options is additional PHP
   MongoDB options, like stream context and callbacks. */
PHP_METHOD(MongoClient, __construct)
{
	php_mongo_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}

void php_mongo_ctor(INTERNAL_FUNCTION_PARAMETERS, int bc)
{
	char         *server = 0;
	int           server_len = 0;
	zend_bool     connect = 1;
	zval         *options = 0;
	zval         *slave_okay = 0;
	zval         *zdoptions = NULL;
	mongoclient  *link;
	zval        **opt_entry;
	char         *opt_key;
	int           error;
	char         *error_message = NULL;
	uint          opt_key_len;
	ulong         num_key;
	HashPosition  pos;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s!a!/a!/", &server, &server_len, &options, &zdoptions) == FAILURE) {
		zval *object = getThis();
		ZVAL_NULL(object);
		return;
	}

	link = (mongoclient*)zend_object_store_get_object(getThis() TSRMLS_CC);

	/* Set the manager from the global manager */
	link->manager = MonGlo(manager);
	
	/* Parse the server specification
	 * Default to the mongo.default_host & mongo.default_port INI options */
	link->servers = mongo_parse_init();
	if (server_len) {
		error = mongo_parse_server_spec(link->manager, link->servers, server, (char **)&error_message);
		if (error) {
			zend_throw_exception(mongo_ce_ConnectionException, error_message, 20 + error TSRMLS_CC);
			free(error_message);
			return;
		}
	} else {
		char *tmp;

		spprintf(&tmp, 0, "%s:%ld", MonGlo(default_host), MonGlo(default_port));
		error = mongo_parse_server_spec(link->manager, link->servers, tmp, (char **)&error_message);
		efree(tmp);

		if (error) {
			zend_throw_exception(mongo_ce_ConnectionException, error_message, 0 TSRMLS_CC);
			free(error_message);
			return;
		}
	}

	/* If "w" was *not* set as an option, then assign the default */
	if (link->servers->options.default_w == -1) {
		if (bc) {
			/* Default to WriteConcern=0 for Mongo */
			link->servers->options.default_w = 0;
		} else {
			/* Default to WriteConcern=1 for MongoClient */
			link->servers->options.default_w = 1;
		}
	}

	/* Options through array */
	if (options) {
		for (
			zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(options), &pos);
			zend_hash_get_current_data_ex(Z_ARRVAL_P(options), (void **) &opt_entry, &pos) == SUCCESS;
			zend_hash_move_forward_ex(Z_ARRVAL_P(options), &pos)
		) {
			switch (zend_hash_get_current_key_ex(Z_ARRVAL_P(options), &opt_key, &opt_key_len, &num_key, 0, &pos)) {
				case HASH_KEY_IS_STRING: {
					int error = 0;

					error = mongo_store_option_wrapper(link->manager, link->servers, opt_key, opt_entry, (char **)&error_message);

					switch (error) {
						case -1: /* Deprecated options */
							if (strcasecmp(opt_key, "slaveOkay") == 0) {
								php_error_docref(NULL TSRMLS_CC, MONGO_E_DEPRECATED, "The 'slaveOkay' option is deprecated. Please switch to read-preferences");
							} else if (strcasecmp(opt_key, "timeout") == 0) {
								php_error_docref(NULL TSRMLS_CC, MONGO_E_DEPRECATED, "The 'timeout' option is deprecated. Please use 'connectTimeoutMS' instead");
							}
							break;
						case 4: /* Special options parameters, invalid for URL parsing - only possiblity is 'connect' for now */
							if (strcasecmp(opt_key, "connect") == 0) {
								convert_to_boolean_ex(opt_entry);
								connect = Z_BVAL_PP(opt_entry);
							}
							break;

						case 3: /* Logical error (i.e. conflicting options) */
						case 2: /* Unknown connection string option */
						case 1: /* Empty option name or value */
							/* Throw exception - error code is 20 + above value. They are defined in php_mongo.h */
							zend_throw_exception(mongo_ce_ConnectionException, error_message, 20 + error TSRMLS_CC);
							free(error_message);
							return;
					}
				} break;

				case HASH_KEY_IS_LONG:
					/* Throw exception - error code is 25. This is defined in php_mongo.h */
					zend_throw_exception(mongo_ce_ConnectionException, "Unrecognized or unsupported option", 25 TSRMLS_CC);
					return;
			}
		}
	}

#if !MONGO_PHP_STREAMS
	if (link->servers->options.ssl) {
		zend_throw_exception(mongo_ce_ConnectionException, "SSL support is only available when compiled against PHP Streams", 26 TSRMLS_CC);
		return;
	}
	if (zdoptions) {
		zend_throw_exception(mongo_ce_ConnectionException, "Driver options are only available when compiled against PHP Streams", 27 TSRMLS_CC);
		return;
	}
#endif
	if (zdoptions) {
		/* Possibly add more indexes in the future, like "log_queries",
		 * "log_commands", "log_failures", "log_metadata"... */
		zval **zcontext;

		if (zend_hash_find(Z_ARRVAL_P(zdoptions), "context", strlen("context") + 1, (void**)&zcontext) == SUCCESS) {
			link->servers->options.ctx = php_stream_context_from_zval(*zcontext, PHP_FILE_NO_DEFAULT_CONTEXT);
			mongo_manager_log(link->manager, MLOG_CON, MLOG_INFO, "Found Stream context");
		}
	}


	slave_okay = zend_read_static_property(mongo_ce_Cursor, "slaveOkay", strlen("slaveOkay"), NOISY TSRMLS_CC);
	if (Z_BVAL_P(slave_okay)) {
		if (link->servers->read_pref.type != MONGO_RP_PRIMARY) {
			/* The server already has read preferences configured, but we're still
			 * trying to set slave okay. The spec says that's an error, so we
			 * throw an exception with code 23 (defined in php_mongo.h) */
			zend_throw_exception(mongo_ce_ConnectionException, "You can not use both slaveOkay and read-preferences. Please switch to read-preferences.", 23 TSRMLS_CC);
			return;
		} else {
			/* Old style option, that needs to be removed. For now, spec dictates
			 * it needs to be ReadPreference=SECONDARY_PREFERRED */
			link->servers->read_pref.type = MONGO_RP_SECONDARY_PREFERRED;
		}
	}

	if (connect) {
		php_mongo_connect(link, MONGO_CON_FLAG_READ|MONGO_CON_FLAG_DONT_FILTER TSRMLS_CC);
	}
}
/* }}} */


/* {{{ MongoClient->connect
 */
PHP_METHOD(MongoClient, connect)
{
	mongoclient *link;

	PHP_MONGO_GET_LINK(getThis());
	RETURN_BOOL(php_mongo_connect(link, MONGO_CON_FLAG_READ TSRMLS_CC) != NULL);
}
/* }}} */


/* {{{ proto int MongoClient->close([string|bool hash|all])
   Closes the connection to $hash, or only master - or all open connections. Returns how many connections were closed */
PHP_METHOD(MongoClient, close)
{
	char             *hash = NULL;
	int               hash_len;
	mongoclient       *link;
	mongo_connection *connection;
	char             *error_message = NULL;
	zval             *all = NULL;

	PHP_MONGO_GET_LINK(getThis());
	if (ZEND_NUM_ARGS() == 0) {
		/* BC: Close master when no arguments passed */
		connection = mongo_get_read_write_connection(link->manager, link->servers, MONGO_CON_FLAG_WRITE|MONGO_CON_FLAG_DONT_CONNECT, (char **) &error_message);
		RETVAL_LONG(close_connection(link->manager, connection));
	} else if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "z", &all) == SUCCESS && Z_TYPE_P(all) == IS_BOOL) {
		if (Z_BVAL_P(all)) {
			/* Close all connections */
			mongo_con_manager_item *ptr = link->manager->connections;
			mongo_con_manager_item *current;
			long                    count = 0;

			while (ptr) {
				current = ptr;
				ptr = ptr->next;
				close_connection(link->manager, (mongo_connection *)current->data);
				count++;
			}

			RETVAL_LONG(count);
		} else {
			/* Close master */
			connection = mongo_get_read_write_connection(link->manager, link->servers, MONGO_CON_FLAG_WRITE|MONGO_CON_FLAG_DONT_CONNECT, (char **) &error_message);
			RETVAL_LONG(close_connection(link->manager, connection));
		}
	} else if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hash, &hash_len) == SUCCESS) {
		/* Lookup hash and destroy it */
		connection = mongo_manager_connection_find_by_hash(link->manager, hash);
		if (!connection) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "A connection with hash '%s' does not exist.", hash);
			RETURN_LONG(0);
		}
		RETVAL_LONG(close_connection(link->manager, connection));
	} else {
		return;
	}

	if (error_message) {
		free(error_message);
	}
	RETURN_TRUE;
}
/* }}} */

static int close_connection(mongo_con_manager *manager, mongo_connection *connection)
{
	if (connection) {
		mongo_manager_connection_deregister(manager, connection);
		return 1;
	} else {
		return 0;
	}
}

static void stringify_server(mongo_server_def *server, smart_str *str)
{
	/* copy host */
	smart_str_appends(str, server->host);
	smart_str_appendc(str, ':');
	smart_str_append_long(str, server->port);
}


/* {{{ MongoClient->__toString()
 */
PHP_METHOD(MongoClient, __toString)
{
	smart_str str = { 0 };
	mongoclient *link;
	int i;

	PHP_MONGO_GET_LINK(getThis());

	for (i = 0; i < link->servers->count; i++) {
		/* if this is not the first one, add a comma */
		if (i) {
			smart_str_appendc(&str, ',');
		}

		stringify_server(link->servers->server[i], &str);
	}

	smart_str_0(&str);

	RETURN_STRING(str.c, 0);
}
/* }}} */


/* {{{ MongoClient->selectDB()
 */
PHP_METHOD(MongoClient, selectDB)
{
	zval temp, *name;
	char *db;
	int db_len;
	mongoclient *link;
	int free_this_ptr = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &db, &db_len) == FAILURE) {
		return;
	}

	if (memchr(db, '\0', db_len) != NULL) {
		zend_throw_exception_ex(mongo_ce_Exception, 2 TSRMLS_CC, "'\\0' not allowed in database names: %s\\0...", db);
		return;
	}

	MAKE_STD_ZVAL(name);
	ZVAL_STRINGL(name, db, db_len, 1);

	PHP_MONGO_GET_LINK(getThis());

	/* We need to check whether we are switching to a database that was not
	 * part of the connection string. This is not a problem if we are not using
	 * authentication, but it is if we are. If we are, we need to do some fancy
	 * cloning and creating a new mongo_servers structure. Authentication is a
	 * pain™ */
	if (link->servers->server[0]->db && strcmp(link->servers->server[0]->db, db) != 0) {
		mongo_manager_log(
			link->manager, MLOG_CON, MLOG_INFO,
			"The requested database (%s) is not what we have in the link info (%s)",
			db, link->servers->server[0]->db
		);
		/* So here we check if a username and password are used. If so, the
		 * madness starts */
		if (link->servers->server[0]->username && link->servers->server[0]->password) {
			zval       *new_link;
			mongoclient *tmp_link;
			int i;
		
			if (strcmp(link->servers->server[0]->db, "admin") == 0) {
				mongo_manager_log(
					link->manager, MLOG_CON, MLOG_FINE,
					"The link info has 'admin' as database, no need to clone it then"
				);
			} else {
				mongo_manager_log(
					link->manager, MLOG_CON, MLOG_INFO,
					"We are in an authenticated link (db: %s, user: %s), so we need to clone it.",
					link->servers->server[0]->db, link->servers->server[0]->username
				);

				/* Create the new link object */
				MAKE_STD_ZVAL(new_link);
				object_init_ex(new_link, mongo_ce_MongoClient);
				tmp_link = (mongoclient*) zend_object_store_get_object(new_link TSRMLS_CC);

				tmp_link->manager = link->manager;
				tmp_link->servers = (mongo_servers*) calloc(1, sizeof(mongo_servers));
				mongo_servers_copy(tmp_link->servers, link->servers, MONGO_SERVER_COPY_CREDENTIALS);
				/* We assume the previous credentials will work on this
				 * database too, or if authSource is set, authenticate against
				 * that database */
				for (i = 0; i < tmp_link->servers->count; i++) {
					tmp_link->servers->server[i]->db = strdup(db);
				}

				this_ptr = new_link;
				free_this_ptr = 1;
			}
		}
	}

	object_init_ex(return_value, mongo_ce_DB);
	MONGO_METHOD2(MongoDB, __construct, &temp, return_value, getThis(), name);

	zval_ptr_dtor(&name);
	if (free_this_ptr) {
		zval_ptr_dtor(&this_ptr);
	}
}
/* }}} */


/* {{{ Mongo::__get
 */
PHP_METHOD(MongoClient, __get)
{
	zval *name;
	char *str;
	int str_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
		return;
	}

	MAKE_STD_ZVAL(name);
	ZVAL_STRINGL(name, str, str_len, 1);

	/* select this db */
	MONGO_METHOD1(MongoClient, selectDB, return_value, getThis(), name);

	zval_ptr_dtor(&name);
}
/* }}} */


/* {{{ Mongo::selectCollection()
 */
PHP_METHOD(MongoClient, selectCollection)
{
	char *db, *coll;
	int db_len, coll_len;
	zval *db_name, *coll_name, *temp_db;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &db, &db_len, &coll, &coll_len) == FAILURE) {
		return;
	}

	MAKE_STD_ZVAL(db_name);
	ZVAL_STRINGL(db_name, db, db_len, 1);

	MAKE_STD_ZVAL(temp_db);
	MONGO_METHOD1(MongoClient, selectDB, temp_db, getThis(), db_name);
	zval_ptr_dtor(&db_name);
	PHP_MONGO_CHECK_EXCEPTION1(&temp_db);

	MAKE_STD_ZVAL(coll_name);
	ZVAL_STRINGL(coll_name, coll, coll_len, 1);

	MONGO_METHOD1(MongoDB, selectCollection, return_value, temp_db, coll_name);

	zval_ptr_dtor(&coll_name);
	zval_ptr_dtor(&temp_db);
}
/* }}} */


/* {{{ array Mongo::getReadPreference()
 * Returns the currently set read preference.*/
PHP_METHOD(MongoClient, getReadPreference)
{
	mongoclient *link;
	PHP_MONGO_GET_LINK(getThis());

	array_init(return_value);
	add_assoc_string(return_value, "type", mongo_read_preference_type_to_name(link->servers->read_pref.type), 1);
	php_mongo_add_tagsets(return_value, &link->servers->read_pref);
}
/* }}} */


/* {{{ Mongo::setReadPreference(string read_preference [, array tags ])
 * Sets a read preference to be used for all read queries.*/
PHP_METHOD(MongoClient, setReadPreference)
{
	char *read_preference;
	int   read_preference_len;
	mongoclient *link;
	HashTable  *tags = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|h", &read_preference, &read_preference_len, &tags) == FAILURE) {
		return;
	}

	PHP_MONGO_GET_LINK(getThis());

	if (php_mongo_set_readpreference(&link->servers->read_pref, read_preference, tags TSRMLS_CC)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ Mongo::dropDB()
 */
PHP_METHOD(MongoClient, dropDB)
{
	zval *db, *temp_db;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &db) == FAILURE) {
		RETURN_FALSE;
	}

	if (Z_TYPE_P(db) != IS_OBJECT || Z_OBJCE_P(db) != mongo_ce_DB) {
		MAKE_STD_ZVAL(temp_db);
		ZVAL_NULL(temp_db);

		/* reusing db param from Mongo::drop call */
		MONGO_METHOD_BASE(MongoClient, selectDB)(1, temp_db, NULL, getThis(), 0 TSRMLS_CC);
		db = temp_db;
	} else {
		zval_add_ref(&db);
	}

	MONGO_METHOD(MongoDB, drop, return_value, db);
	zval_ptr_dtor(&db);
}
/* }}} */

/* {{{ MongoClient->listDBs
 */
PHP_METHOD(MongoClient, listDBs)
{
	zval *admin, *data, *db;

	MAKE_STD_ZVAL(admin);
	ZVAL_STRING(admin, "admin", 1);

	MAKE_STD_ZVAL(db);

	MONGO_METHOD1(MongoClient, selectDB, db, getThis(), admin);

	zval_ptr_dtor(&admin);

	MAKE_STD_ZVAL(data);
	array_init(data);
	add_assoc_long(data, "listDatabases", 1);

	MONGO_CMD(return_value, db);

	zval_ptr_dtor(&data);
	zval_ptr_dtor(&db);
}
/* }}} */

/* {{{ MongoClient->getHosts
 */
PHP_METHOD(MongoClient, getHosts)
{
	mongoclient            *link;
	mongo_con_manager_item *item;

	PHP_MONGO_GET_LINK(getThis());
	item = link->manager->connections;

	array_init(return_value);

	while (item) {
		zval *infoz;
		char *host;
		int   port;
		mongo_connection *con = (mongo_connection*) item->data;

		MAKE_STD_ZVAL(infoz);
		array_init(infoz);

		mongo_server_split_hash(con->hash, (char**) &host, (int*) &port, NULL, NULL, NULL, NULL, NULL);
		add_assoc_string(infoz, "host", host, 1);
		add_assoc_long(infoz, "port", port);
		free(host);

		add_assoc_long(infoz, "health", 1);
		add_assoc_long(infoz, "state", con->connection_type == MONGO_NODE_PRIMARY ? 1 : (con->connection_type == MONGO_NODE_SECONDARY ? 2 : 0));
		add_assoc_long(infoz, "ping", con->ping_ms);
		add_assoc_long(infoz, "lastPing", con->last_ping);

		add_assoc_zval(return_value, con->hash, infoz);
		item = item->next;
	}
}
/* }}} */

/* {{{ proto static array Mongo::getConnections(void)
   Returns an array of all open connections, and information about each of the servers */
PHP_METHOD(MongoClient, getConnections)
{
	mongo_con_manager_item *ptr;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ptr = MonGlo(manager)->connections;

	array_init(return_value);
	while (ptr) {
		zval *entry, *server, *connection, *tags;
		char *host, *repl_set_name, *database, *username, *auth_hash;
		int port, pid, i;
		mongo_connection *con = (mongo_connection*) ptr->data;

		MAKE_STD_ZVAL(entry);
		array_init(entry);

		MAKE_STD_ZVAL(server);
		array_init(server);

		MAKE_STD_ZVAL(connection);
		array_init(connection);

		MAKE_STD_ZVAL(tags);
		array_init(tags);

		/* Grab server information */
		mongo_server_split_hash(con->hash, &host, &port, &repl_set_name, &database, &username, &auth_hash, &pid);

		add_assoc_string(server, "host", host, 1);
		free(host);
		add_assoc_long(server, "port", port);
		if (repl_set_name) {
			add_assoc_string(server, "repl_set_name", repl_set_name, 1);
			free(repl_set_name);
		}
		if (database) {
			add_assoc_string(server, "database", database, 1);
			free(database);
		}
		if (username) {
			add_assoc_string(server, "username", username, 1);
			free(username);
		}
		if (auth_hash) {
			add_assoc_string(server, "auth_hash", auth_hash, 1);
			free(auth_hash);
		}
		add_assoc_long(server, "pid", pid);

		/* Grab connection info */
		add_assoc_long(connection, "last_ping", con->last_ping);
		add_assoc_long(connection, "last_ismaster", con->last_ismaster);
		add_assoc_long(connection, "ping_ms", con->ping_ms);
		add_assoc_long(connection, "connection_type", con->connection_type);
		add_assoc_string(connection, "connection_type_desc", mongo_connection_type(con->connection_type), 1);
		add_assoc_long(connection, "max_bson_size", con->max_bson_size);
		add_assoc_long(connection, "tag_count", con->tag_count);
		for (i = 0; i < con->tag_count; i++) {
			add_next_index_string(tags, con->tags[i], 1);
		}
		add_assoc_zval(connection, "tags", tags);

		/* Top level elements */
		add_assoc_string(entry, "hash", con->hash, 1);
		add_assoc_zval(entry, "server", server);
		add_assoc_zval(entry, "connection", connection);
		add_next_index_zval(return_value, entry);

		ptr = ptr->next;
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
