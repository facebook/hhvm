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

#include "../php_mongo.h"
#include "log.h"

zend_class_entry *mongo_ce_Log;
ZEND_EXTERN_MODULE_GLOBALS(mongo)

static long set_value(char *setting, zval *return_value TSRMLS_DC);
static void get_value(char *setting, zval *return_value TSRMLS_DC);
#if PHP_VERSION_ID >= 50300
static void userland_callback(int module, int level, char *message TSRMLS_DC);
#endif


static zend_function_entry mongo_log_methods[] = {
	PHP_ME(MongoLog, setLevel, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(MongoLog, getLevel, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(MongoLog, setModule, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(MongoLog, getModule, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#if PHP_VERSION_ID >= 50300
	PHP_ME(MongoLog, setCallback, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(MongoLog, getCallback, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
	{NULL, NULL, NULL}
};

void mongo_init_MongoLog(TSRMLS_D) {
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoLog", mongo_log_methods);
	mongo_ce_Log = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_class_constant_long(mongo_ce_Log, "NONE", strlen("NONE"), MLOG_NONE TSRMLS_CC);

	zend_declare_class_constant_long(mongo_ce_Log, "WARNING", strlen("WARNING"), MLOG_WARN TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_Log, "INFO", strlen("INFO"), MLOG_INFO TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_Log, "FINE", strlen("FINE"), MLOG_FINE TSRMLS_CC);

	zend_declare_class_constant_long(mongo_ce_Log, "RS", strlen("RS"), MLOG_RS TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_Log, "POOL", strlen("POOL"), MLOG_RS TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_Log, "PARSE", strlen("PARSE"), MLOG_PARSE TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_Log, "CON", strlen("CON"), MLOG_CON TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_Log, "IO", strlen("IO"), MLOG_IO TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_Log, "SERVER", strlen("SERVER"), MLOG_SERVER TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_Log, "ALL", strlen("ALL"), MLOG_ALL TSRMLS_CC);

	zend_declare_property_long(mongo_ce_Log, "level", strlen("level"), 0, ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_long(mongo_ce_Log, "module", strlen("module"), 0, ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_long(mongo_ce_Log, "callback", strlen("callback"), 0, ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
}

static long set_value(char *setting, zval *return_value TSRMLS_DC) {
	long value;

	if (zend_parse_parameters(1 TSRMLS_CC, "l", &value) == FAILURE) {
		return 0;
	}

	zend_update_static_property_long(mongo_ce_Log, setting, strlen(setting), value TSRMLS_CC);

	return value;
}

static void get_value(char *setting, zval *return_value TSRMLS_DC) {
	zval *value;

	value = zend_read_static_property(mongo_ce_Log, setting, strlen(setting), NOISY TSRMLS_CC);

	ZVAL_LONG(return_value, Z_LVAL_P(value));
}

PHP_METHOD(MongoLog, setLevel)
{
	MonGlo(log_level) = set_value("level", return_value TSRMLS_CC);
}

PHP_METHOD(MongoLog, getLevel)
{
	get_value("level", return_value TSRMLS_CC);
}

PHP_METHOD(MongoLog, setModule)
{
	MonGlo(log_module) = set_value("module", return_value TSRMLS_CC);
}

#if PHP_VERSION_ID >= 50300
static void userland_callback(int module, int level, char *message TSRMLS_DC)
{
	zval **params[3];
	zval *z_module, *z_level, *z_message, *z_retval = NULL;


	ALLOC_INIT_ZVAL(z_module);
	ZVAL_LONG(z_module, module);
	params[0] = &z_module;

	ALLOC_INIT_ZVAL(z_level);
	ZVAL_LONG(z_level, level);
	params[1] = &z_level;

	ALLOC_INIT_ZVAL(z_message);
	ZVAL_STRING(z_message, message, 1);
	params[2] = &z_message;

	MonGlo(log_callback_info).param_count = 3;
	MonGlo(log_callback_info).params = params;
	MonGlo(log_callback_info).retval_ptr_ptr = &z_retval;

	if (SUCCESS == zend_call_function(&MonGlo(log_callback_info), &MonGlo(log_callback_info_cache) TSRMLS_CC)) {
		zval_ptr_dtor(&z_retval);
	}

	zval_ptr_dtor(&z_message);
	zval_ptr_dtor(&z_level);
	zval_ptr_dtor(&z_module);
}

PHP_METHOD(MongoLog, setCallback)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f/", &MonGlo(log_callback_info), &MonGlo(log_callback_info_cache)) == FAILURE) {
		return;
	}
	zend_update_static_property(mongo_ce_Log, "callback", strlen("callback"), MonGlo(log_callback_info).function_name TSRMLS_CC);

	RETURN_TRUE;
}

PHP_METHOD(MongoLog, getCallback)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	if (MonGlo(log_callback_info).function_name) {
		RETURN_ZVAL(MonGlo(log_callback_info).function_name, 1, 0);
	} else {
		RETURN_FALSE;
	}
}
#endif

PHP_METHOD(MongoLog, getModule)
{
	get_value("module", return_value TSRMLS_CC);
}

static char *level_name(int level)
{
	switch (level) {
		case MLOG_WARN: return "WARN";
		case MLOG_INFO: return "INFO";
		case MLOG_FINE: return "FINE";
		default: return "?";
	}
}

static char *module_name(int module)
{
	switch (module) {
		case MLOG_RS: return "REPLSET";
		case MLOG_CON: return "CON    ";
		case MLOG_IO: return "IO     ";
		case MLOG_SERVER: return "SERVER ";
		case MLOG_PARSE: return "PARSE  ";
		default: return "?";
	}
}

void php_mongo_log(const int module, const int level TSRMLS_DC, const char *format, ...)
{
	if ((module & MonGlo(log_module)) && (level & MonGlo(log_level))) {
		va_list  args;
		char    *tmp = (char*) malloc(256);

		va_start(args, format);
		vsnprintf(tmp, 256, format, args);
		va_end(args);

#if PHP_VERSION_ID >= 50300
		if (MonGlo(log_callback_info).function_name) {
			userland_callback(module, level, tmp TSRMLS_CC);
		} else {
#endif
			php_error(E_NOTICE, "%s %s: %s", module_name(module), level_name(level), tmp);
#if PHP_VERSION_ID >= 50300
		}
#endif

		free(tmp);
	}
}

void php_mcon_log_wrapper(int module, int level, void *context, char *format, va_list args)
{
	TSRMLS_FETCH_FROM_CTX(context);

	if ((module & MonGlo(log_module)) && (level & MonGlo(log_level))) {
		va_list  tmp_args;
		char    *tmp = (char*) malloc(256);

		va_copy(tmp_args, args);
		vsnprintf(tmp, 256, format, tmp_args);
		va_end(tmp_args);

#if PHP_VERSION_ID >= 50300
		if (MonGlo(log_callback_info).function_name) {
			userland_callback(module, level, tmp TSRMLS_CC);
		} else {
#endif
			php_error(E_NOTICE, "%s %s: %s", module_name(module), level_name(level), tmp);
#if PHP_VERSION_ID >= 50300
		}
#endif

		free(tmp);
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
