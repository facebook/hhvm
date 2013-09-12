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

#define ZEND_INI_USER	(1<<0)
#define ZEND_INI_PERDIR	(1<<1)
#define ZEND_INI_SYSTEM	(1<<2)

#define ZEND_INI_ALL (ZEND_INI_USER|ZEND_INI_PERDIR|ZEND_INI_SYSTEM)

#define ZEND_INI_MH(name) int name(zend_ini_entry *entry, char *new_value, uint new_value_length, void *mh_arg1, void *mh_arg2, void *mh_arg3, int stage TSRMLS_DC)

struct _zend_ini_entry {
	int module_number;
	int modifiable;
	char *name;
	uint name_length;
	ZEND_INI_MH((*on_modify));
	void *mh_arg1;
	void *mh_arg2;
	void *mh_arg3;

	char *value;
	uint value_length;

	char *orig_value;
	uint orig_value_length;
	int orig_modifiable;
	int modified;

	void (*displayer)(zend_ini_entry *ini_entry, int type);
};

BEGIN_EXTERN_C()
ZEND_API int zend_register_ini_entries(const zend_ini_entry *ini_entry, int module_number TSRMLS_DC);
ZEND_API void zend_unregister_ini_entries(int module_number TSRMLS_DC);

ZEND_API inline void display_ini_entries(zend_module_entry *module) {}
END_EXTERN_C()

#define ZEND_INI_BEGIN()		static const zend_ini_entry ini_entries[] = {
#define ZEND_INI_END()		{ 0, 0, NULL, 0, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, 0, 0, NULL } };

#define ZEND_INI_ENTRY3_EX(name, default_value, modifiable, on_modify, arg1, arg2, arg3, displayer) \
	{ 0, modifiable, name, sizeof(name), on_modify, arg1, arg2, arg3, default_value, sizeof(default_value)-1, NULL, 0, 0, 0, displayer },

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

#define REGISTER_INI_ENTRIES() zend_register_ini_entries(ini_entries, module_number TSRMLS_CC)
#define UNREGISTER_INI_ENTRIES() zend_unregister_ini_entries(module_number TSRMLS_CC)
#define DISPLAY_INI_ENTRIES() display_ini_entries(zend_module)

/* Standard message handlers */
BEGIN_EXTERN_C()
ZEND_API ZEND_INI_MH(OnUpdateBool);
ZEND_API ZEND_INI_MH(OnUpdateLong);
ZEND_API ZEND_INI_MH(OnUpdateLongGEZero);
ZEND_API ZEND_INI_MH(OnUpdateReal);
ZEND_API ZEND_INI_MH(OnUpdateString);
ZEND_API ZEND_INI_MH(OnUpdateStringUnempty);
END_EXTERN_C()

#define ZEND_INI_STAGE_STARTUP		(1<<0)
#define ZEND_INI_STAGE_SHUTDOWN		(1<<1)
#define ZEND_INI_STAGE_ACTIVATE		(1<<2)
#define ZEND_INI_STAGE_DEACTIVATE	(1<<3)
#define ZEND_INI_STAGE_RUNTIME		(1<<4)
#define ZEND_INI_STAGE_HTACCESS		(1<<5)

#endif /* ZEND_INI_H */
