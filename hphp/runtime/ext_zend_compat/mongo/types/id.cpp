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
#include "../php_mongo.h"
#include "../bson.h"

ZEND_EXTERN_MODULE_GLOBALS(mongo)

extern zend_class_entry *mongo_ce_Exception;

zend_class_entry *mongo_ce_Id = NULL;

zend_object_handlers mongo_id_handlers;

void generate_id(char *data TSRMLS_DC)
{
	int inc;

#ifdef WIN32
	int pid = GetCurrentThreadId();
#else
	int pid = (int)getpid();
#endif

	unsigned t = (unsigned) time(0);
	char *T = (char*)&t,
	*M = (char*)&MonGlo(machine),
	*P = (char*)&pid,
	*I = (char*)&inc;

	/* inc */
	inc = MonGlo(inc);
	MonGlo(inc)++;

	/* actually generate the MongoId */
#if PHP_C_BIGENDIAN
	/* 4 bytes ts */
	memcpy(data, T, 4);

	/* we add 1 or 2 to the pointers so we don't end up with all 0s, as the
	 * interesting stuff is at the end for big endian systems */

	/* 3 bytes machine */
	memcpy(data + 4, M + 1, 3);

	/* 2 bytes pid */
	memcpy(data + 7, P + 2, 2);

	/* 3 bytes inc */
	memcpy(data + 9, I + 1, 3);
#else
	/* 4 bytes ts */
	data[0] = T[3];
	data[1] = T[2];
	data[2] = T[1];
	data[3] = T[0];

	/* 3 bytes machine */
	memcpy(data + 4, M, 3);

	/* 2 bytes pid */
	memcpy(data + 7, P, 2);

	/* 3 bytes inc */
	data[9] = I[2];
	data[10] = I[1];
	data[11] = I[0];
#endif
}

int php_mongo_compare_ids(zval *o1, zval *o2 TSRMLS_DC)
{
	if (
		Z_TYPE_P(o1) == IS_OBJECT && Z_TYPE_P(o2) == IS_OBJECT &&
		instanceof_function(Z_OBJCE_P(o1), mongo_ce_Id TSRMLS_CC) &&
		instanceof_function(Z_OBJCE_P(o2), mongo_ce_Id TSRMLS_CC)
	) {
		int i;

		mongo_id *id1 = (mongo_id*)zend_object_store_get_object(o1 TSRMLS_CC);
		mongo_id *id2 = (mongo_id*)zend_object_store_get_object(o2 TSRMLS_CC);

		for (i=0; i<12; i++) {
			if (id1->id[i] < id2->id[i]) {
				return -1;
			} else if (id1->id[i] > id2->id[i]) {
				return 1;
			}
		}
		return 0;
	}

	return 1;
}

static void php_mongo_id_free(void *object TSRMLS_DC)
{
	mongo_id *id = (mongo_id*)object;

	if (id) {
		if (id->id) {
			efree(id->id);
		}
		zend_object_std_dtor(&id->std TSRMLS_CC);
		efree(id);
	}
}

static zend_object_value php_mongo_id_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	mongo_id *intern;

	intern = (mongo_id*)emalloc(sizeof(mongo_id));
	memset(intern, 0, sizeof(mongo_id));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	init_properties(intern);

	retval.handle = zend_objects_store_put(intern,
	(zend_objects_store_dtor_t) zend_objects_destroy_object,
	php_mongo_id_free, NULL TSRMLS_CC);
	retval.handlers = &mongo_id_handlers;

	return retval;
}

/* {{{ MongoId::__toString()
 */
PHP_METHOD(MongoId, __toString)
{
	int i;
	mongo_id *this_id;
	char *id_str;
	char *id;

	this_id = (mongo_id*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED_STRING(this_id->id, MongoId);

	id = (char*)emalloc(25);
	id_str = this_id->id;

	for ( i = 0; i < 12; i++) {
		int x = *id_str;
		char digit1, digit2;

		if (*id_str < 0) {
			x = 256 + *id_str;
		}

		digit1 = x / 16;
		digit2 = x % 16;

		id[2 * i]   = (digit1 < 10) ? '0' + digit1 : digit1 - 10 + 'a';
		id[2 * i + 1] = (digit2 < 10) ? '0' + digit2 : digit2 - 10 + 'a';

		id_str++;
	}

	id[24] = '\0';

	RETURN_STRING(id, NO_DUP);
}
/* }}} */

/* {{{ MongoId::__construct()
 */
PHP_METHOD(MongoId, __construct)
{
	zval *id = 0, *str = 0;
	mongo_id *this_id = (mongo_id*)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z!", &id) == FAILURE) {
		return;
	}

	if (!this_id->id) {
		this_id->id = (char*)emalloc(OID_SIZE + 1);
		this_id->id[OID_SIZE] = '\0';
	}

	if (id && Z_TYPE_P(id) == IS_STRING && Z_STRLEN_P(id) == 24) {
		int i;

		if (strspn(Z_STRVAL_P(id), "0123456789abcdefABCDEF") != 24) {
			zend_throw_exception(mongo_ce_Exception, "ID must be valid hex characters", 18 TSRMLS_CC);
			return;
		}
		for (i = 0; i < 12;i++) {
			char digit1 = Z_STRVAL_P(id)[i * 2], digit2 = Z_STRVAL_P(id)[i * 2 + 1];

			digit1 = digit1 >= 'a' && digit1 <= 'f' ? digit1 - 87 : digit1;
			digit1 = digit1 >= 'A' && digit1 <= 'F' ? digit1 - 55 : digit1;
			digit1 = digit1 >= '0' && digit1 <= '9' ? digit1 - 48 : digit1;

			digit2 = digit2 >= 'a' && digit2 <= 'f' ? digit2 - 87 : digit2;
			digit2 = digit2 >= 'A' && digit2 <= 'F' ? digit2 - 55 : digit2;
			digit2 = digit2 >= '0' && digit2 <= '9' ? digit2 - 48 : digit2;

			this_id->id[i] = digit1 * 16 + digit2;
		}

		zend_update_property(mongo_ce_Id, getThis(), "$id", strlen("$id"), id TSRMLS_CC);
	} else if (id && Z_TYPE_P(id) == IS_OBJECT && Z_OBJCE_P(id) == mongo_ce_Id) {
		zval *str;

		mongo_id *that_id = (mongo_id*)zend_object_store_get_object(id TSRMLS_CC);

		memcpy(this_id->id, that_id->id, OID_SIZE);

		str = zend_read_property(mongo_ce_Id, id, "$id", strlen("$id"), NOISY TSRMLS_CC);
		zend_update_property(mongo_ce_Id, getThis(), "$id", strlen("$id"), str TSRMLS_CC);
	} else if (id) {
		zend_throw_exception(mongo_ce_Exception, "Invalid object ID", 19 TSRMLS_CC);
		return;
	} else {
		generate_id(this_id->id TSRMLS_CC);

		MAKE_STD_ZVAL(str);
		ZVAL_NULL(str);

		MONGO_METHOD(MongoId, __toString, str, getThis());
		zend_update_property(mongo_ce_Id, getThis(), "$id", strlen("$id"), str TSRMLS_CC);
		zval_ptr_dtor(&str);
	}
}
/* }}} */

/* {{{ MongoId::__set_state()
 */
PHP_METHOD(MongoId, __set_state)
{
	zval temp, *state, **id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &state) == FAILURE) {
		return;
	}

	if (zend_hash_find(HASH_P(state), "$id", strlen("$id") + 1, (void**) &id) == FAILURE) {
		return;
	}

	object_init_ex(return_value, mongo_ce_Id);
	MONGO_METHOD1(MongoId, __construct, &temp, return_value, *id);
}
/* }}} */

/* {{{ MongoId::getTimestamp
 */
PHP_METHOD(MongoId, getTimestamp)
{
	int ts = 0, i;
	mongo_id *id = (mongo_id*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED_STRING(id->id, MongoId);

	for ( i = 0; i < 4; i++) {
		int x = ((int)id->id[i] < 0) ? 256 + id->id[i] : id->id[i];
		ts = (ts * 256) + x;
	}

	RETURN_LONG(ts);
}
/* }}} */

/* {{{ MongoId::getPID
 */
PHP_METHOD(MongoId, getPID)
{
	int pid = 0, i;
	mongo_id *id = (mongo_id*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED_STRING(id->id, MongoId);

	for (i = 8; i > 6; i--) {
		int x;

		x = ((int)id->id[i] < 0) ? 256 + id->id[i] : id->id[i];
		pid = (pid * 256) + x;
	}

	RETURN_LONG(pid);
}
/* }}} */

PHP_METHOD(MongoId, getInc)
{
	int inc = 0;
	char *ptr = (char*)&inc;
	mongo_id *id = (mongo_id*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED_STRING(id->id, MongoId);

	/* 11, 10, 9, '\0' */
	ptr[0] = id->id[11];
	ptr[1] = id->id[10];
	ptr[2] = id->id[9];

	RETURN_LONG(inc);
}

/* {{{ MongoId::getHostname
 */
PHP_METHOD(MongoId, getHostname)
{
	char hostname[256];

	gethostname(hostname, 256);
	RETURN_STRING(hostname, 1);
}
/* }}} */

int php_mongo_id_serialize(zval *struc, unsigned char **serialized_data, zend_uint *serialized_length, zend_serialize_data *var_hash TSRMLS_DC)
{
	zval str;

	MONGO_METHOD(MongoId, __toString, &str, struc);
	*(serialized_length) = Z_STRLEN(str);
	*(serialized_data) = (unsigned char*)Z_STRVAL(str);
	return SUCCESS;
}

int php_mongo_id_unserialize(zval **rval, zend_class_entry *ce, const unsigned char* p, zend_uint datalen, zend_unserialize_data* var_hash TSRMLS_DC)
{
	zval temp, *str;

	MAKE_STD_ZVAL(str);
	ZVAL_STRINGL(str, (const char*)p, 24, 1);

	object_init_ex(*rval, mongo_ce_Id);

	MONGO_METHOD1(MongoId, __construct, &temp, *rval, str);
	zval_ptr_dtor(&str);

	return SUCCESS;
}

static zend_function_entry MongoId_methods[] = {
	PHP_ME(MongoId, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoId, __toString, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoId, __set_state, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(MongoId, getTimestamp, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoId, getHostname, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(MongoId, getPID, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoId, getInc, NULL, ZEND_ACC_PUBLIC)

	{ NULL, NULL, NULL }
};

void mongo_init_MongoId(TSRMLS_D)
{
	zend_class_entry id;
	INIT_CLASS_ENTRY(id, "MongoId", MongoId_methods);

	id.create_object = php_mongo_id_new;
	id.serialize = php_mongo_id_serialize;
	id.unserialize = php_mongo_id_unserialize;

	mongo_ce_Id = zend_register_internal_class(&id TSRMLS_CC);

	zend_declare_property_null(mongo_ce_Id, "$id", strlen("$id"), ZEND_ACC_PUBLIC TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
