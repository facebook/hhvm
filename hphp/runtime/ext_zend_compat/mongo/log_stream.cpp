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

#include "io_stream.h"
#include "mcon/types.h"
#include "mcon/utils.h"
#include "mcon/manager.h"
#include "php_mongo.h"
#include "bson.h"

#include <php.h>
#include <main/php_streams.h>
#include <main/php_network.h>

zval *php_log_get_server_info(mongo_connection *connection)
{
	zval *retval;
	MAKE_STD_ZVAL(retval);
	array_init(retval);

	add_assoc_string(retval, "hash", connection->hash, 1);
	add_assoc_long(retval, "type", connection->connection_type);
	add_assoc_long(retval, "max_bson_size", connection->max_bson_size);
	add_assoc_long(retval, "max_message_size", connection->max_message_size);
	add_assoc_long(retval, "request_id", connection->last_reqid);

	return retval;
}

void mongo_log_stream_insert(mongo_connection *connection, zval *document, zval *options TSRMLS_DC)
{
	zval **callback;
	php_stream_context *context = ((php_stream *)connection->socket)->context;

	if (context && php_stream_context_get_option(context, "mongodb", "log_insert", &callback) == SUCCESS) {
		zval **args[3];
		zval *server;
		zval *retval = NULL;
		int   free_options = 0;

		server = php_log_get_server_info(connection);

		args[0] = &server;
		args[1] = &document;
		if (!options) {
			free_options = 1;
			MAKE_STD_ZVAL(options);
			ZVAL_NULL(options);
		}
		args[2] = &options;

		if (FAILURE == call_user_function_ex(EG(function_table), NULL, *callback, &retval, 3, args, 0, NULL TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to call stream context callback function 'log_insert' for 'mongodb' context option");
		}

		if (retval) {
			zval_ptr_dtor(&retval);
		}
		zval_ptr_dtor(args[0]);
		if (free_options) {
			zval_ptr_dtor(args[2]);
		}
	}
}

void mongo_log_stream_query(mongo_connection *connection, mongo_cursor *cursor TSRMLS_DC)
{
	zval **callback;
	php_stream_context *context = ((php_stream *)connection->socket)->context;

	if (context && php_stream_context_get_option(context, "mongodb", "log_query", &callback) == SUCCESS) {
		zval **args[3];
		zval *server, *info;
		zval *retval = NULL;

		server = php_log_get_server_info(connection);

		MAKE_STD_ZVAL(info);
		array_init(info);

		add_assoc_long(info, "request_id", cursor->send.request_id);
		add_assoc_long(info, "skip", cursor->skip);
		add_assoc_long(info, "limit", mongo_get_limit(cursor));
		add_assoc_long(info, "options", cursor->opts);
		add_assoc_long(info, "cursor_id", cursor->cursor_id);

		args[0] = &server;
		args[1] = &cursor->query;
		args[2] = &info;


		if (FAILURE == call_user_function_ex(EG(function_table), NULL, *callback, &retval, 3, args, 0, NULL TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to call stream context callback function 'log_query' for 'mongodb' context option");
		}

		if (retval) {
			zval_ptr_dtor(&retval);
		}
		zval_ptr_dtor(args[0]);
		zval_ptr_dtor(&info);
	}
}

void mongo_log_stream_update(mongo_connection *connection, zval *ns, zval *criteria, zval *newobj, zval *options, int flags TSRMLS_DC)
{
	zval **callback;
	php_stream_context *context = ((php_stream *)connection->socket)->context;

	if (context && php_stream_context_get_option(context, "mongodb", "log_update", &callback) == SUCCESS) {
		zval **args[5];
		zval *server, *info;
		zval *retval = NULL;

		server = php_log_get_server_info(connection);

		MAKE_STD_ZVAL(info);
		array_init(info);

		add_assoc_stringl(info, "namespace", Z_STRVAL_P(ns), Z_STRLEN_P(ns), 1);
		add_assoc_long(info, "flags", flags);

		args[0] = &server;
		args[1] = &criteria;
		args[2] = &newobj;
		args[3] = &options;
		args[4] = &info;


		if (FAILURE == call_user_function_ex(EG(function_table), NULL, *callback, &retval, 5, args, 0, NULL TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to call stream context callback function 'log_update' for 'mongodb' context option");
		}

		if (retval) {
			zval_ptr_dtor(&retval);
		}
		zval_ptr_dtor(args[0]);
		zval_ptr_dtor(&info);
	}
}

void mongo_log_stream_delete(mongo_connection *connection, zval *ns, zval *criteria, zval *options, int flags TSRMLS_DC)
{
	zval **callback;
	php_stream_context *context = ((php_stream *)connection->socket)->context;

	if (context && php_stream_context_get_option(context, "mongodb", "log_delete", &callback) == SUCCESS) {
		zval **args[4];
		zval *server, *info;
		zval *retval = NULL;

		server = php_log_get_server_info(connection);

		MAKE_STD_ZVAL(info);
		array_init(info);

		add_assoc_stringl(info, "namespace", Z_STRVAL_P(ns), Z_STRLEN_P(ns), 1);
		add_assoc_long(info, "flags", flags);

		args[0] = &server;
		args[1] = &criteria;
		args[2] = &options;
		args[3] = &info;


		if (FAILURE == call_user_function_ex(EG(function_table), NULL, *callback, &retval, 4, args, 0, NULL TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to call stream context callback function 'log_delete' for 'mongodb' context option");
		}

		if (retval) {
			zval_ptr_dtor(&retval);
		}
		zval_ptr_dtor(args[0]);
		zval_ptr_dtor(&info);
	}
}

void mongo_log_stream_getmore(mongo_connection *connection, mongo_cursor *cursor TSRMLS_DC)
{
	zval **callback;
	php_stream_context *context = ((php_stream *)connection->socket)->context;

	if (context && php_stream_context_get_option(context, "mongodb", "log_getmore", &callback) == SUCCESS) {
		zval **args[2];
		zval *server, *info;
		zval *retval = NULL;

		server = php_log_get_server_info(connection);

		MAKE_STD_ZVAL(info);
		array_init(info);

		add_assoc_long(info, "request_id", cursor->recv.request_id);
		add_assoc_long(info, "cursor_id", cursor->cursor_id);

		args[0] = &server;
		args[1] = &info;


		if (FAILURE == call_user_function_ex(EG(function_table), NULL, *callback, &retval, 2, args, 0, NULL TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to call stream context callback function 'log_getmore' for 'mongodb' context option");
		}

		if (retval) {
			zval_ptr_dtor(&retval);
		}
		zval_ptr_dtor(args[0]);
		zval_ptr_dtor(&info);
	}
}

void mongo_log_stream_killcursor(mongo_connection *connection, int64_t cursor_id TSRMLS_DC)
{
	zval **callback;
	php_stream_context *context = ((php_stream *)connection->socket)->context;

	if (context && php_stream_context_get_option(context, "mongodb", "log_killcursor", &callback) == SUCCESS) {
		zval **args[2];
		zval *server, *info;
		zval *retval = NULL;

		server = php_log_get_server_info(connection);

		MAKE_STD_ZVAL(info);
		array_init(info);

		add_assoc_long(info, "cursor_id", cursor_id);

		args[0] = &server;
		args[1] = &info;


		if (FAILURE == call_user_function_ex(EG(function_table), NULL, *callback, &retval, 2, args, 0, NULL TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to call stream context callback function 'log_killcursor' for 'mongodb' context option");
		}

		if (retval) {
			zval_ptr_dtor(&retval);
		}
		zval_ptr_dtor(args[0]);
		zval_ptr_dtor(&info);
	}
}

void mongo_log_stream_batchinsert(mongo_connection *connection, zval *docs, zval *options, int flags TSRMLS_DC)
{
	zval **callback;
	php_stream_context *context = ((php_stream *)connection->socket)->context;

	if (context && php_stream_context_get_option(context, "mongodb", "log_batchinsert", &callback) == SUCCESS) {
		zval **args[4];
		zval *server, *info;
		zval *retval = NULL;

		server = php_log_get_server_info(connection);

		MAKE_STD_ZVAL(info);
		array_init(info);

		add_assoc_long(info, "flags", flags);


		args[0] = &server;
		args[1] = &docs;
		args[2] = &info;
		args[3] = &options;

		if (FAILURE == call_user_function_ex(EG(function_table), NULL, *callback, &retval, 4, args, 0, NULL TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to call stream context callback function 'log_batchinsert' for 'mongodb' context option");
		}

		if (retval) {
			zval_ptr_dtor(&retval);
		}
		zval_ptr_dtor(args[0]);
		zval_ptr_dtor(&info);
	}
}

void mongo_log_stream_response_header(mongo_connection *connection, mongo_cursor *cursor TSRMLS_DC)
{
	zval **callback;
	php_stream_context *context = ((php_stream *)connection->socket)->context;

	if (context && php_stream_context_get_option(context, "mongodb", "log_response_header", &callback) == SUCCESS) {
		zval **args[3];
		zval *server, *info;
		zval *retval = NULL;

		server = php_log_get_server_info(connection);

		MAKE_STD_ZVAL(info);
		array_init(info);

		add_assoc_long(info, "send_request_id", cursor->send.request_id);
		add_assoc_long(info, "cursor_id", cursor->cursor_id);
		add_assoc_long(info, "recv_request_id", cursor->recv.request_id);
		add_assoc_long(info, "recv_response_to", cursor->recv.response_to);
		add_assoc_long(info, "recv_opcode", cursor->recv.op);
		add_assoc_long(info, "flag", cursor->flag);
		add_assoc_long(info, "start", cursor->start);
		/*add_assoc_long(info, "global_response_num", MonGlo(response_num)); */

		args[0] = &server;
		args[1] = &cursor->query;
		args[2] = &info;


		if (FAILURE == call_user_function_ex(EG(function_table), NULL, *callback, &retval, 3, args, 0, NULL TSRMLS_CC)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to call stream context callback function 'log_response_header' for 'mongodb' context option");
		}

		if (retval) {
			zval_ptr_dtor(&retval);
		}
		zval_ptr_dtor(args[0]);
		zval_ptr_dtor(&info);
	}
}

