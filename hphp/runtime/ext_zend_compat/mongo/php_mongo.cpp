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
#include <php_ini.h>
#include <ext/standard/info.h>

#include "php_mongo.h"

#include "mongoclient.h"
#include "mongo.h"
#include "cursor.h"
#include "io_stream.h"

#include "exceptions/exception.h"
#include "exceptions/connection_exception.h"
#include "exceptions/cursor_exception.h"
#include "exceptions/cursor_timeout_exception.h"
#include "exceptions/gridfs_exception.h"
#include "exceptions/result_exception.h"

#include "types/id.h"

#include "util/log.h"
#include "util/pool.h"

#include "mcon/manager.h"

extern zend_object_handlers mongo_default_handlers, mongo_id_handlers;

/** Classes */
extern zend_class_entry *mongo_ce_CursorException, *mongo_ce_ResultException;
extern zend_class_entry *mongo_ce_ConnectionException, *mongo_ce_Exception;
extern zend_class_entry *mongo_ce_GridFSException;

zend_class_entry *mongo_ce_MaxKey, *mongo_ce_MinKey;

/** Resources */
int le_cursor_list;

static void mongo_init_MongoExceptions(TSRMLS_D);

ZEND_DECLARE_MODULE_GLOBALS(mongo)

static PHP_GINIT_FUNCTION(mongo);
static PHP_GSHUTDOWN_FUNCTION(mongo);

#if WIN32
extern HANDLE cursor_mutex;
#endif

zend_function_entry mongo_functions[] = {
	PHP_FE(bson_encode, NULL)
	PHP_FE(bson_decode, NULL)
	{ NULL, NULL, NULL }
};

/* {{{ mongo_module_entry
 */
static const zend_module_dep mongo_deps[] = {
	ZEND_MOD_OPTIONAL("openssl")
#if PHP_VERSION_ID >= 50307
	ZEND_MOD_END
#else /* pre-5.3.7 */
	{ NULL, NULL, NULL, 0 }
#endif
};
zend_module_entry mongo_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	mongo_deps,
	PHP_MONGO_EXTNAME,
	mongo_functions,
	PHP_MINIT(mongo),
	PHP_MSHUTDOWN(mongo),
	PHP_RINIT(mongo),
	NULL,
	PHP_MINFO(mongo),
	PHP_MONGO_VERSION,
	PHP_MODULE_GLOBALS(mongo),
	PHP_GINIT(mongo),
	PHP_GSHUTDOWN(mongo),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_MONGO
ZEND_GET_MODULE(mongo)
#endif

static PHP_INI_MH(OnUpdatePingInterval)
{
	long converted_val;

	if (new_value && is_numeric_string(new_value, new_value_length, &converted_val, NULL, 0) == IS_LONG && converted_val > 0) {
		MonGlo(manager)->ping_interval = converted_val;
		return SUCCESS;
	}

	return FAILURE;
}

static PHP_INI_MH(OnUpdateIsMasterInterval)
{
	long converted_val;

	if (new_value && is_numeric_string(new_value, new_value_length, &converted_val, NULL, 0) == IS_LONG && converted_val > 0) {
		MonGlo(manager)->ismaster_interval = converted_val;
		return SUCCESS;
	}

	return FAILURE;
}

/* {{{ PHP_INI */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("mongo.default_host", "localhost", PHP_INI_ALL, OnUpdateString, default_host, zend_mongo_globals, mongo_globals)
	STD_PHP_INI_ENTRY("mongo.default_port", "27017", PHP_INI_ALL, OnUpdateLong, default_port, zend_mongo_globals, mongo_globals)
	STD_PHP_INI_ENTRY("mongo.chunk_size", "262144", PHP_INI_ALL, OnUpdateLong, chunk_size, zend_mongo_globals, mongo_globals)
	STD_PHP_INI_ENTRY("mongo.cmd", "$", PHP_INI_ALL, OnUpdateStringUnempty, cmd_char, zend_mongo_globals, mongo_globals)
	STD_PHP_INI_ENTRY("mongo.native_long", "0", PHP_INI_ALL, OnUpdateLong, native_long, zend_mongo_globals, mongo_globals)
	STD_PHP_INI_ENTRY("mongo.long_as_object", "0", PHP_INI_ALL, OnUpdateLong, long_as_object, zend_mongo_globals, mongo_globals)
	STD_PHP_INI_ENTRY("mongo.allow_empty_keys", "0", PHP_INI_ALL, OnUpdateLong, allow_empty_keys, zend_mongo_globals, mongo_globals)

	PHP_INI_ENTRY("mongo.ping_interval", MONGO_MANAGER_DEFAULT_PING_INTERVAL_S, PHP_INI_ALL, OnUpdatePingInterval)
	PHP_INI_ENTRY("mongo.is_master_interval", MONGO_MANAGER_DEFAULT_MASTER_INTERVAL_S, PHP_INI_ALL, OnUpdateIsMasterInterval)
PHP_INI_END()
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(mongo)
{
	zend_class_entry max_key, min_key;

	REGISTER_INI_ENTRIES();
	le_cursor_list = zend_register_list_destructors_ex(NULL, php_mongo_cursor_list_pfree, PHP_CURSOR_LIST_RES_NAME, module_number);

	mongo_init_MongoClient(TSRMLS_C);
	mongo_init_Mongo(TSRMLS_C);
	mongo_init_MongoDB(TSRMLS_C);
	mongo_init_MongoCollection(TSRMLS_C);
	mongo_init_MongoCursor(TSRMLS_C);

	mongo_init_MongoGridFS(TSRMLS_C);
	mongo_init_MongoGridFSFile(TSRMLS_C);
	mongo_init_MongoGridFSCursor(TSRMLS_C);

	mongo_init_MongoId(TSRMLS_C);
	mongo_init_MongoCode(TSRMLS_C);
	mongo_init_MongoRegex(TSRMLS_C);
	mongo_init_MongoDate(TSRMLS_C);
	mongo_init_MongoBinData(TSRMLS_C);
	mongo_init_MongoDBRef(TSRMLS_C);

	mongo_init_MongoExceptions(TSRMLS_C);

	mongo_init_MongoTimestamp(TSRMLS_C);
	mongo_init_MongoInt32(TSRMLS_C);
	mongo_init_MongoInt64(TSRMLS_C);

	mongo_init_MongoLog(TSRMLS_C);

	/* Deprecated, but we will keep it for now */
	mongo_init_MongoPool(TSRMLS_C);

	/* MongoMaxKey and MongoMinKey are completely non-interactive: they have no
	 * method, fields, or constants.  */
	INIT_CLASS_ENTRY(max_key, "MongoMaxKey", NULL);
	mongo_ce_MaxKey = zend_register_internal_class(&max_key TSRMLS_CC);
	INIT_CLASS_ENTRY(min_key, "MongoMinKey", NULL);
	mongo_ce_MinKey = zend_register_internal_class(&min_key TSRMLS_CC);

	/* Make mongo objects uncloneable */
	memcpy(&mongo_default_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	mongo_default_handlers.clone_obj = NULL;

	/* Add compare_objects for MongoId */
	memcpy(&mongo_id_handlers, &mongo_default_handlers, sizeof(zend_object_handlers));
	mongo_id_handlers.compare_objects = php_mongo_compare_ids;

	/* Start random number generator */
	srand(time(0));

#ifdef WIN32
	cursor_mutex = CreateMutex(NULL, FALSE, NULL);
	if (cursor_mutex == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Windows couldn't create a mutex: %s", GetLastError());
		return FAILURE;
	}
#endif

#if MONGO_PHP_STREAMS
	REGISTER_LONG_CONSTANT("MONGO_STREAMS", 1, CONST_PERSISTENT);
#else
	REGISTER_LONG_CONSTANT("MONGO_STREAMS", 0, CONST_PERSISTENT);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
 */
static PHP_GINIT_FUNCTION(mongo)
{
	/* On windows, the max length is 256. Linux doesn't have a limit, but it
	 * will fill in the first 256 chars of hostname even if the actual
	 * hostname is longer. If you can't get a unique character in the first
	 * 256 chars of your hostname, you're doing it wrong. */
	int len, win_max = 256;
	char *hostname, host_start[256];
	register ulong hash;

	mongo_globals->default_host = "localhost";
	mongo_globals->default_port = 27017;
	mongo_globals->request_id = 3;
	mongo_globals->chunk_size = DEFAULT_CHUNK_SIZE;
	mongo_globals->cmd_char = "$";

	mongo_globals->response_num = 0;
	mongo_globals->errmsg = 0;

	mongo_globals->pool_size = -1;

	hostname = host_start;
	/* from the gnu manual:
	 *     gethostname stores the beginning of the host name in name even if the
	 *     host name won't entirely fit. For some purposes, a truncated host name
	 *     is good enough. If it is, you can ignore the error code.
	 * So we'll ignore the error code.
	 * Returns 0-terminated hostname. */
	gethostname(hostname, win_max);
	len = strlen(hostname);

	hash = 5381;

	/* from zend_hash.h */
	/* variant with the hash unrolled eight times */
	for (; len >= 8; len -= 8) {
		hash = ((hash << 5) + hash) + *hostname++;
		hash = ((hash << 5) + hash) + *hostname++;
		hash = ((hash << 5) + hash) + *hostname++;
		hash = ((hash << 5) + hash) + *hostname++;
		hash = ((hash << 5) + hash) + *hostname++;
		hash = ((hash << 5) + hash) + *hostname++;
		hash = ((hash << 5) + hash) + *hostname++;
		hash = ((hash << 5) + hash) + *hostname++;
	}

	switch (len) {
		case 7: hash = ((hash << 5) + hash) + *hostname++; /* fallthrough... */
		case 6: hash = ((hash << 5) + hash) + *hostname++; /* fallthrough... */
		case 5: hash = ((hash << 5) + hash) + *hostname++; /* fallthrough... */
		case 4: hash = ((hash << 5) + hash) + *hostname++; /* fallthrough... */
		case 3: hash = ((hash << 5) + hash) + *hostname++; /* fallthrough... */
		case 2: hash = ((hash << 5) + hash) + *hostname++; /* fallthrough... */
		case 1: hash = ((hash << 5) + hash) + *hostname++; break;
		case 0: break;
	}

	mongo_globals->machine = hash;

	mongo_globals->ts_inc = 0;
	mongo_globals->inc = rand() & 0xFFFFFF;

#if PHP_VERSION_ID >= 50300
	mongo_globals->log_callback_info = empty_fcall_info;
	mongo_globals->log_callback_info_cache = empty_fcall_info_cache;
#endif

	mongo_globals->manager = mongo_init();
	TSRMLS_SET_CTX(mongo_globals->manager->log_context);
	mongo_globals->manager->log_function = php_mcon_log_wrapper;

#if MONGO_PHP_STREAMS
	mongo_globals->manager->connect     = php_mongo_io_stream_connect;
	mongo_globals->manager->recv_header = php_mongo_io_stream_read;
	mongo_globals->manager->recv_data   = php_mongo_io_stream_read;
	mongo_globals->manager->send        = php_mongo_io_stream_send;
	mongo_globals->manager->close       = php_mongo_io_stream_close;
	mongo_globals->manager->forget      = php_mongo_io_stream_forget;
#endif
}
/* }}} */

PHP_GSHUTDOWN_FUNCTION(mongo)
{
	mongo_deinit(mongo_globals->manager);
}

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mongo)
{
	UNREGISTER_INI_ENTRIES();

#if WIN32
	/* 0 is failure */
	if (CloseHandle(cursor_mutex) == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Windows couldn't destroy a mutex: %s", GetLastError());
		return FAILURE;
	}
#endif

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(mongo)
{
	MonGlo(log_level) = 0;
	MonGlo(log_module) = 0;

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mongo)
{
	php_info_print_table_start();

	php_info_print_table_header(2, "MongoDB Support", "enabled");
	php_info_print_table_row(2, "Version", PHP_MONGO_VERSION);
#if MONGO_PHP_STREAMS
	php_info_print_table_row(2, "SSL Support", "enabled");
	php_info_print_table_row(2, "Streams Support", "enabled");
#else
	php_info_print_table_row(2, "SSL Support", "disabled");
	php_info_print_table_row(2, "Streams Support", "disabled");
#endif

	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

static void mongo_init_MongoExceptions(TSRMLS_D)
{
	mongo_init_MongoException(TSRMLS_C);
	mongo_init_MongoConnectionException(TSRMLS_C);
	mongo_init_MongoCursorException(TSRMLS_C);
	mongo_init_MongoCursorTimeoutException(TSRMLS_C);
	mongo_init_MongoGridFSException(TSRMLS_C);
	mongo_init_MongoResultException(TSRMLS_C);
}

/* Shared helper functions */
static mongo_read_preference_tagset *get_tagset_from_array(int tagset_id, zval *ztagset TSRMLS_DC)
{
	HashTable  *tagset = HASH_OF(ztagset);
	zval      **tag;
	int         item_count = 1, fail = 0;
	mongo_read_preference_tagset *tmp_ts = (mongo_read_preference_tagset*) calloc(1, sizeof(mongo_read_preference_tagset));

	zend_hash_internal_pointer_reset(tagset);
	while (zend_hash_get_current_data(tagset, (void **)&tag) == SUCCESS) {
		if (Z_TYPE_PP(tag) != IS_STRING) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tag %d in tagset %d needs to contain a string", item_count, tagset_id);
			fail = 1;
		} else {
			char *key;
			uint key_len;
			ulong num_key;

			switch (zend_hash_get_current_key_ex(tagset, &key, &key_len, &num_key, 0, NULL)) {
				case HASH_KEY_IS_LONG:
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tag %d in tagset %d has no string key", item_count, tagset_id);
					fail = 1;
					break;
				case HASH_KEY_IS_STRING:
					mongo_read_preference_add_tag(tmp_ts, key, Z_STRVAL_PP(tag));
					break;
			}

		}
		item_count++;
		zend_hash_move_forward(tagset);
	}
	if (fail) {
		mongo_read_preference_tagset_dtor(tmp_ts);
		return NULL;
	}
	return tmp_ts;
}

/* Returns an array of key=>value pairs, per tagset, from a
 * mongo_read_preference.  This maps to the structure on how mongos expects
 * them */
zval *php_mongo_make_tagsets(mongo_read_preference *rp)
{
	zval *tagsets, *tagset;
	int   i, j;

	if (!rp->tagset_count) {
		return NULL;
	}

	MAKE_STD_ZVAL(tagsets);
	array_init(tagsets);

	for (i = 0; i < rp->tagset_count; i++) {
		MAKE_STD_ZVAL(tagset);
		array_init(tagset);

		for (j = 0; j < rp->tagsets[i]->tag_count; j++) {
			char *name, *colon;
			char *tag = rp->tagsets[i]->tags[j];

			/* Split the "dc:ny" into ["dc" => "ny"] */
			colon = strchr(tag, ':');
			name = zend_strndup(tag, colon - tag);

			add_assoc_string(tagset, name, colon + 1, 1);
		}

		add_next_index_zval(tagsets, tagset);
	}

	return tagsets;
}

void php_mongo_add_tagsets(zval *return_value, mongo_read_preference *rp)
{
	zval *tagsets = php_mongo_make_tagsets(rp);

	if (!tagsets) {
		return;
	}

	add_assoc_zval_ex(return_value, "tagsets", sizeof("tagsets"), tagsets);
}

/* Applies an array of tagsets to the read preference. This function clears the
 * read preference before adding tagsets. If an error is encountered adding a
 * tagset, the read preference will again be cleared to avoid being left in an
 * inconsistent state. */
static int php_mongo_use_tagsets(mongo_read_preference *rp, HashTable *tagsets TSRMLS_DC)
{
	zval **tagset;
	int    item_count = 1;
	mongo_read_preference_tagset *tagset_tmp;

	/* Clear existing tagsets */
	mongo_read_preference_dtor(rp);

	zend_hash_internal_pointer_reset(tagsets);
	while (zend_hash_get_current_data(tagsets, (void **)&tagset) == SUCCESS) {
		if (Z_TYPE_PP(tagset) != IS_ARRAY) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tagset %d needs to contain an array of 0 or more tags", item_count);
			/* Clear any added tagsets to avoid an inconsistent state */
			mongo_read_preference_dtor(rp);
			return 0;
		} else {
			tagset_tmp = get_tagset_from_array(item_count, *tagset TSRMLS_CC);
			if (tagset_tmp) {
				mongo_read_preference_add_tagset(rp, tagset_tmp);
			} else {
				/* Clear any added tagsets to avoid an inconsistent state */
				mongo_read_preference_dtor(rp);
				return 0;
			}
		}
		item_count++;
		zend_hash_move_forward(tagsets);
	}
	return 1;
}

/* Sets read preference mode and tagsets. If an error is encountered, the read
 * preference will not be changed. */
int php_mongo_set_readpreference(mongo_read_preference *rp, char *read_preference, HashTable *tags TSRMLS_DC)
{
	mongo_read_preference tmp_rp;

	if (strcasecmp(read_preference, "primary") == 0) {
		if (tags && zend_hash_num_elements(tags)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "You can't use read preference tags with a read preference of PRIMARY");
			return 0;
		}
		tmp_rp.type = MONGO_RP_PRIMARY;
	} else if (strcasecmp(read_preference, "primaryPreferred") == 0) {
		tmp_rp.type = MONGO_RP_PRIMARY_PREFERRED;
	} else if (strcasecmp(read_preference, "secondary") == 0) {
		tmp_rp.type = MONGO_RP_SECONDARY;
	} else if (strcasecmp(read_preference, "secondaryPreferred") == 0) {
		tmp_rp.type = MONGO_RP_SECONDARY_PREFERRED;
	} else if (strcasecmp(read_preference, "nearest") == 0) {
		tmp_rp.type = MONGO_RP_NEAREST;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "The value '%s' is not valid as read preference type", read_preference);
		return 0;
	}

	tmp_rp.tagsets = NULL;
	tmp_rp.tagset_count = 0;

	if (tags && zend_hash_num_elements(tags)) {
		if (!php_mongo_use_tagsets(&tmp_rp, tags TSRMLS_CC)) {
			return 0;
		}
	}

	mongo_read_preference_replace(&tmp_rp, rp);
	mongo_read_preference_dtor(&tmp_rp);

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
