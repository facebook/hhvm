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
#include "mongoclient.h"
#include "mongo.h"
#include "db.h"
#include "util/pool.h"

extern zend_object_handlers mongoclient_handlers;

ZEND_EXTERN_MODULE_GLOBALS(mongo)

zend_class_entry *mongo_ce_Mongo;

extern zend_class_entry *mongo_ce_MongoClient, *mongo_ce_DB;
extern zend_class_entry *mongo_ce_Exception;

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo___construct, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, server)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_no_parameters, 0, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_setSlaveOkay, 0, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, slave_okay)
ZEND_END_ARG_INFO()

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_setPoolSize, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

static zend_function_entry mongo_methods[] = {
	PHP_ME(Mongo, __construct, arginfo___construct, ZEND_ACC_PUBLIC)

	/* All these methods only exist in Mongo, and no longer in MongoClient */
	PHP_ME(Mongo, connectUtil, arginfo_no_parameters, ZEND_ACC_PROTECTED)
	PHP_ME(Mongo, getSlaveOkay, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, setSlaveOkay, arginfo_setSlaveOkay, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, lastError, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, prevError, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, resetError, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, forceError, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, getSlave, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, switchSlave, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, setPoolSize, arginfo_setPoolSize, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, getPoolSize, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_DEPRECATED)
	PHP_ME(Mongo, poolDebug, arginfo_no_parameters, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_DEPRECATED)

	{ NULL, NULL, NULL }
};


void mongo_init_Mongo(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Mongo", mongo_methods); /* FIXME: Use mongo_methods here */
	ce.create_object = php_mongoclient_new;
	mongo_ce_Mongo = zend_register_internal_class_ex(&ce, mongo_ce_MongoClient, NULL TSRMLS_CC);

	/* make mongoclient object uncloneable, and with its own read_property */
	memcpy(&mongoclient_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	mongoclient_handlers.clone_obj = NULL;
	mongoclient_handlers.read_property = mongo_read_property;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	mongoclient_handlers.get_debug_info = mongo_get_debug_info;
#endif
}

/* {{{ Mongo->__construct
*/
PHP_METHOD(Mongo, __construct)
{
	php_mongo_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

PHP_METHOD(Mongo, getSlaveOkay)
{
	mongoclient *link;
	PHP_MONGO_GET_LINK(getThis());
	RETURN_BOOL(link->servers->read_pref.type != MONGO_RP_PRIMARY);
}

PHP_METHOD(Mongo, getSlave)
{
	mongoclient *link;
	mongo_connection *con;

	PHP_MONGO_GET_LINK(getThis());
	con = php_mongo_connect(link, MONGO_CON_FLAG_READ TSRMLS_CC);
	if (!con) {
		/* We have to return here, as otherwise the exception doesn't trigger
		 * before we return the hash at the end. */
		return;
	}

	RETURN_STRING(con->hash, 1);
}

PHP_METHOD(Mongo, setSlaveOkay)
{
	zend_bool slave_okay = 1;
	mongoclient *link;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &slave_okay) == FAILURE) {
		return;
	}

	PHP_MONGO_GET_LINK(getThis());

	RETVAL_BOOL(link->servers->read_pref.type != MONGO_RP_PRIMARY);
	link->servers->read_pref.type = slave_okay ? MONGO_RP_SECONDARY_PREFERRED : MONGO_RP_PRIMARY;
}


static void run_err(int err_type, zval *return_value, zval *this_ptr TSRMLS_DC)
{
	zval *db_name, *db;
	MAKE_STD_ZVAL(db_name);
	ZVAL_STRING(db_name, "admin", 1);

	MAKE_STD_ZVAL(db);
	MONGO_METHOD1(MongoClient, selectDB, db, getThis(), db_name);
	zval_ptr_dtor(&db_name);

	switch (err_type) {
		case LAST_ERROR:
			MONGO_METHOD(MongoDB, lastError, return_value, db);
			break;
		case PREV_ERROR:
			MONGO_METHOD(MongoDB, prevError, return_value, db);
			break;
		case RESET_ERROR:
			MONGO_METHOD(MongoDB, resetError, return_value, db);
			break;
		case FORCE_ERROR:
			MONGO_METHOD(MongoDB, forceError, return_value, db);
			break;
	}

	zval_ptr_dtor(&db);
}

/* {{{ Mongo->lastError() */
PHP_METHOD(Mongo, lastError)
{
	run_err(LAST_ERROR, return_value, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ Mongo->prevError() */
PHP_METHOD(Mongo, prevError)
{
	run_err(PREV_ERROR, return_value, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ Mongo->resetError() */
PHP_METHOD(Mongo, resetError)
{
	run_err(RESET_ERROR, return_value, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ Mongo->forceError() */
PHP_METHOD(Mongo, forceError)
{
	run_err(FORCE_ERROR, return_value, getThis() TSRMLS_CC);
}
/* }}} */

/* {{{ Mongo->connectUtil */
PHP_METHOD(Mongo, connectUtil)
{
	mongoclient *link;

	PHP_MONGO_GET_LINK(getThis());
	php_mongo_connect(link, MONGO_CON_FLAG_READ TSRMLS_CC);
}
/* }}} */

/* {{{ Mongo->switchSlave */
PHP_METHOD(Mongo, switchSlave)
{
	zim_Mongo_getSlave(INTERNAL_FUNCTION_PARAM_PASSTHRU);
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
