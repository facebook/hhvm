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

#ifndef ZEND_INI_H
#define ZEND_INI_H

#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/thread-init-fini.h"

BEGIN_EXTERN_C()
ZEND_API inline void display_ini_entries(zend_module_entry *module) {}
END_EXTERN_C()

#define ZEND_INI_BEGIN()    void init_ini_entries() {
#define ZEND_INI_END()      };

#define ZEND_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, arg1, arg2, arg3, displayer) \
  HPHP::IniSetting::Bind(name, default_value, on_modify, (int)arg1 + (char*)arg2);

#define ZEND_INI_ENTRY3(name, default_value, modifiable, on_modify, arg1, arg2, arg3) \
  ZEND_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, arg1, arg2, arg3, NULL)

#define ZEND_INI_ENTRY2_EX(name, default_value, modifiable, on_modify, arg1, arg2, displayer) \
  ZEND_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, arg1, arg2, NULL, displayer)

#define ZEND_INI_ENTRY2(name, default_value, modifiable, on_modify, arg1, arg2) \
  ZEND_INI_ENTRY2_EX(name, default_value, modifiable, on_modify, arg1, arg2, NULL)

#define ZEND_INI_ENTRY1_EX(name, default_value, modifiable, on_modify, arg1, displayer) \
  ZEND_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, arg1, NULL, NULL, displayer)

#define ZEND_INI_ENTRY1(name, default_value, modifiable, on_modify, arg1) \
  ZEND_INI_ENTRY1_EX(name, default_value, modifiable, on_modify, arg1, NULL)

#define ZEND_INI_ENTRY_EX(name, default_value, modifiable, on_modify, displayer) \
  ZEND_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, NULL, NULL, NULL, displayer)

#define ZEND_INI_ENTRY(name, default_value, modifiable, on_modify) \
  ZEND_INI_ENTRY_EX(name, default_value, modifiable, on_modify, NULL)

#define STD_ZEND_INI_ENTRY(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr) \
  ZEND_INI_ENTRY2(name, default_value, modifiable, on_modify, (void *) XtOffsetOf(struct_type, property_name), (void *) &struct_ptr)
#define STD_ZEND_INI_ENTRY_EX(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr, displayer) \
  ZEND_INI_ENTRY2_EX(name, default_value, modifiable, on_modify, (void *) XtOffsetOf(struct_type, property_name), (void *) &struct_ptr, displayer)
#define STD_ZEND_INI_BOOLEAN(name, default_value, modifiable, on_modify, property_name, struct_type, struct_ptr) \
  ZEND_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, (void *) XtOffsetOf(struct_type, property_name), (void *) &struct_ptr, NULL, zend_ini_boolean_displayer_cb)

#define REGISTER_INI_ENTRIES()                                          \
  static HPHP::InitFiniNode init(init_ini_entries,                      \
    HPHP::InitFiniNode::When::ThreadInit);
#define UNREGISTER_INI_ENTRIES()
#define DISPLAY_INI_ENTRIES() display_ini_entries(zend_module)

/* Standard message handlers */
#define OnUpdateBool HPHP::ini_on_update_bool
#define OnUpdateLong HPHP::ini_on_update_long
#define OnUpdateLongGEZero HPHP::ini_on_update_non_negative
#define OnUpdateReal HPHP::ini_on_update_real
#define OnUpdateString HPHP::ini_on_update_string
#define OnUpdateStringUnempty HPHP::ini_on_update_string_non_empty


#endif /* ZEND_INI_H */
