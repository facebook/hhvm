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

#define NO_VALUE_PLAINTEXT "no value"
#define NO_VALUE_HTML      "<i>no value</i>"

/* {{{ */

namespace HPHP {

struct ZendUserIniData : UserIniData {
  explicit ZendUserIniData (zend_ini_entry *p)
    : UserIniData(), p(p)
  {}

  virtual ~ZendUserIniData () {
    if (p) {
      /*
       * The string p->value was allocated by estrndup, which calls
       * req::malloc.
       * As such, the string will be freed at the end of the request.
       *
       * This destructor is called, for example, in the course
       * of deleting the CallbackMap s_system_ini_callbacks,
       * which happens after the end of the request.
       * So at this point we may not actually own p->value,
       * and its referent may already have been freed.
       *
       * Just be overly fussy here and have us forget about p->value.
       */
      p->value = nullptr;
      p->value_length = -1;
    }
    delete p;
    p = nullptr;
  }

public:
  zend_ini_entry *p;
};

} /* namespace HPHP */

static int hhvm_register_ini_entries_do_work(
  const zend_ini_entry *ini_entry,
  int module_number,
  int acceptable_mode_mask
  TSRMLS_DC)
{
  auto extension = HPHP::ZendExtension::GetByModuleNumber(module_number);
  assert(extension);

  for (const zend_ini_entry *constp = ini_entry; constp->name; constp++) {
    /*
     * Make a copy of the const version of zend_ini_entry.
     * This const version is static const in the compiled php extension.
     *
     * Copy the given (default) value into a freeable string,
     * so that assignments to the setting will consistently free the
     * previous string.
     *
     * This copy will have its ownership in the 1:1 corresponding
     * ZendUserIniData structure.
     */
    zend_ini_entry *p = new zend_ini_entry();
    *p = *constp;
    p->value = estrndup(p->value, p->value_length);

    /*
     * A factory to generate a unique instance of a UserIniData
     * which will eventually end up in either the per-request or system-wide
     * table that maps ini settings to their callbacks.
     */
    auto userDataCallback = [p]() -> HPHP::UserIniData * {
      return new HPHP::ZendUserIniData(p);
    };

    auto updateCallback = [p](const std::string& value) -> bool {
      TSRMLS_FETCH();
      char* data = estrndup(value.data(), value.size());
      int ret = FAILURE;
      /*
       * Store the data as the current value of this extension's ini setting.
       *
       * It should be safe to free the string we previously allocated,
       * since we have not got to the end of the request when the
       * MemoryManager will free these strings.
       */
      efree(p->value);
      p->value = data;
      p->value_length = value.size();
      if (p->on_modify) {
        ret = p->on_modify(
          p, p->value, p->value_length,
          p->mh_arg1, p->mh_arg2, p->mh_arg3, ZEND_INI_STAGE_STARTUP TSRMLS_CC
        );
      }
      return (ret == SUCCESS);
    };

    auto getCallback = [p]() {
      return std::string(p->value, p->value_length);
    };

    int mode_mask = 0;
    assert(HPHP::IniSetting::Mode::PHP_INI_NONE == 0);
    if (p->modifiable & ZEND_INI_USER & acceptable_mode_mask) {
      mode_mask |= HPHP::IniSetting::Mode::PHP_INI_USER;
    }
    if (p->modifiable & ZEND_INI_PERDIR & acceptable_mode_mask) {
      mode_mask |= HPHP::IniSetting::Mode::PHP_INI_PERDIR;
    }
    if (p->modifiable & ZEND_INI_SYSTEM & acceptable_mode_mask) {
      mode_mask |= HPHP::IniSetting::Mode::PHP_INI_SYSTEM;
    }
    if (mode_mask) {
      HPHP::IniSetting::Mode mode = (HPHP::IniSetting::Mode) mode_mask;
      /*
       * Calls HPHP::IniSetting::Bind<std::string> from/near
       *   hphp/runtime/base/ini-setting.h:205
       * The default value is p->value, which is assigned back
       * by an immediate call to the updateCallback
       *
       */
      HPHP::IniSetting::Bind(
        extension, mode,
        p->name, p->value,
        HPHP::IniSetting::SetAndGet<std::string>(
          updateCallback,
          getCallback,
          userDataCallback));
    }
  }
  return SUCCESS;
}

ZEND_API int zend_register_ini_entries_of_certain_mode(
  const zend_ini_entry *ini_entry,
  int module_number,
  int acceptable_mode_mask
  TSRMLS_DC)
{
  int code = 0;
  code = hhvm_register_ini_entries_do_work(
    ini_entry,
    module_number,
    acceptable_mode_mask
    TSRMLS_CC);
  return code;
}

ZEND_API int zend_register_ini_entries(
  const zend_ini_entry *ini_entry,
  int module_number TSRMLS_DC)
{
  int acceptable_mode_mask = 0;
  acceptable_mode_mask |= ZEND_INI_USER;
  acceptable_mode_mask |= ZEND_INI_PERDIR;
  acceptable_mode_mask |= ZEND_INI_SYSTEM;
  return zend_register_ini_entries_of_certain_mode(
    ini_entry,
    module_number,
    acceptable_mode_mask
    TSRMLS_CC);
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
