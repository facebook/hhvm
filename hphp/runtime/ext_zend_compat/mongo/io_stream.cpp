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

#include <php.h>
#include <main/php_streams.h>
#include <main/php_network.h>


void* php_mongo_io_stream_connect(mongo_con_manager *manager, mongo_server_def *server, mongo_server_options *options, char **error_message)
{
	char *errmsg;
	int errcode;
	php_stream *stream;
	char *hash = mongo_server_create_hash(server);
	struct timeval ctimeout = {0};
	char *dsn;
	int dsn_len;
	TSRMLS_FETCH();

	if (server->host[0] == '/') {
		dsn_len = spprintf(&dsn, 0, "unix://%s", server->host);
	} else {
		dsn_len = spprintf(&dsn, 0, "tcp://%s:%d", server->host, server->port);
	}


	if (options->connectTimeoutMS) {
		ctimeout.tv_sec = options->connectTimeoutMS / 1000;
		ctimeout.tv_usec = (options->connectTimeoutMS % 1000) * 1000;
	}

	stream = php_stream_xport_create(dsn, dsn_len, 0, STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT, hash, options->connectTimeoutMS ? &ctimeout : NULL, (php_stream_context *)options->ctx, &errmsg, &errcode);
	efree(dsn);
	free(hash);

	if (!stream) {
		/* error_message will be free()d, but errmsg was allocated by PHP and needs efree() */
		*error_message = strdup(errmsg);
		efree(errmsg);
		return NULL;
	}

	if (options->ssl) {
		if (php_stream_xport_crypto_setup(stream, STREAM_CRYPTO_METHOD_TLS_CLIENT, NULL TSRMLS_CC) < 0) {
			*error_message = strdup("Cannot setup SSL, is ext/openssl loaded?");
			php_stream_close(stream);
			return NULL;
		}
		if (php_stream_xport_crypto_enable(stream, 1 TSRMLS_CC) < 0) {
			/* Setting up crypto failed. Thats only OK if we only preferred it */
			if (options->ssl == MONGO_SSL_PREFER) {
				/* FIXME: We can't actually get here because we reject setting
				 * this option to prefer in mcon/parse.c. This is however
				 * probably what we need to do in the future when mongod starts
				 * actually supporting this! :) */
				mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "stream_connect: Failed establishing SSL for %s:%d", server->host, server->port);
				php_stream_xport_crypto_enable(stream, 0 TSRMLS_CC);
			} else {
				*error_message = strdup("Can't connect over SSL, is mongod running with SSL?");
				php_stream_close(stream);
				return NULL;
			}
		} else {
			mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "stream_connect: Establish SSL for %s:%d", server->host, server->port);
		}
	} else {
		mongo_manager_log(manager, MLOG_CON, MLOG_INFO, "stream_connect: Not establishing SSL for %s:%d", server->host, server->port);
	}

	php_stream_notify_progress_init(stream->context, 0, 0);

	if (options->socketTimeoutMS) {
		struct timeval rtimeout = {0};
		rtimeout.tv_sec = options->socketTimeoutMS / 1000;
		rtimeout.tv_usec = (options->socketTimeoutMS % 1000) * 1000;
		php_stream_set_option(stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &rtimeout);
	}


	/* Avoid a weird leak warning in debug mode when freeing the stream */
#if ZEND_DEBUG
	stream->__exposed = 1;
#endif
	return stream;

}

/* Returns the bytes read on success
 * Returns -31 on unknown failure
 * Returns -80 on timeout
 * Returns -32 when remote server closes the connection
 */
int php_mongo_io_stream_read(mongo_connection *con, mongo_server_options *options, int timeout, void *data, int size, char **error_message)
{
	int num = 1, received = 0;
	TSRMLS_FETCH();

	if (timeout > 0 && options->socketTimeoutMS != timeout) {
		struct timeval rtimeout = {0};
		rtimeout.tv_sec = timeout / 1000;
		rtimeout.tv_usec = (timeout % 1000) * 1000;

		php_stream_set_option((php_stream*) con->socket, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &rtimeout);
	}

	/* this can return FAILED if there is just no more data from db */
	while (received < size && num > 0) {
		int len = 4096 < (size - received) ? 4096 : size - received;

		num = php_stream_read((php_stream*) con->socket, (char *) data, len);

		if (num < 0) {
			/* Doesn't look like this can happen, php_sockop_read overwrites
			 * the failure from recv() to return 0 */
			*error_message = strdup("Read from socket failed");
			return -31;
		}

		/* It *may* have failed. It also may simply have no data */
		if (num == 0) {
			zval *metadata;

			MAKE_STD_ZVAL(metadata);
			array_init(metadata);
			if (php_stream_populate_meta_data((php_stream*) con->socket, metadata)) {
				zval **tmp;

				if (zend_hash_find(Z_ARRVAL_P(metadata), "timed_out", sizeof("timed_out"), (void**)&tmp) == SUCCESS) {
					convert_to_boolean_ex(tmp);
					if (Z_BVAL_PP(tmp)) {
						struct timeval rtimeout = {0};

						if (timeout > 0 && options->socketTimeoutMS != timeout) {
							rtimeout.tv_sec = timeout / 1000;
							rtimeout.tv_usec = (timeout % 1000) * 1000;
						} else {
							rtimeout.tv_sec = options->socketTimeoutMS / 1000;
							rtimeout.tv_usec = (options->socketTimeoutMS % 1000) * 1000;
						}
						*error_message = (char*) malloc(256);
						snprintf(*error_message, 256, "Read timed out after reading %d bytes, waited for %d.%06d seconds", num, rtimeout.tv_sec, rtimeout.tv_usec);
						zval_ptr_dtor(&metadata);
						return -80;
					}
				}
				if (zend_hash_find(Z_ARRVAL_P(metadata), "eof", sizeof("eof"), (void**)&tmp) == SUCCESS) {
					convert_to_boolean_ex(tmp);
					if (Z_BVAL_PP(tmp)) {
						*error_message = strdup("Remote server has closed the connection");
						zval_ptr_dtor(&metadata);
						return -32;
					}
				}
			}
			zval_ptr_dtor(&metadata);
		}

		data = (char*)data + num;
		received += num;
	}

	if (options && options->ctx) {
		php_stream_notify_progress_increment((php_stream_context *)options->ctx, received, size);
	}
	if (timeout > 0 && options->socketTimeoutMS != timeout) {
		struct timeval rtimeout = {0};
		rtimeout.tv_sec = options->socketTimeoutMS / 1000;
		rtimeout.tv_usec = (options->socketTimeoutMS % 1000) * 1000;

		php_stream_set_option((php_stream*) con->socket, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &rtimeout);
	}

	return received;
}

int php_mongo_io_stream_send(mongo_connection *con, mongo_server_options *options, void *data, int size, char **error_message)
{
	int retval;
	TSRMLS_FETCH();

	retval = php_stream_write((php_stream*) con->socket, (char *) data, size);

	return retval;
}

void php_mongo_io_stream_close(mongo_connection *con, int why)
{
	TSRMLS_FETCH();

	if (why == MONGO_CLOSE_BROKEN) {
		if (con->socket) {
			php_stream_free((php_stream*) con->socket, PHP_STREAM_FREE_CLOSE_PERSISTENT | PHP_STREAM_FREE_RSRC_DTOR);
		}
	} else if (why == MONGO_CLOSE_SHUTDOWN) {
		/* No need to do anything, it was freed from the persistent_list */
	}
}

void php_mongo_io_stream_forget(mongo_con_manager *manager, mongo_connection *con)
{
	zend_rsrc_list_entry *le;
	// TSRMLS_FETCH();

	/* When we fork we need to unregister the parents hash so we don't
	 * accidentally destroy it */
	if (zend_hash_find(&EG(persistent_list), con->hash, strlen(con->hash) + 1, (void**) &le) == SUCCESS) {
		((php_stream *)con->socket)->in_free = 1;
		zend_hash_del(&EG(persistent_list), con->hash, strlen(con->hash) + 1);
		((php_stream *)con->socket)->in_free = 0;
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
