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

zend_class_entry *mongo_ce_Date = NULL;
zend_object_handlers mongo_date_handlers;

typedef struct {
	zend_object std;
	int64_t     datetime;
} mongo_date;

void php_mongo_date_init(zval *value, int64_t datetime TSRMLS_DC)
{
	mongo_date *date;
	long        sec, usec;

	date = (mongo_date*) zend_object_store_get_object(value TSRMLS_CC);

	/* Store untouched full datetimestamp */
	date->datetime = datetime;
	
	/* Store it as properties too */
	usec = (long) ((((datetime * 1000) % 1000000) + 1000000) % 1000000);
	sec  = (long) ((datetime/1000) - (datetime < 0 && usec));

	zend_update_property_long(mongo_ce_Date, value, "sec", strlen("sec"), sec TSRMLS_CC);
	zend_update_property_long(mongo_ce_Date, value, "usec", strlen("usec"), usec TSRMLS_CC);
}

/* {{{ MongoDate::__construct
 */
PHP_METHOD(MongoDate, __construct)
{
	long arg1 = 0, arg2 = 0;
	mongo_date *date;
	int64_t internal_date = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ll", &arg1, &arg2) == FAILURE) {
		return;
	}

	switch (ZEND_NUM_ARGS()) {
		case 2:
			zend_update_property_long(mongo_ce_Date, getThis(), "usec", strlen("usec"), (arg2 / 1000) * 1000 TSRMLS_CC);
			internal_date += (arg2 / 1000);
			/* fallthrough */

		case 1:
			zend_update_property_long(mongo_ce_Date, getThis(), "sec", strlen("sec"), arg1 TSRMLS_CC);
			/* usec is already 0, if not set above */
			internal_date += (arg1 * 1000);
			break;

		case 0: {
#ifdef WIN32
			time_t sec = time(0);
			zend_update_property_long(mongo_ce_Date, getThis(), "sec", strlen("sec"), sec TSRMLS_CC);
			zend_update_property_long(mongo_ce_Date, getThis(), "usec", strlen("usec"), 0 TSRMLS_CC);
			internal_date = sec * 1000;
#else
			struct timeval time;
			gettimeofday(&time, NULL);

			zend_update_property_long(mongo_ce_Date, getThis(), "sec", strlen("sec"), time.tv_sec TSRMLS_CC);
			zend_update_property_long(mongo_ce_Date, getThis(), "usec", strlen("usec"), (time.tv_usec / 1000) * 1000 TSRMLS_CC);
			internal_date = (time.tv_sec * 1000) + (time.tv_usec / 1000);
#endif
		}
	}

	date = (mongo_date*) zend_object_store_get_object(getThis() TSRMLS_CC);
	date->datetime = internal_date;
}
/* }}} */


/* {{{ MongoDate::__toString()
 */
PHP_METHOD(MongoDate, __toString)
{
	mongo_date *date;
	int64_t     sec;
	int64_t     usec;
	double      dusec;
	char       *str;

	date = (mongo_date*) zend_object_store_get_object(getThis() TSRMLS_CC);

	usec  = (int64_t) ((((date->datetime * 1000) % 1000000) + 1000000) % 1000000);
	sec   = (int64_t) ((date->datetime/1000) - (date->datetime < 0 && usec));
	dusec = (double) usec / 1000000;

#ifdef WIN32
	spprintf(&str, 0, "%.8f %I64d", dusec, (int64_t) sec);
#else
	spprintf(&str, 0, "%.8f %lld", dusec, (long long int) sec);
#endif

	RETURN_STRING(str, 0);
}
/* }}} */


static zend_function_entry MongoDate_methods[] = {
	PHP_ME(MongoDate, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoDate, __toString, NULL, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

/* {{{ php_mongo_date_free
 */
static void php_mongo_date_free(void *object TSRMLS_DC)
{
	mongo_date *date = (mongo_date*)object;

	zend_object_std_dtor(&date->std TSRMLS_CC);

	efree(date);
}
/* }}} */

static zend_object_value php_mongodate_new(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;
	mongo_date *intern;

	intern = (mongo_date*)emalloc(sizeof(mongo_date));
	memset(intern, 0, sizeof(mongo_date));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	init_properties(intern);

	retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t) zend_objects_destroy_object, php_mongo_date_free, NULL TSRMLS_CC);
	retval.handlers = &mongo_date_handlers;

	return retval;
}

void mongo_init_MongoDate(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoDate", MongoDate_methods);
	ce.create_object = php_mongodate_new;
	mongo_ce_Date = zend_register_internal_class(&ce TSRMLS_CC);
	memcpy(&mongo_date_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	zend_declare_property_long(mongo_ce_Date, "sec", strlen("sec"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(mongo_ce_Date, "usec", strlen("usec"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
