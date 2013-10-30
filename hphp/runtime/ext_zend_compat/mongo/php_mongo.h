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
#ifndef PHP_MONGO_H
#define PHP_MONGO_H 1

#define PHP_MONGO_VERSION "1.4.3"
#define PHP_MONGO_EXTNAME "mongo"

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# if WIN32
#  include "config-w32.h"
# endif
#endif

#include "mcon/types.h"
#include "mcon/read_preference.h"

/* resource names */
#define PHP_CONNECTION_RES_NAME "mongo connection"
#define PHP_SERVER_RES_NAME "mongo server info"
#define PHP_CURSOR_LIST_RES_NAME "cursor list"

#ifndef zend_parse_parameters_none
# define zend_parse_parameters_none() \
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "")
#endif

#ifndef Z_UNSET_ISREF_P
# define Z_UNSET_ISREF_P(pz)      pz->is_ref = 0
#endif

#ifndef ZVAL_COPY_VALUE
#define ZVAL_COPY_VALUE(z, v) \
	do { \
		(z)->value = (v)->value; \
		Z_TYPE_P(z) = Z_TYPE_P(v); \
	} while (0)
#endif

#ifndef INIT_PZVAL_COPY
# define INIT_PZVAL_COPY(z, v) \
	do { \
		ZVAL_COPY_VALUE(z, v); \
		Z_SET_REFCOUNT_P(z, 1); \
		Z_UNSET_ISREF_P(z); \
	} while (0)
#endif

#ifndef MAKE_COPY_ZVAL
# define MAKE_COPY_ZVAL(ppzv, pzv) \
	INIT_PZVAL_COPY(pzv, *(ppzv)); \
	zval_copy_ctor((pzv));
#endif

#ifdef WIN32
# ifndef int64_t
typedef __int64 int64_t;
# endif
#endif

#ifndef Z_ADDREF_P
# define Z_ADDREF_P(pz)                (pz)->refcount++
#endif

#ifndef Z_ADDREF_PP
# define Z_ADDREF_PP(ppz)               Z_ADDREF_P(*(ppz))
#endif

#ifndef Z_DELREF_P
# define Z_DELREF_P(pz)                (pz)->refcount--
#endif

#ifndef Z_SET_REFCOUNT_P
# define Z_SET_REFCOUNT_P(pz, rc)      (pz)->refcount = (rc)
#endif

#define INT_32 4
#define INT_64 8
#define DOUBLE_64 8
#define BYTE_8 1

/* db ops */
#define OP_REPLY 1
#define OP_MSG 1000
#define OP_UPDATE 2001
#define OP_INSERT 2002
#define OP_GET_BY_OID 2003
#define OP_QUERY 2004
#define OP_GET_MORE 2005
#define OP_DELETE 2006
#define OP_KILL_CURSORS 2007

/* cursor flags */
#define CURSOR_NOT_FOUND 1
#define CURSOR_ERR 2

#define MSG_HEADER_SIZE 16
#define REPLY_HEADER_SIZE (MSG_HEADER_SIZE+20)
#define INITIAL_BUF_SIZE 4096
#define DEFAULT_CHUNK_SIZE (256*1024)

#define PHP_MONGO_DEFAULT_WTIMEOUT 10000
#define PHP_MONGO_DEFAULT_SOCKET_TIMEOUT 30000L

/* if _id field should be added */
#define PREP 1
#define NO_PREP 0

#define NOISY 0
#define QUIET 1

/* duplicate strings */
#define DUP 1
#define NO_DUP 0

#define PERSIST 1
#define NO_PERSIST 0

#define FLAGS 0

#define LAST_ERROR 0
#define PREV_ERROR 1
#define RESET_ERROR 2
#define FORCE_ERROR 3

#if PHP_VERSION_ID > 50300
# define MONGO_ARGINFO_STATIC
#else
# define MONGO_ARGINFO_STATIC static
#endif

#if PHP_VERSION_ID >= 50300
# define PUSH_PARAM(arg) zend_vm_stack_push(arg TSRMLS_CC)
# define POP_PARAM() (void)zend_vm_stack_pop(TSRMLS_C)
# define PUSH_EO_PARAM()
# define POP_EO_PARAM()
#else
# define PUSH_PARAM(arg) zend_ptr_stack_push(&EG(argument_stack), arg)
# define POP_PARAM() (void)zend_ptr_stack_pop(&EG(argument_stack))
# define PUSH_EO_PARAM() zend_ptr_stack_push(&EG(argument_stack), NULL)
# define POP_EO_PARAM() (void)zend_ptr_stack_pop(&EG(argument_stack))
#endif

#if PHP_VERSION_ID >= 50300
# define MONGO_E_DEPRECATED E_DEPRECATED
#else
# define MONGO_E_DEPRECATED E_STRICT
#endif

#define MUST_BE_ARRAY_OR_OBJECT(num, arg) do { \
	if (arg && !(Z_TYPE_P(arg) == IS_ARRAY || Z_TYPE_P(arg) == IS_OBJECT)) { \
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "expects parameter %d to be an array or object, %s given", num, zend_get_type_by_const(Z_TYPE_P(arg))); \
		RETURN_NULL(); \
	} \
} while(0);

#if PHP_VERSION_ID < 50300
#define zpp_var_args(argv, argc) do { \
	if (ZEND_NUM_ARGS() < 1) { \
		WRONG_PARAM_COUNT; \
	} \
	argv = (zval ***)safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval **), 0); \
	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), argv) == FAILURE) { \
		efree(argv); \
		WRONG_PARAM_COUNT; \
	} \
	argc = ZEND_NUM_ARGS(); \
} while(0);
#else
#define zpp_var_args(argv, argc) do { \
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &argv, &argc) == FAILURE) { \
		return; \
	} \
} while(0);
#endif

#define MONGO_METHOD_BASE(classname, name) zim_##classname##_##name

#define MONGO_METHOD_HELPER(classname, name, retval, thisptr, num, param) \
	PUSH_PARAM(param); PUSH_PARAM((void*)num);				\
	PUSH_EO_PARAM();							\
	MONGO_METHOD_BASE(classname, name)(num, retval, NULL, thisptr, 0 TSRMLS_CC); \
	POP_EO_PARAM();			\
	POP_PARAM(); POP_PARAM();

/* push parameters, call function, pop parameters */
#define MONGO_METHOD(classname, name, retval, thisptr)			\
	MONGO_METHOD_BASE(classname, name)(0, retval, NULL, thisptr, 0 TSRMLS_CC);

#define MONGO_METHOD1(classname, name, retval, thisptr, param1)		\
	MONGO_METHOD_HELPER(classname, name, retval, thisptr, 1, param1);

#define MONGO_METHOD2(classname, name, retval, thisptr, param1, param2)	\
	PUSH_PARAM(param1);							\
	MONGO_METHOD_HELPER(classname, name, retval, thisptr, 2, param2);	\
	POP_PARAM();

#define MONGO_METHOD3(classname, name, retval, thisptr, param1, param2, param3) \
	PUSH_PARAM(param1); PUSH_PARAM(param2);				\
	MONGO_METHOD_HELPER(classname, name, retval, thisptr, 3, param3);	\
	POP_PARAM(); POP_PARAM();

#define MONGO_METHOD4(classname, name, retval, thisptr, param1, param2, param3, param4) \
	PUSH_PARAM(param1); PUSH_PARAM(param2); PUSH_PARAM(param3);		\
	MONGO_METHOD_HELPER(classname, name, retval, thisptr, 4, param4);	\
	POP_PARAM(); POP_PARAM(); POP_PARAM();

#define MONGO_METHOD5(classname, name, retval, thisptr, param1, param2, param3, param4, param5) \
	PUSH_PARAM(param1); PUSH_PARAM(param2); PUSH_PARAM(param3); PUSH_PARAM(param4); \
	MONGO_METHOD_HELPER(classname, name, retval, thisptr, 5, param5);	\
	POP_PARAM(); POP_PARAM(); POP_PARAM(); POP_PARAM();

#define MONGO_CMD(retval, thisptr) MONGO_METHOD1(MongoDB, command, retval, thisptr, data)
#define MONGO_CMD_WITH_RP(retval, thisptr, collection) \
	do { \
		mongo_db *db = (mongo_db*)zend_object_store_get_object(collection->parent TSRMLS_CC); \
		mongo_read_preference rp; \
		mongo_read_preference_copy(&db->read_pref, &rp); \
		mongo_read_preference_replace(&collection->read_pref, &db->read_pref); \
		MONGO_METHOD1(MongoDB, command, retval, thisptr, data) \
		mongo_read_preference_replace(&rp, &db->read_pref); \
		mongo_read_preference_dtor(&rp); \
	} while(0);

#define HASH_P(a) (Z_TYPE_P(a) == IS_ARRAY ? Z_ARRVAL_P(a) : Z_OBJPROP_P(a))
#define HASH_PP(a) (Z_TYPE_PP(a) == IS_ARRAY ? Z_ARRVAL_PP(a) : Z_OBJPROP_PP(a))

#define IS_SCALAR_P(a) (Z_TYPE_P(a) != IS_ARRAY && Z_TYPE_P(a) != IS_OBJECT)
#define IS_SCALAR_PP(a) (Z_TYPE_PP(a) != IS_ARRAY && Z_TYPE_PP(a) != IS_OBJECT)

/* TODO: this should be expanded to handle long_as_object being set */
#define Z_NUMVAL_P(variable, value)                                     \
  ((Z_TYPE_P(variable) == IS_LONG && Z_LVAL_P(variable) == value) ||    \
   (Z_TYPE_P(variable) == IS_DOUBLE && Z_DVAL_P(variable) == value))
#define Z_NUMVAL_PP(variable, value)                                    \
  ((Z_TYPE_PP(variable) == IS_LONG && Z_LVAL_PP(variable) == value) ||  \
   (Z_TYPE_PP(variable) == IS_DOUBLE && Z_DVAL_PP(variable) == value))

#if PHP_VERSION_ID >= 50400
# define init_properties(intern) object_properties_init(&intern->std, class_type)
#else
# define init_properties(intern) {                                     \
	zval *tmp;                                                         \
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *)); \
}
#endif

#define PHP_MONGO_OBJ_NEW(mongo_obj)                    \
	zend_object_value retval;                           \
	mongo_obj *intern;                                  \
	                                                    \
	intern = (mongo_obj*)emalloc(sizeof(mongo_obj));               \
	memset(intern, 0, sizeof(mongo_obj));                          \
	                                                               \
	zend_object_std_init(&intern->std, class_type TSRMLS_CC);      \
	init_properties(intern);                                       \
	                                                               \
	retval.handle = zend_objects_store_put(intern,(zend_objects_store_dtor_t) zend_objects_destroy_object, php_##mongo_obj##_free, NULL TSRMLS_CC); \
	retval.handlers = &mongo_default_handlers;                     \
	                                                               \
	return retval;

#define RS_PRIMARY 1
#define RS_SECONDARY 2

typedef struct {
	zend_object std;

	mongo_con_manager *manager; /* Contains a link to the manager */
	mongo_servers     *servers;
} mongoclient;

#define MONGO_CURSOR 1

typedef struct {
	int length;
	int request_id;
	int response_to;
	int op;
} mongo_msg_header;

typedef struct {
	char *start;
	char *pos;
	char *end;
} buffer;

#define CREATE_MSG_HEADER(rid, rto, opcode) \
	header.length = 0; \
	header.request_id = rid; \
	header.response_to = rto; \
	header.op = opcode;

#define CREATE_RESPONSE_HEADER(buf, ns, rto, opcode) \
	CREATE_MSG_HEADER(MonGlo(request_id)++, rto, opcode); \
	APPEND_HEADER_NS(buf, ns, 0);

#define CREATE_HEADER_WITH_OPTS(buf, ns, opcode, opts) \
	CREATE_MSG_HEADER(MonGlo(request_id)++, 0, opcode); \
	APPEND_HEADER_NS(buf, ns, opts);

#define CREATE_HEADER(buf, ns, opcode) \
	CREATE_RESPONSE_HEADER(buf, ns, 0, opcode);

#define APPEND_HEADER(buf, opts) buf->pos += INT_32; \
	php_mongo_serialize_int(buf, header.request_id); \
	php_mongo_serialize_int(buf, header.response_to); \
	php_mongo_serialize_int(buf, header.op); \
	php_mongo_serialize_int(buf, opts);


#define APPEND_HEADER_NS(buf, ns, opts) \
	APPEND_HEADER(buf, opts); \
	php_mongo_serialize_ns(buf, ns TSRMLS_CC);


#define MONGO_CHECK_INITIALIZED(member, class_name) \
	if (!(member)) { \
		zend_throw_exception(mongo_ce_Exception, "The " #class_name " object has not been correctly initialized by its constructor", 0 TSRMLS_CC); \
		RETURN_FALSE; \
	}

#define MONGO_CHECK_INITIALIZED_STRING(member, class_name) \
	if (!(member)) { \
		zend_throw_exception(mongo_ce_Exception, "The " #class_name " object has not been correctly initialized by its constructor", 0 TSRMLS_CC); \
		RETURN_STRING("", 1); \
	}

#define PHP_MONGO_GET_LINK(obj) \
	link = (mongoclient*)zend_object_store_get_object((obj) TSRMLS_CC); \
	MONGO_CHECK_INITIALIZED(link->servers, Mongo);

#define PHP_MONGO_GET_DB(obj) \
	db = (mongo_db*)zend_object_store_get_object((obj) TSRMLS_CC); \
	MONGO_CHECK_INITIALIZED(db->name, MongoDB);

#define PHP_MONGO_GET_COLLECTION(obj) \
	c = (mongo_collection*)zend_object_store_get_object((obj) TSRMLS_CC); \
	MONGO_CHECK_INITIALIZED(c->ns, MongoCollection);

#define PHP_MONGO_GET_CURSOR(obj) \
	cursor = (mongo_cursor*)zend_object_store_get_object((obj) TSRMLS_CC); \
	MONGO_CHECK_INITIALIZED(cursor->resource, MongoCursor);

#define PHP_MONGO_CHECK_EXCEPTION() if (EG(exception)) { return; }

#define PHP_MONGO_CHECK_EXCEPTION1(arg1) \
	if (EG(exception)) { \
		zval_ptr_dtor(arg1); \
		return; \
	}

#define PHP_MONGO_CHECK_EXCEPTION2(arg1, arg2) \
	if (EG(exception)) { \
		zval_ptr_dtor(arg1); \
		zval_ptr_dtor(arg2); \
	return; \
	}

#define PHP_MONGO_CHECK_EXCEPTION3(arg1, arg2, arg3) \
	if (EG(exception)) { \
		zval_ptr_dtor(arg1); \
		zval_ptr_dtor(arg2); \
		zval_ptr_dtor(arg3); \
		return; \
	}

#define PHP_MONGO_CHECK_EXCEPTION4(arg1, arg2, arg3, arg4) \
	if (EG(exception)) { \
		zval_ptr_dtor(arg1); \
		zval_ptr_dtor(arg2); \
		zval_ptr_dtor(arg3); \
		zval_ptr_dtor(arg4); \
		return; \
	}

#define PHP_MONGO_SERIALIZE_KEY(type) \
	php_mongo_set_type(buf, type); \
	php_mongo_serialize_key(buf, name, name_len, prep TSRMLS_CC); \
	if (EG(exception)) { \
		return ZEND_HASH_APPLY_STOP; \
	}


#define REPLY_HEADER_LEN 36

typedef struct {
	zend_object std;

	/* Connection */
	mongo_connection *connection;
	zval *resource;

	/* collection namespace */
	char *ns;

	/* fields to send */
	zval *query;
	zval *fields;
	int limit;
	int batch_size;
	int skip;
	int opts;

	char special;
	int timeout;

	mongo_msg_header send;
	mongo_msg_header recv;

	/* response fields */
	int flag;
	int start;
	/* number of results used */
	int at;
	/* number results returned */
	int num;
	/* results */
	buffer buf;

	/* cursor_id indicates if there are more results to fetch.  If cursor_id
	 * is 0, the cursor is "dead."  If cursor_id != 0, server is set to the
	 * server that was queried, so a get_more doesn't try to fetch results
	 * from the wrong server.  server just points to a member of link, so it
	 * should never need to be freed. */
	int64_t cursor_id;

	zend_bool started_iterating;
	zend_bool persist;

	zval *current;
	int retry;

	mongo_read_preference read_pref;

	int force_primary; /* If set to 1 then the connection selection will request a WRITE (primary) connection */
	int dead;
} mongo_cursor;

/* Unfortunately, cursors can be freed before or after link is destroyed, so we
 * can't actually depend on having a link to the database. So, we're going to
 * keep a separate list of link ids associated with cursor ids.
 *
 * When a cursor is to be freed, we try to find this cursor in the list. If
 * it's there, kill it.  If not, the db connection is probably already dead.
 *
 * When a connection is killed, we sweep through the list and kill all the
 * cursors for that link. */
typedef struct _cursor_node {
	int64_t cursor_id;
	void *socket;

	struct _cursor_node *next;
	struct _cursor_node *prev;
} cursor_node;

typedef struct {
	zend_object std;
	char *id;
} mongo_id;


typedef struct {
	zend_object std;
	zval *link;
	zval *name;

	mongo_read_preference read_pref;
} mongo_db;

typedef struct {
	zend_object std;

	/* parent database */
	zval *parent;
	zval *link;

	/* names */
	zval *name;
	zval *ns;

	mongo_read_preference read_pref;
} mongo_collection;


#define BUF_REMAINING (buf->end-buf->pos)

#define CREATE_BUF(buf, size) \
	buf.start = (char*)emalloc(size); \
	buf.pos = buf.start; \
	buf.end = buf.start + size;

PHP_MINIT_FUNCTION(mongo);
PHP_MSHUTDOWN_FUNCTION(mongo);
PHP_RINIT_FUNCTION(mongo);
PHP_MINFO_FUNCTION(mongo);

/* Serialization functions */
PHP_FUNCTION(bson_encode);
PHP_FUNCTION(bson_decode);


/* Mutex macros */
#ifdef WIN32
# define LOCK(lk) { \
	int ret = -1; \
	int tries = 0; \
	\
	while (tries++ < 3 && ret != 0) { \
		ret = WaitForSingleObject(lk##_mutex, 5000); \
		if (ret != 0) { \
			if (ret == WAIT_TIMEOUT) { \
				continue; \
			} else { \
				break; \
			} \
		} \
	} \
}
# define UNLOCK(lk) ReleaseMutex(lk##_mutex);
#else
# define LOCK(lk) pthread_mutex_lock(&lk##_mutex);
# define UNLOCK(lk) pthread_mutex_unlock(&lk##_mutex);
#endif

void mongo_init_MongoDB(TSRMLS_D);
void mongo_init_MongoCollection(TSRMLS_D);
void mongo_init_MongoCursor(TSRMLS_D);

void mongo_init_MongoGridFS(TSRMLS_D);
void mongo_init_MongoGridFSFile(TSRMLS_D);
void mongo_init_MongoGridFSCursor(TSRMLS_D);

void mongo_init_MongoId(TSRMLS_D);
void mongo_init_MongoCode(TSRMLS_D);
void mongo_init_MongoRegex(TSRMLS_D);
void mongo_init_MongoDate(TSRMLS_D);
void mongo_init_MongoBinData(TSRMLS_D);
void mongo_init_MongoDBRef(TSRMLS_D);
void mongo_init_MongoTimestamp(TSRMLS_D);
void mongo_init_MongoInt32(TSRMLS_D);
void mongo_init_MongoInt64(TSRMLS_D);

/* Shared helper functions */
zval *php_mongo_make_tagsets(mongo_read_preference *rp);
void php_mongo_add_tagsets(zval *return_value, mongo_read_preference *rp);
int php_mongo_set_readpreference(mongo_read_preference *rp, char *read_preference, HashTable *tags TSRMLS_DC);

ZEND_BEGIN_MODULE_GLOBALS(mongo)
	/* php.ini options */
	char *default_host;
	long default_port;
	long request_id;
	int chunk_size;

	/* $ alternative */
	char *cmd_char;
	long native_long;
	long long_as_object;
	long allow_empty_keys;

	/* _id generation helpers */
	int inc, pid, machine;

	/* timestamp generation helper */
	long ts_inc;
	char *errmsg;
	int response_num;
	int pool_size;

	long log_level;
	long log_module;
	zend_fcall_info log_callback_info;
	zend_fcall_info_cache log_callback_info_cache;

	long ping_interval;
	long ismaster_interval;

	mongo_con_manager *manager;
ZEND_END_MODULE_GLOBALS(mongo)

#ifdef ZTS
#include <TSRM.h>
# define MonGlo(v) TSRMG(mongo_globals_id, zend_mongo_globals *, v)
#else
# define MonGlo(v) (mongo_globals.v)
#endif

extern zend_module_entry mongo_module_entry;
#define phpext_mongo_ptr &mongo_module_entry

#endif

/*
 * Error codes
 *
 * TODO: Check and update those all
 *
 * MongoException:
 * 0: The <class> object has not been correctly initialized by its constructor
 * 1: zero-length keys are not allowed, did you use $ with double quotes?
 * 2: characters not allowed in key: <key>
 * 3: insert too large: <size>, max: 16000000
 * 4: no elements in doc
 * 5: size of BSON doc is <size> bytes, max 4MB
 * 6: no documents given
 * 7: MongoCollection::group takes an array, object, or MongoCode key
 * 8: field names must be strings
 * 9: invalid regex
 * 10: MongoDBRef::get: $ref field must be a string
 * 11: MongoDBRef::get: $db field must be a string
 * 12: non-utf8 string: <str>
 * 13: mutex error: <err>
 * 14: index name too long: <len>, max <max> characters
 * 15: Reading from slaves won't work without using the replicaSet option on connect
 * 16: No server found for reads
 * 17: The MongoCollection object has not been correctly initialized by its constructor
 * 18: ID must be valid hex characters
 * 19: Invalid object ID
 * 20: Cannot run command count(): (error message from MongoDB)
 * 21: Namespace field is invalid.
 *
 * MongoConnectionException:
 * 0: connection to <host> failed: <errmsg>
 * 1: no server name given
 * 2: can't use slaveOkay without replicaSet
 * 3: could not store persistent link
 * 4: pass in an identifying string to get a persistent connection
 * 5: failed to get primary or secondary
 * 10: failed to get host from <substr> of <str>
 * 11: failed to get port from <substr> of <str>
 * 12: lost db connection
 * 2X: Parsing errors (ununsed)
 * 21: Empty option name or value
 * 22: Unknown connection string option
 * 23: Logical error (conflicting options)
 * 24: (unused)
 * 25: Option with no string key
 * 26: SSL support is only available when compiled against PHP Streams
 * 27: Driver options are only available when compiled against PHP Streams
 * 31: Unknown failure doing io_stream_read.
 * 32: When the remote server closes the connection in io_stream_read.
 * 72: Could not retrieve connection
 *
 * MongoCursorTimeoutException:
 * 80: timeout exception
 *
 * MongoCursorException:
 * 0: cannot modify cursor after beginning iteration
 * 1: get more: send error (C error string)
 * 2: get more: cursor not found
 * 3: cursor->buf.pos is null
 * 4: couldn't get response header
 * 5: no db response
 * 6: bad response length: <len>, max: <len>, did the db assert?
 * 7: incomplete header
 * 8: incomplete response
 * 9: couldn't find a response
 * 10: error getting socket
 * 11: couldn't find reply, please try again
 * 12: [WSA ]error getting database response: <err>
 * 13: Timeout error (C error)
 * 14: couldn't send query: <err>
 * 15: couldn't get sock for safe op
 * 16: couldn't send safe op
 * 17: exceptional condition on socket
 * 18: Trying to get more, but cannot find server
 * 19: max number of retries exhausted, couldn't send query
 * 20: something exceptional has happened, and the cursor is now dead
 * 21: invalid string length for key "%s"
 * 22: invalid binary length for key "%s"
 * 23: Can not natively represent the long %llu on this platform
 * 24: invalid code length for key "%s"
 * 28: recv_header() (abs()) recv_data() stream handlers error (timeout)
 * 29: Unknown query/get_more failure
 *
 * MongoGridFSException:
 * 0:
 * 1: There is more data in the stored file than the meta data shows
 * 2: Invalid collection prefix (throws Exception, not MongoGridFSException)
 * 3: Could not open file for reading
 * 4: Filesize larger then we can handle
 * 5: Invalid filehandle for a resource
 * 6: Resource doesn't contain filehandle
 * 7: Error setting up file for reading
 * 8: Argument not a file stream or a filename string (throws Exception, not MongoGridFSException)
 * 9: Error reading file data
 * 10: Error reading from resource
 * 11: Can't find uploaded file
 * 12: tmp_name not found, upload probably failed
 * 13: tmp_name was not a valid filename
 * 14: Unable to determin file size
 * 15: Missing filename
 * 16: Could not open filename for writing
 * 17: Could not read chunk
 * 18: Failed creating file stream
 * 19: Could not find array key
 * 20: Chunk larger then chunksize
 * 21: Unexpected chunk format
 */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
