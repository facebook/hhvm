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
#include <php.h>
#include <zend_exceptions.h>

#include "php_mongo.h"
#include "collection.h"
#include "cursor.h"
#include "bson.h"
#include "types/code.h"
#include "types/db_ref.h"
#include "db.h"
#include "mcon/manager.h"
#include "mcon/io.h"
#include "log_stream.h"

extern zend_class_entry *mongo_ce_MongoClient, *mongo_ce_DB, *mongo_ce_Cursor;
extern zend_class_entry *mongo_ce_Code, *mongo_ce_Exception, *mongo_ce_ResultException;

extern int le_pconnection, le_connection;
extern zend_object_handlers mongo_default_handlers;

ZEND_EXTERN_MODULE_GLOBALS(mongo)

zend_class_entry *mongo_ce_Collection = NULL;

static mongo_connection* get_server(mongo_collection *c, int connection_flags TSRMLS_DC);
static int is_gle_op(zval *options, mongo_server_options *server_options TSRMLS_DC);
static void do_safe_op(mongo_con_manager *manager, mongo_connection *connection, zval *cursor_z, buffer *buf, zval *return_value TSRMLS_DC);
static zval* append_getlasterror(zval *coll, buffer *buf, zval *options, mongo_connection *connection TSRMLS_DC);
static int php_mongo_trigger_error_on_command_failure(zval *document TSRMLS_DC);

/* {{{ proto MongoCollection MongoCollection::__construct(MongoDB db, string name)
   Initializes a new MongoCollection */
PHP_METHOD(MongoCollection, __construct)
{
	zval *parent, *name, *zns, *w, *wtimeout;
	mongo_collection *c;
	mongo_db *db;
	char *ns, *name_str;
	int name_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os", &parent, mongo_ce_DB, &name_str, &name_len) == FAILURE) {
		zval *object = getThis();
		ZVAL_NULL(object);
		return;
	}

	/* check for empty and invalid collection names */
	if (
		name_len == 0 ||
		memchr(name_str, '\0', name_len) != 0
	) {
		zend_throw_exception_ex(mongo_ce_Exception, 2 TSRMLS_CC, "MongoDB::__construct(): invalid name %s", name_str);
		return;
	}

	c = (mongo_collection*)zend_object_store_get_object(getThis() TSRMLS_CC);

	PHP_MONGO_GET_DB(parent);

	c->link = db->link;
	zval_add_ref(&db->link);

	c->parent = parent;
	zval_add_ref(&parent);

	MAKE_STD_ZVAL(name);
	ZVAL_STRINGL(name, name_str, name_len, 1);
	c->name = name;

	spprintf(&ns, 0, "%s.%s", Z_STRVAL_P(db->name), Z_STRVAL_P(name));

	MAKE_STD_ZVAL(zns);
	ZVAL_STRING(zns, ns, 0);
	c->ns = zns;
	mongo_read_preference_copy(&db->read_pref, &c->read_pref);

	w = zend_read_property(mongo_ce_DB, parent, "w", strlen("w"), NOISY TSRMLS_CC);
	if (Z_TYPE_P(w) == IS_STRING) {
		zend_update_property_string(mongo_ce_Collection, getThis(), "w", strlen("w"), Z_STRVAL_P(w) TSRMLS_CC);
	} else {
		convert_to_long(w);
		zend_update_property_long(mongo_ce_Collection, getThis(), "w", strlen("w"), Z_LVAL_P(w) TSRMLS_CC);
	}

	wtimeout = zend_read_property(mongo_ce_DB, parent, "wtimeout", strlen("wtimeout"), NOISY TSRMLS_CC);
	convert_to_long(wtimeout);
	zend_update_property_long(mongo_ce_Collection, getThis(), "wtimeout", strlen("wtimeout"), Z_LVAL_P(wtimeout) TSRMLS_CC);
}
/* }}} */

/* {{{ proto string MongoCollection::__toString()
   Returns the full namespace for this collection (includes database name) */
PHP_METHOD(MongoCollection, __toString)
{
	mongo_collection *c;

	PHP_MONGO_GET_COLLECTION(getThis());
	RETURN_ZVAL(c->ns, 1, 0);
}
/* }}} */

/* {{{ proto string MongoCollection::getName()
   Returns the collection name */
PHP_METHOD(MongoCollection, getName)
{
	mongo_collection *c;

	PHP_MONGO_GET_COLLECTION(getThis());
	RETURN_ZVAL(c->name, 1, 0);
}
/* }}} */

/* {{{ proto bool MongoCollection::getSlaveOkay()
   Returns the slaveOkay flag for this collection */
PHP_METHOD(MongoCollection, getSlaveOkay)
{
	mongo_collection *c;

	PHP_MONGO_GET_COLLECTION(getThis());
	RETURN_BOOL(c->read_pref.type != MONGO_RP_PRIMARY);
}
/* }}} */

/* {{{ proto bool MongoCollection::setSlaveOkay([bool slave_okay = true])
   Sets the slaveOkay flag for this collection and returns the previous value */
PHP_METHOD(MongoCollection, setSlaveOkay)
{
	zend_bool slave_okay = 1;
	mongo_collection *c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &slave_okay) == FAILURE) {
		return;
	}

	PHP_MONGO_GET_COLLECTION(getThis());

	RETVAL_BOOL(c->read_pref.type != MONGO_RP_PRIMARY);
	c->read_pref.type = slave_okay ? MONGO_RP_SECONDARY_PREFERRED : MONGO_RP_PRIMARY;
}
/* }}} */

/* {{{ proto array MongoCollection::getReadPreference()
   Returns an array describing the read preference for this collection. Tag sets will be included if available. */
PHP_METHOD(MongoCollection, getReadPreference)
{
	mongo_collection *c;
	PHP_MONGO_GET_COLLECTION(getThis());

	array_init(return_value);
	add_assoc_string(return_value, "type", mongo_read_preference_type_to_name(c->read_pref.type), 1);
	php_mongo_add_tagsets(return_value, &c->read_pref);
}
/* }}} */

/* {{{ proto bool MongoCollection::setReadPreference(string read_preference [, array tags ])
   Sets the read preference for this collection */
PHP_METHOD(MongoCollection, setReadPreference)
{
	char *read_preference;
	int   read_preference_len;
	mongo_collection *c;
	HashTable  *tags = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|h", &read_preference, &read_preference_len, &tags) == FAILURE) {
		return;
	}

	PHP_MONGO_GET_COLLECTION(getThis());

	if (php_mongo_set_readpreference(&c->read_pref, read_preference, tags TSRMLS_CC)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array MongoCollection::drop()
   Drops the current collection and returns the database response */
PHP_METHOD(MongoCollection, drop)
{
	zval *data;
	mongo_collection *c;

	PHP_MONGO_GET_COLLECTION(getThis());

	MAKE_STD_ZVAL(data);
	array_init(data);
	add_assoc_zval(data, "drop", c->name);
	zval_add_ref(&c->name);

	MONGO_CMD(return_value, c->parent);

	zval_ptr_dtor(&data);
}
/* }}} */

/* {{{ proto array MongoCollection::validate([bool scan_data])
   Validates the current collection, optionally include the data, and returns the database response */
PHP_METHOD(MongoCollection, validate)
{
	zval *data;
	zend_bool scan_data = 0;
	mongo_collection *c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &scan_data) == FAILURE) {
		return;
	}

	PHP_MONGO_GET_COLLECTION(getThis());

	MAKE_STD_ZVAL(data);
	array_init(data);
	add_assoc_string(data, "validate", Z_STRVAL_P(c->name), 1);
	add_assoc_bool(data, "full", scan_data);

	MONGO_CMD(return_value, c->parent);

	zval_ptr_dtor(&data);
}
/* }}} */

/* This should probably be split into two methods... right now appends the
 * getlasterror query to the buffer and alloc & inits the cursor zval. */
static zval* append_getlasterror(zval *coll, buffer *buf, zval *options, mongo_connection *connection TSRMLS_DC)
{
	zval *cmd_ns_z, *cmd, *cursor_z, *temp, *timeout_p;
	char *cmd_ns, *w_str = NULL;
	mongo_cursor *cursor;
	mongo_collection *c = (mongo_collection*)zend_object_store_get_object(coll TSRMLS_CC);
	mongo_db *db = (mongo_db*)zend_object_store_get_object(c->parent TSRMLS_CC);
	int response, w = 0, fsync = 0, journal = 0, timeout = -1;
	mongoclient *link = (mongoclient*) zend_object_store_get_object(c->link TSRMLS_CC);
	int max_document_size = connection->max_bson_size;
	int max_message_size = connection->max_message_size;

	mongo_manager_log(MonGlo(manager), MLOG_IO, MLOG_FINE, "append_getlasterror");

	timeout_p = zend_read_static_property(mongo_ce_Cursor, "timeout", strlen("timeout"), NOISY TSRMLS_CC);
	convert_to_long(timeout_p);
	timeout = Z_LVAL_P(timeout_p);

	/* Overwrite the timeout if MongoCursor::$timeout is the default and we
	 * passed in socketTimeoutMS in the connection string */
	if (timeout == PHP_MONGO_DEFAULT_SOCKET_TIMEOUT && link->servers->options.socketTimeoutMS > 0) {
		timeout = link->servers->options.socketTimeoutMS;
	}

    /* Get the default value for journalling */
	fsync = link->servers->options.default_fsync;
	journal = link->servers->options.default_journal;

	/* Read the default_* properties from the link */
	if (link->servers->options.default_w != -1) {
		w = link->servers->options.default_w;
	}
	if (link->servers->options.default_wstring != NULL) {
		w_str = link->servers->options.default_wstring;
	}

	/* This picks up the default "w" through the properties of MongoCollection
	 * and MongoDb, but only if w is still 1 - as otherwise it was perhaps
	 * overridden with the "w" (or "safe") option. */
	{
		zval *w_prop = zend_read_property(mongo_ce_Collection, coll, "w", strlen("w"), NOISY TSRMLS_CC);

		if (Z_TYPE_P(w_prop) == IS_STRING) {
			w_str = Z_STRVAL_P(w_prop);
		} else {
			convert_to_long(w_prop);
			if (Z_LVAL_P(w_prop) != 1) {
				w = Z_LVAL_P(w_prop);
				w_str = NULL;
			}
		}
	}

	/* Fetch all the options from the options array */
	if (options && !IS_SCALAR_P(options)) {
		zval **w_pp = NULL, **fsync_pp, **timeout_pp, **journal_pp;

		/* First we try "w", and if that is not found we check for "safe" */
		if (zend_hash_find(HASH_P(options), "w", strlen("w") + 1, (void**) &w_pp) == SUCCESS) {
			switch (Z_TYPE_PP(w_pp)) {
				case IS_STRING:
					w_str = Z_STRVAL_PP(w_pp);
					break;
				case IS_BOOL:
				case IS_LONG:
					w = Z_LVAL_PP(w_pp); /* This is actually "wrong" for bools, but it works */
					break;
				default:
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "The value of the 'w' option either needs to be a integer or string");
			}
		} else if(zend_hash_find(HASH_P(options), "safe", strlen("safe") + 1, (void**) &w_pp) == SUCCESS) {
			switch (Z_TYPE_PP(w_pp)) {
				case IS_STRING:
					w_str = Z_STRVAL_PP(w_pp);
					break;
				case IS_LONG:
					w = Z_LVAL_PP(w_pp);
					break;
				case IS_BOOL:
					if (Z_BVAL_PP(w_pp)) {
						/* If we already provided Write Concern, do not overwrite it with w=1 */
						if (!(w > 1 || w_str)) {
							w = 1;
						}
					} else {
						w = 0;
					}
					break;
				default:
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "The value of the 'safe' option either needs to be a integer or string");
			}
		}

		if (SUCCESS == zend_hash_find(HASH_P(options), "fsync", strlen("fsync") + 1, (void**) &fsync_pp)) {
			convert_to_boolean(*fsync_pp);
			fsync = Z_BVAL_PP(fsync_pp);
		}

		if (zend_hash_find(HASH_P(options), "j", strlen("j") + 1, (void**) &journal_pp) == SUCCESS) {
			convert_to_boolean(*journal_pp);
			journal = Z_BVAL_PP(journal_pp);
		}

		if (SUCCESS == zend_hash_find(HASH_P(options), "timeout", strlen("timeout") + 1, (void**) &timeout_pp)) {
			convert_to_long(*timeout_pp);
			timeout = Z_LVAL_PP(timeout_pp);
		}
	}

	/* fsync forces "w" to be atleast 1, so don't touch it if it's
	 * already set to something else above while parsing "w" (and
	 * "safe") */
	if (fsync && w == 0) {
		w = 1;
	}

	/* get "db.$cmd" zval */
	MAKE_STD_ZVAL(cmd_ns_z);
	spprintf(&cmd_ns, 0, "%s.$cmd", Z_STRVAL_P(db->name));
	ZVAL_STRING(cmd_ns_z, cmd_ns, 0);

	/* get {"getlasterror" : 1} zval */
	MAKE_STD_ZVAL(cmd);
	array_init(cmd);
	add_assoc_long(cmd, "getlasterror", 1);

	/* if we have either a string, or w > 1, then we need to add "w" and
	 * perhaps "wtimeout" to GLE */
	if (w_str || w > 1) {
		zval *wtimeout, **wtimeout_pp;

		if (w_str) {
			add_assoc_string(cmd, "w", w_str, 1);
			mongo_manager_log(MonGlo(manager), MLOG_IO, MLOG_FINE, "append_getlasterror: added w='%s'", w_str);
		} else {
			add_assoc_long(cmd, "w", w);
			mongo_manager_log(MonGlo(manager), MLOG_IO, MLOG_FINE, "append_getlasterror: added w=%d", w);
		}

		if (options && zend_hash_find(HASH_P(options), "wTimeoutMS", strlen("wTimeoutMS") + 1, (void **)&wtimeout_pp) == SUCCESS) {
			convert_to_long(*wtimeout_pp);
			add_assoc_long(cmd, "wtimeout", Z_LVAL_PP(wtimeout_pp));
			mongo_manager_log(MonGlo(manager), MLOG_IO, MLOG_FINE, "append_getlasterror: added wtimeout=%d (wTimeoutMS from options array)", Z_LVAL_PP(wtimeout_pp));
		} else if (options && zend_hash_find(HASH_P(options), "wtimeout", strlen("wtimeout") + 1, (void **)&wtimeout_pp) == SUCCESS) {
			convert_to_long(*wtimeout_pp);
			add_assoc_long(cmd, "wtimeout", Z_LVAL_PP(wtimeout_pp));
			mongo_manager_log(MonGlo(manager), MLOG_IO, MLOG_FINE, "append_getlasterror: added wtimeout=%d (wtimeout from options array)", Z_LVAL_PP(wtimeout_pp));
		} else {
			wtimeout = zend_read_property(mongo_ce_Collection, coll, "wtimeout", strlen("wtimeout"), NOISY TSRMLS_CC);
			convert_to_long(wtimeout);
			add_assoc_long(cmd, "wtimeout", Z_LVAL_P(wtimeout));
			mongo_manager_log(MonGlo(manager), MLOG_IO, MLOG_FINE, "append_getlasterror: added wtimeout=%d (from collection property)", Z_LVAL_P(wtimeout));
		}
	}

	if (fsync) {
		add_assoc_bool(cmd, "fsync", 1);
		mongo_manager_log(MonGlo(manager), MLOG_IO, MLOG_FINE, "append_getlasterror: added fsync=1");
	}

	if (journal) {
		add_assoc_bool(cmd, "journal", 1);
		mongo_manager_log(MonGlo(manager), MLOG_IO, MLOG_FINE, "append_getlasterror: added journal=1");
	}

	/* get cursor */
	MAKE_STD_ZVAL(cursor_z);
	object_init_ex(cursor_z, mongo_ce_Cursor);

	MAKE_STD_ZVAL(temp);
	ZVAL_NULL(temp);
	MONGO_METHOD2(MongoCursor, __construct, temp, cursor_z, c->link, cmd_ns_z);
	zval_ptr_dtor(&temp);
	if (EG(exception)) {
		zval_ptr_dtor(&cursor_z);
		zval_ptr_dtor(&cmd_ns_z);
		zval_ptr_dtor(&cmd);
		return 0;
	}

	cursor = (mongo_cursor*)zend_object_store_get_object(cursor_z TSRMLS_CC);

	/* Make sure the "getLastError" also gets send to a primary. This should
	 * be refactored alongside with the getLastError redirection in
	 * db.c/MongoDB::command. The Cursor creation should be done through an
	 * init method otherwise a connection have to be requested twice. */
	mongo_manager_log(link->manager, MLOG_CON, MLOG_INFO, "forcing primary for getlasterror");
	php_mongo_connection_force_primary(cursor);

	cursor->limit = -1;
	cursor->timeout = timeout;
	zval_ptr_dtor(&cursor->query);
	/* cmd is now part of cursor, so it shouldn't be dtored until cursor is */
	cursor->query = cmd;

	/* append the query */
	response = php_mongo_write_query(buf, cursor, max_document_size, max_message_size TSRMLS_CC);
	zval_ptr_dtor(&cmd_ns_z);

#if MONGO_PHP_STREAMS
	mongo_log_stream_query(connection, cursor TSRMLS_CC);
#endif

	if (FAILURE == response) {
		zval_ptr_dtor(&cursor_z);
		return 0;
	}

	return cursor_z;
}

/* Returns a connection for the operation.
 * Connection flags (connection_flags) are MONGO_CON_TYPE_READ and MONGO_CON_TYPE_WRITE. */
static mongo_connection* get_server(mongo_collection *c, int connection_flags TSRMLS_DC)
{
	mongoclient *link;
	mongo_connection *connection;
	char *error_message = NULL;

	link = (mongoclient*)zend_object_store_get_object((c->link) TSRMLS_CC);
	if (!link) {
		zend_throw_exception(mongo_ce_Exception, "The MongoCollection object has not been correctly initialized by its constructor", 17 TSRMLS_CC);
		return 0;
	}

	/* TODO: Fix better error message */
	if ((connection = mongo_get_read_write_connection(link->manager, link->servers, connection_flags, (char **) &error_message)) == NULL) {
		if (error_message) {
			mongo_cursor_throw(NULL, 16 TSRMLS_CC, "Couldn't get connection: %s", error_message);
			free(error_message);
		} else {
			mongo_cursor_throw(NULL, 16 TSRMLS_CC, "Couldn't get connection");
		}
		return 0;
	}

	return connection;
}

/* Wrapper for sending and wrapping in a safe op */
static int send_message(zval *this_ptr, mongo_connection *connection, buffer *buf, zval *options, zval *return_value TSRMLS_DC)
{
	int retval = 1;
	char *error_message = NULL;
	mongoclient *link;
	mongo_collection *c;

	c = (mongo_collection*)zend_object_store_get_object(this_ptr TSRMLS_CC);
	if (!c->ns) {
		zend_throw_exception(mongo_ce_Exception, "The MongoCollection object has not been correctly initialized by its constructor", 0 TSRMLS_CC);
		return 0;
	}

	link = (mongoclient*)zend_object_store_get_object((c->link) TSRMLS_CC);
	if (!link) {
		zend_throw_exception(mongo_ce_Exception, "The MongoCollection object has not been correctly initialized by its constructor", 17 TSRMLS_CC);
		return 0;
	}

	if (is_gle_op(options, &link->servers->options TSRMLS_CC)) {
		zval *cursor = append_getlasterror(getThis(), buf, options, connection TSRMLS_CC);
		if (cursor) {
			do_safe_op(link->manager, connection, cursor, buf, return_value TSRMLS_CC);
			retval = -1;
		} else {
			retval = 0;
		}
	} else if (link->manager->send(connection, &link->servers->options, buf->start, buf->pos - buf->start, (char **) &error_message) == -1) {
		/* TODO: Find out what to do with the error message here */
		free(error_message);
		retval = 0;
	} else {
		retval = 1;
	}
	return retval;
}


static int is_gle_op(zval *options, mongo_server_options *server_options TSRMLS_DC)
{
	zval **gle_pp = 0, **fsync_pp = 0;
	int    gle_op = 0;

	/* First we check for the global (connection string) default */
	if (server_options->default_w != -1) {
		gle_op = server_options->default_w;
	}
	if (server_options->default_fsync || server_options->default_journal) {
		gle_op = 1;
	}

	/* Then we check the options array that could overwrite the default */
	if (options && Z_TYPE_P(options) == IS_ARRAY) {
		zval **journal_pp;

		/* First we try "w", and if that is not found we check for "safe" */
		if (zend_hash_find(HASH_P(options), "w", strlen("w") + 1, (void**) &gle_pp) == FAILURE) {
			zend_hash_find(HASH_P(options), "safe", strlen("safe") + 1, (void**) &gle_pp);
		}
		/* After that, gle_pp is either still NULL, or set to something if one of
		 * the options was found */
		if (gle_pp) {
			/* Check for bool/int value >= 1 */
			if ((Z_TYPE_PP(gle_pp) == IS_LONG || Z_TYPE_PP(gle_pp) == IS_BOOL)) {
				gle_op = (Z_LVAL_PP(gle_pp) >= 1);

			/* Check for string value ("majority", or a tag) */
			} else if (Z_TYPE_PP(gle_pp) == IS_STRING) {
				gle_op = 1;

			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "The value of the 'safe' option either needs to be a boolean or a string");
			}
		}

		/* Check for "fsync" in options array */
		if (zend_hash_find(HASH_P(options), "fsync", strlen("fsync") + 1, (void**)&fsync_pp) == SUCCESS) {
			convert_to_boolean_ex(fsync_pp);
			if (Z_BVAL_PP(fsync_pp)) {
				gle_op = 1;
			}
		}

		/* Check for "j" in options array */
		if (zend_hash_find(HASH_P(options), "j", strlen("j") + 1, (void**)&journal_pp) == SUCCESS) {
			convert_to_boolean_ex(journal_pp);
			if (Z_BVAL_PP(journal_pp)) {
				gle_op = 1;
			}
		}
	}

	mongo_manager_log(MonGlo(manager), MLOG_IO, MLOG_FINE, "is_gle_op: %s", gle_op ? "yes" : "no");
	return gle_op;
}

#if PHP_VERSION_ID >= 50300
# define MONGO_ERROR_G EG
#else
# define MONGO_ERROR_G PG
#endif

/* This wrapper temporarily turns off the exception throwing bit if it has been
 * set (by calling mongo_cursor_throw() before). We can't call
 * mongo_cursor_throw after deregister as it frees up bits of memory that
 * mongo_cursor_throw uses to construct its error message.
 *
 * Without the disabling of the exception bit and when a user defined error
 * handler is used on the PHP side, the notice would never been shown because
 * the exception bubbles up before the notice can actually be shown. By turning
 * the error handling mode to EH_NORMAL temporarily, we circumvent this
 * problem. */
static void connection_deregister_wrapper(mongo_con_manager *manager, mongo_connection *connection TSRMLS_DC)
{
	zend_error_handling_t orig_error_handling;

	/* Save EG/PG(error_handling) so that we can show log messages when we have
	 * already thrown an exception */
	orig_error_handling = MONGO_ERROR_G(error_handling);
	MONGO_ERROR_G(error_handling) = EH_NORMAL;

	mongo_manager_connection_deregister(manager, connection);

	MONGO_ERROR_G(error_handling) = orig_error_handling;
}

static void do_safe_op(mongo_con_manager *manager, mongo_connection *connection, zval *cursor_z, buffer *buf, zval *return_value TSRMLS_DC)
{
	zval *errmsg, **err;
	mongo_cursor *cursor;
	char *error_message;

	cursor = (mongo_cursor*)zend_object_store_get_object(cursor_z TSRMLS_CC);

	cursor->connection = connection;

	if (-1 == manager->send(connection, NULL, buf->start, buf->pos - buf->start, (char **) &error_message)) {
		mongo_manager_log(manager, MLOG_IO, MLOG_WARN, "do_safe_op: sending data failed, removing connection %s", connection->hash);
		mongo_cursor_throw(connection, 16 TSRMLS_CC, "%s", error_message);
		connection_deregister_wrapper(manager, connection TSRMLS_CC);

		free(error_message);
		cursor->connection = NULL;
		zval_ptr_dtor(&cursor_z);
		return;
	}

	/* get reply */
	MAKE_STD_ZVAL(errmsg);
	ZVAL_NULL(errmsg);

	if (FAILURE == php_mongo_get_reply(cursor, errmsg TSRMLS_CC)) {
		/* php_mongo_get_reply() throws exceptions */
		mongo_manager_connection_deregister(manager, connection);
		cursor->connection = NULL;
		zval_ptr_dtor(&cursor_z);
		zval_ptr_dtor(&errmsg);
		return;
	}
	zval_ptr_dtor(&errmsg);

	cursor->started_iterating = 1;

	MONGO_METHOD(MongoCursor, getNext, return_value, cursor_z);

	if (EG(exception) || (Z_TYPE_P(return_value) == IS_BOOL && Z_BVAL_P(return_value) == 0)) {
		cursor->connection = NULL;
		zval_ptr_dtor(&cursor_z);
		return;
	} else if (zend_hash_find(Z_ARRVAL_P(return_value), "errmsg", strlen("errmsg") + 1, (void**)&err) == SUCCESS && Z_TYPE_PP(err) == IS_STRING) {
		zval **code;
		int status = zend_hash_find(Z_ARRVAL_P(return_value), "n", strlen("n") + 1, (void**)&code);

		mongo_cursor_throw(cursor->connection, (status == SUCCESS ? Z_LVAL_PP(code) : 0) TSRMLS_CC, "%s", Z_STRVAL_PP(err));

		cursor->connection = NULL;
		zval_ptr_dtor(&cursor_z);
		return;
	}

	cursor->connection = NULL;
	zval_ptr_dtor(&cursor_z);
	return;
}


/* {{{ proto bool|array MongoCollection::insert(array|object document [, array options])
   Insert a document into the collection and return the database response if
   the write concern is >= 1. Otherwise, boolean true is returned if the
   document is not empty. */
PHP_METHOD(MongoCollection, insert)
{
	zval *a, *options = 0;
	mongo_collection *c;
	buffer buf;
	mongo_connection *connection;
	int retval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &a, &options) == FAILURE) {
		return;
	}
	MUST_BE_ARRAY_OR_OBJECT(1, a);

	PHP_MONGO_GET_COLLECTION(getThis());

	if ((connection = get_server(c, MONGO_CON_FLAG_WRITE TSRMLS_CC)) == 0) {
		RETURN_FALSE;
	}

	CREATE_BUF(buf, INITIAL_BUF_SIZE);
	if (FAILURE == php_mongo_write_insert(&buf, Z_STRVAL_P(c->ns), a, connection->max_bson_size, connection->max_message_size TSRMLS_CC)) {
		efree(buf.start);
		RETURN_FALSE;
	}

#if MONGO_PHP_STREAMS
	mongo_log_stream_insert(connection, a, options TSRMLS_CC);
#endif

	/* retval == -1 means a GLE response was received, so send_message() has
	 * either set return_value or thrown an exception via do_safe_op(). */
	retval = send_message(this_ptr, connection, &buf, options, return_value TSRMLS_CC);
	if (retval != -1) {
		RETVAL_BOOL(retval);
	}

	efree(buf.start);
}
/* }}} */

/* {{{ proto bool|array MongoCollection::batchInsert(array documents [, array options])
   Insert an array of documents and return the database response if the write
   concern is >= 1. Otherwise, a boolean value is returned indicating whether
   the batch was successfully sent. */
PHP_METHOD(MongoCollection, batchInsert)
{
	zval *docs, *options = NULL;
	mongo_collection *c;
	mongo_connection *connection;
	buffer buf;
	int bit_opts = 0;
	int retval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|z/", &docs, &options) == FAILURE) {
		return;
	}

	/* Options are only supported in the new-style, ie: an array of "named
	 * parameters": array("continueOnError" => true); */
	if (options) {
		zval **continue_on_error = NULL;

		if (zend_hash_find(HASH_P(options), "continueOnError", strlen("continueOnError") + 1, (void**)&continue_on_error) == SUCCESS) {
			convert_to_boolean_ex(continue_on_error);
			bit_opts = Z_BVAL_PP(continue_on_error) << 0;
		}

		Z_ADDREF_P(options);
	} else {
		MAKE_STD_ZVAL(options);
		array_init(options);
	}

	PHP_MONGO_GET_COLLECTION(getThis());

	if ((connection = get_server(c, MONGO_CON_FLAG_WRITE TSRMLS_CC)) == 0) {
		zval_ptr_dtor(&options);
		RETURN_FALSE;
	}

	CREATE_BUF(buf, INITIAL_BUF_SIZE);

	if (php_mongo_write_batch_insert(&buf, Z_STRVAL_P(c->ns), bit_opts, docs, connection->max_bson_size, connection->max_message_size TSRMLS_CC) == FAILURE) {
		efree(buf.start);
		zval_ptr_dtor(&options);
		return;
	}

#if MONGO_PHP_STREAMS
	mongo_log_stream_batchinsert(connection, docs, options, bit_opts TSRMLS_CC);
#endif

	/* retval == -1 means a GLE response was received, so send_message() has
	 * either set return_value or thrown an exception via do_safe_op(). */
	retval = send_message(this_ptr, connection, &buf, options, return_value TSRMLS_CC);
	if (retval != -1) {
		RETVAL_BOOL(retval);
	}

	efree(buf.start);
	zval_ptr_dtor(&options);
}
/* }}} */

/* {{{ proto array MongoCollection::find([array|object criteria [, array|object return_fields]])
   Query this collection for documents matching $criteria and use $return_fields
   as the projection. Return a MongoCursor for the result set. */
PHP_METHOD(MongoCollection, find)
{
	zval *query = 0, *fields = 0;
	mongo_collection *c;
	zval temp;
	mongo_cursor *cursor;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &query, &fields) == FAILURE) {
		return;
	}

	MUST_BE_ARRAY_OR_OBJECT(1, query);
	MUST_BE_ARRAY_OR_OBJECT(2, fields);

	PHP_MONGO_GET_COLLECTION(getThis());

	object_init_ex(return_value, mongo_ce_Cursor);

	/* Add read preferences to cursor */
	cursor = (mongo_cursor*)zend_object_store_get_object(return_value TSRMLS_CC);
	mongo_read_preference_replace(&c->read_pref, &cursor->read_pref);

	/* TODO: Don't call an internal function like this, but add a new C-level
	 * function for instantiating cursors */
	if (!query) {
		MONGO_METHOD2(MongoCursor, __construct, &temp, return_value, c->link, c->ns);
	} else if (!fields) {
		MONGO_METHOD3(MongoCursor, __construct, &temp, return_value, c->link, c->ns, query);
	} else {
		MONGO_METHOD4(MongoCursor, __construct, &temp, return_value, c->link, c->ns, query, fields);
	}
}
/* }}} */

/* {{{ proto array MongoCollection::findOne([array|object criteria [, array|object return_fields]])
   Return the first document that matches $criteria and use $return_fields as
   the projection. NULL will be returned if no document matches. */
PHP_METHOD(MongoCollection, findOne)
{
	zval *query = 0, *fields = 0, *cursor;
	zval temp;
	zval *limit = &temp;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &query, &fields) == FAILURE) {
		return;
	}
	MUST_BE_ARRAY_OR_OBJECT(1, query);
	MUST_BE_ARRAY_OR_OBJECT(2, fields);

	MAKE_STD_ZVAL(cursor);
	MONGO_METHOD_BASE(MongoCollection, find)(ZEND_NUM_ARGS(), cursor, NULL, getThis(), 0 TSRMLS_CC);
	PHP_MONGO_CHECK_EXCEPTION1(&cursor);

	ZVAL_LONG(limit, -1);
	MONGO_METHOD1(MongoCursor, limit, cursor, cursor, limit);
	MONGO_METHOD(MongoCursor, getNext, return_value, cursor);

	zend_objects_store_del_ref(cursor TSRMLS_CC);
	zval_ptr_dtor(&cursor);
}
/* }}} */

/* {{{ proto array MongoCollection::findAndModify(array query [, array update[, array fields [, array options]]])
   Atomically update and return a document */
PHP_METHOD(MongoCollection, findAndModify)
{
	zval *query, *update = 0, *fields = 0, *options = 0;
	zval *data, *tmpretval, **values;
	mongo_collection *c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a!|a!a!a!", &query, &update, &fields, &options) == FAILURE) {
		return;
	}

	c = (mongo_collection*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED(c->ns, MongoCollection);


	MAKE_STD_ZVAL(data);
	array_init(data);

	add_assoc_zval(data, "findandmodify", c->name);
	zval_add_ref(&c->name);

	if (query && zend_hash_num_elements(Z_ARRVAL_P(query)) > 0) {
		add_assoc_zval(data, "query", query);
		zval_add_ref(&query);
	}
	if (update && zend_hash_num_elements(Z_ARRVAL_P(update)) > 0) {
		add_assoc_zval(data, "update", update);
		zval_add_ref(&update);
	}
	if (fields && zend_hash_num_elements(Z_ARRVAL_P(fields)) > 0) {
		add_assoc_zval(data, "fields", fields);
		zval_add_ref(&fields);
	}
	if (options && zend_hash_num_elements(Z_ARRVAL_P(options)) > 0) {
		zval temp;
		zend_hash_merge(HASH_P(data), HASH_P(options), (void (*)(void*))zval_add_ref, &temp, sizeof(zval*), 1);
	}

	MAKE_STD_ZVAL(tmpretval);
	ZVAL_NULL(tmpretval);
	MONGO_CMD(tmpretval, c->parent);

	if (php_mongo_trigger_error_on_command_failure(tmpretval TSRMLS_CC) == SUCCESS) {
		if (zend_hash_find(Z_ARRVAL_P(tmpretval), "value", strlen("value") + 1, (void **)&values) == SUCCESS) {
			array_init(return_value);
			/* We may wind up with a NULL here if there simply aren't any results */
			if (Z_TYPE_PP(values) == IS_ARRAY) {
				zend_hash_copy(Z_ARRVAL_P(return_value), Z_ARRVAL_PP(values), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));
			}
		}
	} else {
		RETVAL_FALSE;
	}

	zval_ptr_dtor(&data);
	zval_ptr_dtor(&tmpretval);
}
/* }}} */

/* {{{ proto bool|array MongoCollection::update(array|object criteria, array|object $newobj [, array options])
   Update one or more documents matching $criteria with $newobj and return the
   database response if the write concern is >= 1. Otherwise, boolean true is
   returned. */
PHP_METHOD(MongoCollection, update)
{
	zval *criteria, *newobj, *options = NULL;
	mongo_collection *c;
	mongo_connection *connection;
	buffer buf;
	int bit_opts = 0;
	int retval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|a/", &criteria, &newobj, &options) == FAILURE) {
		return;
	}
	MUST_BE_ARRAY_OR_OBJECT(1, criteria);
	MUST_BE_ARRAY_OR_OBJECT(2, newobj);

	if (options) {
		zval **upsert = 0, **multiple = 0;

		if (zend_hash_find(HASH_P(options), "upsert", strlen("upsert") + 1, (void**)&upsert) == SUCCESS) {
			convert_to_boolean_ex(upsert);
			bit_opts |= Z_BVAL_PP(upsert) << 0;
		}

		if (zend_hash_find(HASH_P(options), "multiple", strlen("multiple") + 1, (void**)&multiple) == SUCCESS) {
			convert_to_boolean_ex(multiple);
			bit_opts |= Z_BVAL_PP(multiple) << 1;
		}

		Z_ADDREF_P(options);
	} else {
		MAKE_STD_ZVAL(options);
		array_init(options);
	}

	PHP_MONGO_GET_COLLECTION(getThis());

	if ((connection = get_server(c, MONGO_CON_FLAG_WRITE TSRMLS_CC)) == 0) {
		zval_ptr_dtor(&options);
		RETURN_FALSE;
	}

	CREATE_BUF(buf, INITIAL_BUF_SIZE);
	if (FAILURE == php_mongo_write_update(&buf, Z_STRVAL_P(c->ns), bit_opts, criteria, newobj, connection->max_bson_size, connection->max_message_size TSRMLS_CC)) {
		efree(buf.start);
		zval_ptr_dtor(&options);
		return;
	}

#if MONGO_PHP_STREAMS
	mongo_log_stream_update(connection, c->ns, criteria, newobj, options, bit_opts TSRMLS_CC);
#endif

	/* retval == -1 means a GLE response was received, so send_message() has
	 * either set return_value or thrown an exception via do_safe_op(). */
	retval = send_message(this_ptr, connection, &buf, options, return_value TSRMLS_CC);
	if (retval != -1) {
		RETVAL_BOOL(retval);
	}

	efree(buf.start);
	zval_ptr_dtor(&options);
}
/* }}} */

/* {{{ proto bool|array MongoCollection::remove([array|object criteria [array options]])
   Remove one or more documents matching $criteria */
PHP_METHOD(MongoCollection, remove)
{
	zval *criteria = 0, *options = 0;
	int bit_opts = 0;
	mongo_collection *c;
	mongo_connection *connection;
	buffer buf;
	int retval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|za/", &criteria, &options) == FAILURE) {
		return;
	}
	MUST_BE_ARRAY_OR_OBJECT(1, criteria);

	if (!criteria) {
		MAKE_STD_ZVAL(criteria);
		array_init(criteria);
	} else {
		zval_add_ref(&criteria);
	}

	if (options) {
		zval **just_one = NULL;

		if (zend_hash_find(HASH_P(options), "justOne", strlen("justOne") + 1, (void**)&just_one) == SUCCESS) {
			convert_to_boolean_ex(just_one);
			bit_opts = Z_BVAL_PP(just_one) << 0;
		}
		Z_ADDREF_P(options);
	} else {
		MAKE_STD_ZVAL(options);
		array_init(options);
	}

	PHP_MONGO_GET_COLLECTION(getThis());

	if ((connection = get_server(c, MONGO_CON_FLAG_WRITE TSRMLS_CC)) == 0) {
		zval_ptr_dtor(&options);
		RETURN_FALSE;
	}

	CREATE_BUF(buf, INITIAL_BUF_SIZE);
	if (FAILURE == php_mongo_write_delete(&buf, Z_STRVAL_P(c->ns), bit_opts, criteria, connection->max_bson_size, connection->max_message_size TSRMLS_CC)) {
		efree(buf.start);
		zval_ptr_dtor(&criteria);
		zval_ptr_dtor(&options);
		return;
	}
#if MONGO_PHP_STREAMS
	mongo_log_stream_delete(connection, c->ns, criteria, options, bit_opts TSRMLS_CC);
#endif

	/* retval == -1 means a GLE response was received, so send_message() has
	 * either set return_value or thrown an exception via do_safe_op(). */
	retval = send_message(this_ptr, connection, &buf, options, return_value TSRMLS_CC);
	if (retval != -1) {
		RETVAL_BOOL(retval);
	}

	efree(buf.start);
	zval_ptr_dtor(&criteria);
	zval_ptr_dtor(&options);
}
/* }}} */

/* {{{ proto bool MongoCollection::ensureIndex(mixed keys [, array options])
   Create the $keys index if it does not already exist */
PHP_METHOD(MongoCollection, ensureIndex)
{
	zval *keys, *options = 0, *db, *system_indexes, *collection, *data, *key_str;
	mongo_collection *c;
	zend_bool done_name = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &keys, &options) == FAILURE) {
		return;
	}

	if (IS_SCALAR_P(keys)) {
		zval *key_array;

		convert_to_string(keys);

		if (Z_STRLEN_P(keys) == 0) {
			return;
		}

		MAKE_STD_ZVAL(key_array);
		array_init(key_array);
		add_assoc_long(key_array, Z_STRVAL_P(keys), 1);

		keys = key_array;
	} else {
		zval_add_ref(&keys);
	}

	PHP_MONGO_GET_COLLECTION(getThis());

	/* get the system.indexes collection */
	db = c->parent;

	MAKE_STD_ZVAL(system_indexes);
	ZVAL_STRING(system_indexes, "system.indexes", 1);

	MAKE_STD_ZVAL(collection);
	MONGO_METHOD1(MongoDB, selectCollection, collection, db, system_indexes);
	PHP_MONGO_CHECK_EXCEPTION3(&keys, &system_indexes, &collection);

	/* set up data */
	MAKE_STD_ZVAL(data);
	array_init(data);

	/* ns */
	add_assoc_zval(data, "ns", c->ns);
	zval_add_ref(&c->ns);
	add_assoc_zval(data, "key", keys);
	zval_add_ref(&keys);

	if (options) {
		zval temp, **safe_pp, **fsync_pp, **timeout_pp, **name;

		zend_hash_merge(HASH_P(data), HASH_P(options), (void (*)(void*))zval_add_ref, &temp, sizeof(zval*), 1);

		if (zend_hash_find(HASH_P(options), "safe", strlen("safe") + 1, (void**)&safe_pp) == SUCCESS) {
			zend_hash_del(HASH_P(data), "safe", strlen("safe") + 1);
		}
		if (zend_hash_find(HASH_P(options), "fsync", strlen("fsync") + 1, (void**)&fsync_pp) == SUCCESS) {
			zend_hash_del(HASH_P(data), "fsync", strlen("fsync") + 1);
		}
		if (zend_hash_find(HASH_P(options), "timeout", strlen("timeout") + 1, (void**)&timeout_pp) == SUCCESS) {
			zend_hash_del(HASH_P(data), "timeout", strlen("timeout") + 1);
		}

		if (zend_hash_find(HASH_P(options), "name", strlen("name") + 1, (void**)&name) == SUCCESS) {
			if (Z_TYPE_PP(name) == IS_STRING && Z_STRLEN_PP(name) > MAX_INDEX_NAME_LEN) {
				zval_ptr_dtor(&data);
				zend_throw_exception_ex(mongo_ce_Exception, 14 TSRMLS_CC, "index name too long: %d, max %d characters", Z_STRLEN_PP(name), MAX_INDEX_NAME_LEN);
				return;
			}
			done_name = 1;
		}
		zval_add_ref(&options);
	} else {
		zval *opts;

		MAKE_STD_ZVAL(opts);
		array_init(opts);
		options = opts;
	}

	if (!done_name) {
		/* turn keys into a string */
		MAKE_STD_ZVAL(key_str);
		ZVAL_NULL(key_str);

		MONGO_METHOD1(MongoCollection, toIndexString, key_str, NULL, keys);

		if (Z_STRLEN_P(key_str) > MAX_INDEX_NAME_LEN) {
			zval_ptr_dtor(&data);
			zend_throw_exception_ex(mongo_ce_Exception, 14 TSRMLS_CC, "index name too long: %d, max %d characters", Z_STRLEN_P(key_str), MAX_INDEX_NAME_LEN);
			zval_ptr_dtor(&key_str);
			zval_ptr_dtor(&options);
			return;
		}

		add_assoc_zval(data, "name", key_str);
		zval_add_ref(&key_str);
	}

	MONGO_METHOD2(MongoCollection, insert, return_value, collection, data, options);

	zval_ptr_dtor(&options);
	zval_ptr_dtor(&data);
	zval_ptr_dtor(&system_indexes);
	zval_ptr_dtor(&collection);
	zval_ptr_dtor(&keys);

	if (!done_name) {
		zval_ptr_dtor(&key_str);
	}
}
/* }}} */

/* {{{ proto array MongoCollection::deleteIndex(mixed keys)
   Remove the $keys index */
PHP_METHOD(MongoCollection, deleteIndex)
{
	zval *keys, *key_str, *data;
	mongo_collection *c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &keys) == FAILURE) {
		return;
	}

	MAKE_STD_ZVAL(key_str);
	MONGO_METHOD1(MongoCollection, toIndexString, key_str, NULL, keys);

	PHP_MONGO_GET_COLLECTION(getThis());

	MAKE_STD_ZVAL(data);
	array_init(data);
	add_assoc_zval(data, "deleteIndexes", c->name);
	zval_add_ref(&c->name);
	add_assoc_zval(data, "index", key_str);

	MONGO_CMD(return_value, c->parent);

	zval_ptr_dtor(&data);
}
/* }}} */

/* {{{ proto array MongoCollection::deleteIndex()
   Removes all indexes for this collection */
PHP_METHOD(MongoCollection, deleteIndexes)
{
	zval *data;
	mongo_collection *c;
	PHP_MONGO_GET_COLLECTION(getThis());

	MAKE_STD_ZVAL(data);
	array_init(data);

	add_assoc_string(data, "deleteIndexes", Z_STRVAL_P(c->name), 1);
	add_assoc_string(data, "index", "*", 1);

	MONGO_CMD(return_value, c->parent);

	zval_ptr_dtor(&data);
}
/* }}} */

/* {{{ proto MongoCollection::getIndexInfo()
   Get all indexes for this collection */
PHP_METHOD(MongoCollection, getIndexInfo)
{
	zval *collection, *i_str, *query, *cursor, *next;
	mongo_collection *c;
	PHP_MONGO_GET_COLLECTION(getThis());

	MAKE_STD_ZVAL(collection);

	MAKE_STD_ZVAL(i_str);
	ZVAL_STRING(i_str, "system.indexes", 1);
	MONGO_METHOD1(MongoDB, selectCollection, collection, c->parent, i_str);
	zval_ptr_dtor(&i_str);
	PHP_MONGO_CHECK_EXCEPTION1(&collection);

	MAKE_STD_ZVAL(query);
	array_init(query);
	add_assoc_string(query, "ns", Z_STRVAL_P(c->ns), 1);

	MAKE_STD_ZVAL(cursor);
	MONGO_METHOD1(MongoCollection, find, cursor, collection, query);
	PHP_MONGO_CHECK_EXCEPTION3(&collection, &query, &cursor);

	zval_ptr_dtor(&query);
	zval_ptr_dtor(&collection);

	array_init(return_value);

	MAKE_STD_ZVAL(next);
	MONGO_METHOD(MongoCursor, getNext, next, cursor);
	PHP_MONGO_CHECK_EXCEPTION2(&cursor, &next);

	while (Z_TYPE_P(next) != IS_NULL) {
		add_next_index_zval(return_value, next);

		MAKE_STD_ZVAL(next);
		MONGO_METHOD(MongoCursor, getNext, next, cursor);
		PHP_MONGO_CHECK_EXCEPTION2(&cursor, &next);
	}

	zval_ptr_dtor(&next);
	zval_ptr_dtor(&cursor);
}
/* }}} */

/* {{{ proto MongoCollection::count([array criteria [, int limit [, int skip]]])
   Count all documents matching $criteria with an optional limit and/or skip */
PHP_METHOD(MongoCollection, count)
{
	zval *response, *data, *query=0;
	long limit = 0, skip = 0;
	zval **n;
	mongo_collection *c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zll", &query, &limit, &skip) == FAILURE) {
		return;
	}

	PHP_MONGO_GET_COLLECTION(getThis());

	MAKE_STD_ZVAL(data);
	array_init(data);
	add_assoc_string(data, "count", Z_STRVAL_P(c->name), 1);
	if (query) {
		add_assoc_zval(data, "query", query);
		zval_add_ref(&query);
	}
	if (limit) {
		add_assoc_long(data, "limit", limit);
	}
	if (skip) {
		add_assoc_long(data, "skip", skip);
	}

	MAKE_STD_ZVAL(response);
	ZVAL_NULL(response);

	MONGO_CMD_WITH_RP(response, c->parent, c);

	zval_ptr_dtor(&data);

	if (EG(exception) || Z_TYPE_P(response) != IS_ARRAY) {
		zval_ptr_dtor(&response);
		return;
	}

	if (zend_hash_find(HASH_P(response), "n", 2, (void**)&n) == SUCCESS) {
		convert_to_long(*n);
		RETVAL_ZVAL(*n, 1, 0);
		zval_ptr_dtor(&response);
	} else {
		zval **errmsg;

		/* The command failed, try to find an error message */
		if (zend_hash_find(HASH_P(response), "errmsg", strlen("errmsg") + 1 , (void**)&errmsg) == SUCCESS) {
			zend_throw_exception_ex(mongo_ce_Exception, 20 TSRMLS_CC, "Cannot run command count(): %s", Z_STRVAL_PP(errmsg));
		} else {
			zend_throw_exception(mongo_ce_Exception, "Cannot run command count()", 20 TSRMLS_CC);
		}
		zval_ptr_dtor(&response);
	}
}
/* }}} */

/* {{{ proto mixed MongoCollection::save(array|object document [, array options])
   Saves $document to this collection. An upsert will be used if the document's
   _id is set; otherwise, it will be inserted. Return the database response if
   the write concern is >= 1. Otherwise, boolean true is returned if the
   document is not empty. */
PHP_METHOD(MongoCollection, save)
{
	zval *a, *options = NULL;
	zval **id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|a/", &a, &options) == FAILURE) {
		return;
	}
	MUST_BE_ARRAY_OR_OBJECT(1, a);

	if (!options) {
		MAKE_STD_ZVAL(options);
		array_init(options);
	} else {
		Z_ADDREF_P(options);
	}

	if (zend_hash_find(HASH_P(a), "_id", 4, (void**)&id) == SUCCESS) {
		zval *criteria;

		MAKE_STD_ZVAL(criteria);
		array_init(criteria);
		add_assoc_zval(criteria, "_id", *id);
		zval_add_ref(id);

		add_assoc_bool(options, "upsert", 1);

		Z_ADDREF_P(options);
		MONGO_METHOD3(MongoCollection, update, return_value, getThis(), criteria, a, options);

		zval_ptr_dtor(&criteria);
		zval_ptr_dtor(&options);
		return;
	}

	MONGO_METHOD2(MongoCollection, insert, return_value, getThis(), a, options);
	zval_ptr_dtor(&options);
}
/* }}} */

/* {{{ proto array MongoCollection::createDBRef(array dbref)
    Create a database reference object */
PHP_METHOD(MongoCollection, createDBRef)
{
	zval *obj;
	mongo_collection *c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &obj) == FAILURE) {
		return;
	}

	PHP_MONGO_GET_COLLECTION(getThis());
	MONGO_METHOD2(MongoDB, createDBRef, return_value, c->parent, c->name, obj);
}
/* }}} */

/* {{{ proto array MongoCollection::getDBRef(array dbref)
   Retrieves the document referenced by $dbref */
PHP_METHOD(MongoCollection, getDBRef)
{
	zval *ref;
	mongo_collection *c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &ref) == FAILURE) {
		return;
	}
	MUST_BE_ARRAY_OR_OBJECT(1, ref);

	PHP_MONGO_GET_COLLECTION(getThis());
	MONGO_METHOD2(MongoDBRef, get, return_value, NULL, c->parent, ref);
}
/* }}} */

static char *replace_dots(char *key, int key_len, char *position)
{
	int i;

	for (i = 0; i < key_len; i++) {
		if (key[i] == '.') {
			*(position)++ = '_';
		} else {
			*(position)++ = key[i];
		}
	}
	return position;
}

/* {{{ proto protected static string MongoCollection::toIndexString(array|string keys)
   Converts $keys to an identifying string for an index */
PHP_METHOD(MongoCollection, toIndexString)
{
	zval *zkeys;
	char *name, *position;
	int len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zkeys) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(zkeys) == IS_ARRAY || Z_TYPE_P(zkeys) == IS_OBJECT) {
		HashTable *hindex = HASH_P(zkeys);
		HashPosition pointer;
		zval **data;
		char *key;
		uint key_len, first = 1, key_type;
		ulong index;

		for (
			zend_hash_internal_pointer_reset_ex(hindex, &pointer);
			zend_hash_get_current_data_ex(hindex, (void**)&data, &pointer) == SUCCESS;
			zend_hash_move_forward_ex(hindex, &pointer)
		) {
			key_type = zend_hash_get_current_key_ex(hindex, &key, &key_len, &index, NO_DUP, &pointer);

			switch (key_type) {
				case HASH_KEY_IS_STRING: {
					len += key_len;

					if (Z_TYPE_PP(data) == IS_STRING) {
						len += Z_STRLEN_PP(data) + 1;
					} else {
						len += Z_LVAL_PP(data) == 1 ? 2 : 3;
					}

					break;
				}

				case HASH_KEY_IS_LONG:
					convert_to_string(*data);

					len += Z_STRLEN_PP(data);
					len += 2;
					break;

				default:
					continue;
			}
		}

		name = (char*)emalloc(len + 1);
		position = name;

		for (
			zend_hash_internal_pointer_reset_ex(hindex, &pointer);
			zend_hash_get_current_data_ex(hindex, (void**)&data, &pointer) == SUCCESS;
			zend_hash_move_forward_ex(hindex, &pointer)
		) {
			if (!first) {
				*(position)++ = '_';
			}
			first = 0;

			key_type = zend_hash_get_current_key_ex(hindex, &key, &key_len, &index, NO_DUP, &pointer);

			if (key_type == HASH_KEY_IS_LONG) {
				key_len = spprintf(&key, 0, "%ld", index);
				key_len += 1;
			}

			/* copy str, replacing '.' with '_' */
			position = replace_dots(key, key_len-1, position);

			*(position)++ = '_';

			if (Z_TYPE_PP(data) == IS_STRING) {
				memcpy(position, Z_STRVAL_PP(data), Z_STRLEN_PP(data));
				position += Z_STRLEN_PP(data);
			} else {
				if (Z_LVAL_PP(data) != 1) {
					*(position)++ = '-';
				}
				*(position)++ = '1';
			}

			if (key_type == HASH_KEY_IS_LONG) {
				efree(key);
			}
		}
		*(position) = 0;
	} else if (Z_TYPE_P(zkeys) == IS_STRING) {
		int len;

		convert_to_string(zkeys);

		len = Z_STRLEN_P(zkeys);

		name = (char*)emalloc(len + 3);
		position = name;

		/* copy str, replacing '.' with '_' */
		position = replace_dots(Z_STRVAL_P(zkeys), Z_STRLEN_P(zkeys), position);

		*(position)++ = '_';
		*(position)++ = '1';
		*(position) = '\0';
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The key needs to be either a string or an array");
		return;
	}
	RETURN_STRING(name, 0)
}
/* }}} */

/* {{{ proto array MongoCollection::aggregate(array pipeline, [, array op [, ...]])
   Wrapper for aggregate command. The pipeline may be specified as a single
   array of operations or a variable number of operation arguments. Returns the
   database response for the command. Aggregation results will be stored in the
   "result" key of the response. */
PHP_METHOD(MongoCollection, aggregate)
{
	zval ***argv, *pipeline, *data, *tmp;
	int argc, i;
	mongo_collection *c;

	zpp_var_args(argv, argc);

	c = (mongo_collection*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED(c->ns, MongoCollection);

	for (i = 0; i < argc; i++) {
		tmp = *argv[i];
		if (Z_TYPE_P(tmp) != IS_ARRAY) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Argument %d is not an array", i + 1);
			efree(argv);
			return;
		}
	}

	MAKE_STD_ZVAL(data);
	array_init(data);

	add_assoc_zval(data, "aggregate", c->name);
	zval_add_ref(&c->name);

	/* If the single array argument contains a zeroth index, consider it an
	 * array of pipeline operators. Otherwise, assume it is a single pipeline
	 * operator and allow it to be wrapped in an array. */
	if (argc == 1 && zend_hash_index_exists(Z_ARRVAL_PP(argv[0]), 0)) {
		Z_ADDREF_PP(*argv);
		add_assoc_zval(data, "pipeline", **argv);
	} else {
		MAKE_STD_ZVAL(pipeline);
		array_init(pipeline);

		for (i = 0; i < argc; i++) {
			tmp = *argv[i];
			Z_ADDREF_P(tmp);
			if (zend_hash_next_index_insert(Z_ARRVAL_P(pipeline), &tmp, sizeof(zval*), NULL) == FAILURE) {
				Z_DELREF_P(tmp);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot create pipeline array");
				efree(argv);
				RETURN_FALSE;
			}
		}
		add_assoc_zval(data, "pipeline", pipeline);
	}
	efree(argv);

	MONGO_CMD_WITH_RP(return_value, c->parent, c);

	zval_ptr_dtor(&data);
}
/* }}} */


/* {{{ proto array MongoCollection::distinct(string key [, array query])
   Wrapper for distinct command. Returns a list of distinct values for the given
   key across a collection. An optional $query may be applied to filter the
   documents considered. */
PHP_METHOD(MongoCollection, distinct)
{
	char *key;
	int key_len;
	zval *data, **values, *tmp, *query = NULL;
	mongo_collection *c;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a!", &key, &key_len, &query) == FAILURE) {
		return;
	}

	c = (mongo_collection*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED(c->ns, MongoCollection);

	MAKE_STD_ZVAL(data);
	array_init(data);

	add_assoc_zval(data, "distinct", c->name);
	zval_add_ref(&c->name);
	add_assoc_stringl(data, "key", key, key_len, 1);

	if (query) {
		add_assoc_zval(data, "query", query);
		zval_add_ref(&query);
	}

	MAKE_STD_ZVAL(tmp);
	MONGO_CMD_WITH_RP(tmp, c->parent, c);

	if (zend_hash_find(Z_ARRVAL_P(tmp), "values", strlen("values") + 1, (void **)&values) == SUCCESS) {
#ifdef array_init_size
		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL_PP(values)));
#else
		array_init(return_value);
#endif
		zend_hash_copy(Z_ARRVAL_P(return_value), Z_ARRVAL_PP(values), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));
	} else {
		RETVAL_FALSE;
	}

	zval_ptr_dtor(&data);
	zval_ptr_dtor(&tmp);
}
/* }}} */

/* {{{ proto array MongoCollection::group(mixed keys, array initial, MongoCode reduce [, array options])
   Wrapper for group command. Returns the database response for the command.
   Aggregation results will be stored in the "retval" key of the response. */
PHP_METHOD(MongoCollection, group)
{
	zval *key, *initial, *options = 0, *group, *data, *reduce;
	mongo_collection *c = (mongo_collection*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED(c->ns, MongoCollection);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz|z", &key, &initial, &reduce, &options) == FAILURE) {
		return;
	}
	MUST_BE_ARRAY_OR_OBJECT(4, options);

	if (Z_TYPE_P(reduce) == IS_STRING) {
		zval *code;
		MAKE_STD_ZVAL(code);
		object_init_ex(code, mongo_ce_Code);

		MONGO_METHOD1(MongoCode, __construct, return_value, code, reduce);

		reduce = code;
	} else {
		zval_add_ref(&reduce);
	}

	MAKE_STD_ZVAL(group);
	array_init(group);
	add_assoc_zval(group, "ns", c->name);
	zval_add_ref(&c->name);
	add_assoc_zval(group, "$reduce", reduce);
	zval_add_ref(&reduce);

	if (Z_TYPE_P(key) == IS_OBJECT && Z_OBJCE_P(key) == mongo_ce_Code) {
		add_assoc_zval(group, "$keyf", key);
	} else if (!IS_SCALAR_P(key)) {
		add_assoc_zval(group, "key", key);
	} else {
		zval_ptr_dtor(&group);
		zval_ptr_dtor(&reduce);
		zend_throw_exception(mongo_ce_Exception, "MongoCollection::group takes an array, object, or MongoCode key", 0 TSRMLS_CC);
		return;
	}
	zval_add_ref(&key);

	/* options used to just be "condition" but now can be "condition" or
	 * "finalize" */
	if (options) {
		zval **condition = 0, **finalize = 0;

		/* new case */
		if (zend_hash_find(HASH_P(options), "condition", strlen("condition") + 1, (void**)&condition) == SUCCESS) {
			add_assoc_zval(group, "cond", *condition);
			zval_add_ref(condition);
		}
		if (zend_hash_find(HASH_P(options), "finalize", strlen("finalize") + 1, (void**)&finalize) == SUCCESS) {
			add_assoc_zval(group, "finalize", *finalize);
			zval_add_ref(finalize);
		}
		if (!condition && !finalize) {
			php_error_docref(NULL TSRMLS_CC, MONGO_E_DEPRECATED, "Implicitly passing condition as $options will be removed in the future");
			add_assoc_zval(group, "cond", options);
			zval_add_ref(&options);
		}
	}

	add_assoc_zval(group, "initial", initial);
	zval_add_ref(&initial);

	MAKE_STD_ZVAL(data);
	array_init(data);
	add_assoc_zval(data, "group", group);

	MONGO_CMD_WITH_RP(return_value, c->parent, c);

	zval_ptr_dtor(&data);
	zval_ptr_dtor(&reduce);
}
/* }}} */

/* {{{ proto MongoCollection MongoCollection::__get(string name)
   Appends this collection name with a period and $name and returns a new
   MongoCollection for the combined string. This is used to allow for concisely
   selecting collections with dotted names. */
PHP_METHOD(MongoCollection, __get)
{
	/* This is a little trickier than the getters in Mongo and MongoDB... we
	 * need to combine the current collection name with the parameter passed
	 * in, get the parent db, then select the new collection from it. */
	zval *full_name;
	char *full_name_s, *name;
	int name_len;
	mongo_collection *c;
	PHP_MONGO_GET_COLLECTION(getThis());

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		return;
	}

	/* If this is "db", return the parent database.  This can't actually be a
	 * property of the obj because apache does weird things on object
	 * destruction that will cause the link to be destroyed twice. */
	if (strcmp(name, "db") == 0) {
		RETURN_ZVAL(c->parent, 1, 0);
	}

	spprintf(&full_name_s, 0, "%s.%s", Z_STRVAL_P(c->name), name);
	MAKE_STD_ZVAL(full_name);
	ZVAL_STRING(full_name, full_name_s, 0);

	/* select this collection */
	MONGO_METHOD1(MongoDB, selectCollection, return_value, c->parent, full_name);

	zval_ptr_dtor(&full_name);
}
/* }}} */


MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo___construct, 0, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_OBJ_INFO(0, database, MongoDB, 0)
	ZEND_ARG_INFO(0, collection_name)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_distinct, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, query)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_no_parameters, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo___get, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_setSlaveOkay, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, slave_okay)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_setReadPreference, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, read_preference)
	ZEND_ARG_ARRAY_INFO(0, tags, 0) /* Yes, this should be an array */
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_validate, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, validate)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_insert, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, array_of_fields_OR_object)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_batchInsert, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_ARRAY_INFO(0, documents, 0) /* Array of documents */
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_find, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, fields)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_find_one, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, fields)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_findandmodify, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_ARRAY_INFO(0, query, 1)
	ZEND_ARG_ARRAY_INFO(0, update, 1)
	ZEND_ARG_ARRAY_INFO(0, fields, 1)
	ZEND_ARG_ARRAY_INFO(0, options, 1)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_update, 0, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, old_array_of_fields_OR_object)
	ZEND_ARG_INFO(0, new_array_of_fields_OR_object)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_remove, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, array_of_fields_OR_object)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_ensureIndex, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, key_OR_array_of_keys)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_deleteIndex, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, string_OR_array_of_keys)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_count, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, query_AS_array_of_fields_OR_object)
	ZEND_ARG_INFO(0, limit)
	ZEND_ARG_INFO(0, skip)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_createDBRef, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, array_with_id_fields_OR_MongoID)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_getDBRef, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, reference)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_toIndexString, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, string_OR_array_of_keys)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_group, 0, ZEND_RETURN_VALUE, 3)
	ZEND_ARG_INFO(0, keys_or_MongoCode)
	ZEND_ARG_INFO(0, initial_value)
	ZEND_ARG_INFO(0, array_OR_MongoCode)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_aggregate, 0, 0, 1)
	ZEND_ARG_INFO(0, pipeline)
	ZEND_ARG_INFO(0, op)
	ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

static zend_function_entry MongoCollection_methods[] = {
	PHP_ME(MongoCollection, __construct, arginfo___construct, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, __toString, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, getName, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, getSlaveOkay, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(MongoCollection, setSlaveOkay, arginfo_setSlaveOkay, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(MongoCollection, getReadPreference, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, setReadPreference, arginfo_setReadPreference, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, drop, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, validate, arginfo_validate, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, insert, arginfo_insert, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, batchInsert, arginfo_batchInsert, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, update, arginfo_update, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, remove, arginfo_remove, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, find, arginfo_find, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, findOne, arginfo_find_one, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, findAndModify, arginfo_findandmodify, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, ensureIndex, arginfo_ensureIndex, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, deleteIndex, arginfo_deleteIndex, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, deleteIndexes, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, getIndexInfo, arginfo_no_parameters, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, count, arginfo_count, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, save, arginfo_insert, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, createDBRef, arginfo_createDBRef, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, getDBRef, arginfo_getDBRef, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, toIndexString, arginfo_toIndexString, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC)
	PHP_ME(MongoCollection, group, arginfo_group, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, distinct, arginfo_distinct, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCollection, aggregate, arginfo_aggregate, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

static void php_mongo_collection_free(void *object TSRMLS_DC)
{
	mongo_collection *c = (mongo_collection*)object;

	if (c) {
		if (c->parent) {
			zval_ptr_dtor(&c->parent);
		}
		if (c->link) {
			zval_ptr_dtor(&c->link);
		}
		if (c->name) {
			zval_ptr_dtor(&c->name);
		}
		if (c->ns) {
			zval_ptr_dtor(&c->ns);
		}
		mongo_read_preference_dtor(&c->read_pref);
		zend_object_std_dtor(&c->std TSRMLS_CC);
		efree(c);
	}
}

static int php_mongo_trigger_error_on_command_failure(zval *document TSRMLS_DC)
{
	zval **tmpvalue;

	if (Z_TYPE_P(document) != IS_ARRAY) {
		zend_throw_exception(mongo_ce_ResultException, strdup("Unknown error executing command (empty document returned)"), 0 TSRMLS_CC);
		return FAILURE;
	}

	if (zend_hash_find(Z_ARRVAL_P(document), "ok", strlen("ok") + 1, (void **) &tmpvalue) == SUCCESS) {
		if ((Z_TYPE_PP(tmpvalue) == IS_LONG && Z_LVAL_PP(tmpvalue) < 1) || (Z_TYPE_PP(tmpvalue) == IS_DOUBLE && Z_DVAL_PP(tmpvalue) < 1)) {
			zval **tmp, *exception;
			char *message;
			long code;

			if (zend_hash_find(Z_ARRVAL_P(document), "errmsg", strlen("errmsg") + 1, (void **) &tmp) == SUCCESS) {
				convert_to_string_ex(tmp);
				message = Z_STRVAL_PP(tmp);
			} else {
				message = strdup("Unknown error executing command");
			}

			if (zend_hash_find(Z_ARRVAL_P(document), "code", strlen("code") + 1, (void **) &tmp) == SUCCESS) {
				convert_to_long_ex(tmp);
				code = Z_LVAL_PP(tmp);
			} else {
				code = 0;
			}

			exception = zend_throw_exception(mongo_ce_ResultException, message, code TSRMLS_CC);
			zend_update_property(mongo_ce_Exception, exception, "document", strlen("document"), document TSRMLS_CC);

			return FAILURE;
		}
	}
	return SUCCESS;
}


/* {{{ php_mongo_collection_new
 */
zend_object_value php_mongo_collection_new(zend_class_entry *class_type TSRMLS_DC) {
	PHP_MONGO_OBJ_NEW(mongo_collection);
}
/* }}} */

void mongo_init_MongoCollection(TSRMLS_D) {
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoCollection", MongoCollection_methods);
	ce.create_object = php_mongo_collection_new;
	mongo_ce_Collection = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_class_constant_long(mongo_ce_Collection, "ASCENDING", strlen("ASCENDING"), 1 TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_Collection, "DESCENDING", strlen("DESCENDING"), -1 TSRMLS_CC);

	zend_declare_property_long(mongo_ce_Collection, "w", strlen("w"), 1, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(mongo_ce_Collection, "wtimeout", strlen("wtimeout"), PHP_MONGO_DEFAULT_WTIMEOUT, ZEND_ACC_PUBLIC TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
