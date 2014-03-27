/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Author: Zeev Suraski <zeev@zend.com>                                 |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "zend.h"
#include "zend_qsort.h"
#include "zend_API.h"
#include "zend_ini.h"
#include "zend_alloc.h"
#include "zend_operators.h"
#include "zend_strtod.h"

#include "hphp/runtime/base/ini-setting.h"

#define NO_VALUE_PLAINTEXT		"no value"
#define NO_VALUE_HTML			"<i>no value</i>"

ZEND_API int zend_register_ini_entries(const zend_ini_entry *ini_entry, int module_number TSRMLS_DC) /* {{{ */
{
	const zend_ini_entry *p = ini_entry;

  auto extension = ZendExtension::GetByModuleNumber(module_number);
  assert(extension);

	while (p->name) {
    auto updateCallback = [p](const std::string& value) -> bool {
      TSRMLS_FETCH();
      // TODO Who is supposed to free this?
      char* data = estrndup(value.data(), value.size());
      auto ret = p->on_modify(
        const_cast<zend_ini_entry*>(p), data, value.size(),
        p->mh_arg1, p->mh_arg2, p->mh_arg3, ZEND_INI_STAGE_STARTUP TSRMLS_CC
      );
      return ret;
    };
    auto getCallback = [p]() {
      return std::string(p->value, p->value_length);
    };
    HPHP::IniSetting::Mode mode =
      p->modifiable == ZEND_INI_USER   ? HPHP::IniSetting::Mode::PHP_INI_USER :
      p->modifiable == ZEND_INI_PERDIR ? HPHP::IniSetting::Mode::PHP_INI_PERDIR :
      p->modifiable == ZEND_INI_SYSTEM ? HPHP::IniSetting::Mode::PHP_INI_SYSTEM :
                                         HPHP::IniSetting::Mode::PHP_INI_NONE;
    HPHP::IniSetting::Bind(
        extension, mode,
        p->name, p->value,
        HPHP::IniSetting::SetAndGet<std::string>(updateCallback, getCallback));
		p++;
	}
	return SUCCESS;
}

ZEND_API void zend_unregister_ini_entries(int module_number TSRMLS_DC) {
}

/* Standard message handlers */
ZEND_API ZEND_INI_MH(OnUpdateBool) /* {{{ */
{
	zend_bool *p;
#ifndef ZTS
	char *base = (char *) mh_arg2;
#else
	char *base;

	base = (char *) ts_resource(*((int *) mh_arg2));
#endif

	p = (zend_bool *) (base+(size_t) mh_arg1);

	if (new_value_length == 2 && strcasecmp("on", new_value) == 0) {
		*p = (zend_bool) 1;
	}
	else if (new_value_length == 3 && strcasecmp("yes", new_value) == 0) {
		*p = (zend_bool) 1;
	}
	else if (new_value_length == 4 && strcasecmp("true", new_value) == 0) {
		*p = (zend_bool) 1;
	}
	else {
		*p = (zend_bool) atoi(new_value);
	}
	return SUCCESS;
}
/* }}} */

ZEND_API ZEND_INI_MH(OnUpdateLong) /* {{{ */
{
	long *p;
#ifndef ZTS
	char *base = (char *) mh_arg2;
#else
	char *base;

	base = (char *) ts_resource(*((int *) mh_arg2));
#endif

	p = (long *) (base+(size_t) mh_arg1);

	*p = zend_atol(new_value, new_value_length);
	return SUCCESS;
}
/* }}} */

ZEND_API ZEND_INI_MH(OnUpdateLongGEZero) /* {{{ */
{
	long *p, tmp;
#ifndef ZTS
	char *base = (char *) mh_arg2;
#else
	char *base;

	base = (char *) ts_resource(*((int *) mh_arg2));
#endif

	tmp = zend_atol(new_value, new_value_length);
	if (tmp < 0) {
		return FAILURE;
	}

	p = (long *) (base+(size_t) mh_arg1);
	*p = tmp;

	return SUCCESS;
}
/* }}} */

ZEND_API ZEND_INI_MH(OnUpdateReal) /* {{{ */
{
	double *p;
#ifndef ZTS
	char *base = (char *) mh_arg2;
#else
	char *base;

	base = (char *) ts_resource(*((int *) mh_arg2));
#endif

	p = (double *) (base+(size_t) mh_arg1);

	*p = zend_strtod(new_value, NULL);
	return SUCCESS;
}
/* }}} */

ZEND_API ZEND_INI_MH(OnUpdateString) /* {{{ */
{
	char **p;
#ifndef ZTS
	char *base = (char *) mh_arg2;
#else
	char *base;

	base = (char *) ts_resource(*((int *) mh_arg2));
#endif

	p = (char **) (base+(size_t) mh_arg1);

	*p = new_value;
	return SUCCESS;
}
/* }}} */

ZEND_API ZEND_INI_MH(OnUpdateStringUnempty) /* {{{ */
{
	char **p;
#ifndef ZTS
	char *base = (char *) mh_arg2;
#else
	char *base;

	base = (char *) ts_resource(*((int *) mh_arg2));
#endif

	if (new_value && !new_value[0]) {
		return FAILURE;
	}

	p = (char **) (base+(size_t) mh_arg1);

	*p = new_value;
	return SUCCESS;
}

ZEND_API void display_ini_entries(zend_module_entry *module) {
}
