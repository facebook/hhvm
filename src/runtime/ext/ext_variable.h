/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef __HPHP_VARIABLE_H__
#define __HPHP_VARIABLE_H__

#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// type testing

inline bool f_is_bool(bool    v) { return true;}
inline bool f_is_bool(char    v) { return false;}
inline bool f_is_bool(short   v) { return false;}
inline bool f_is_bool(int     v) { return false;}
inline bool f_is_bool(int64   v) { return false;}
inline bool f_is_bool(double  v) { return false;}
inline bool f_is_bool(litstr  v) { return false;}
inline bool f_is_bool(CStrRef v) { return false;}
inline bool f_is_bool(CArrRef v) { return false;}
inline bool f_is_bool(CObjRef v) { return false;}
inline bool f_is_bool(CVarRef v) { return v.is(KindOfBoolean);}

inline bool f_is_int(bool    v) { return false;}
inline bool f_is_int(char    v) { return true;}
inline bool f_is_int(short   v) { return true;}
inline bool f_is_int(int     v) { return true;}
inline bool f_is_int(int64   v) { return true;}
inline bool f_is_int(double  v) { return false;}
inline bool f_is_int(litstr  v) { return false;}
inline bool f_is_int(CStrRef v) { return false;}
inline bool f_is_int(CArrRef v) { return false;}
inline bool f_is_int(CObjRef v) { return false;}
inline bool f_is_int(CVarRef v) { return v.isInteger();}

inline bool f_is_integer(bool    v) { return false;}
inline bool f_is_integer(char    v) { return true;}
inline bool f_is_integer(short   v) { return true;}
inline bool f_is_integer(int     v) { return true;}
inline bool f_is_integer(int64   v) { return true;}
inline bool f_is_integer(double  v) { return false;}
inline bool f_is_integer(litstr  v) { return false;}
inline bool f_is_integer(CStrRef v) { return false;}
inline bool f_is_integer(CArrRef v) { return false;}
inline bool f_is_integer(CObjRef v) { return false;}
inline bool f_is_integer(CVarRef v) { return v.isInteger();}

inline bool f_is_long(bool    v) { return false;}
inline bool f_is_long(char    v) { return true;}
inline bool f_is_long(short   v) { return true;}
inline bool f_is_long(int     v) { return true;}
inline bool f_is_long(int64   v) { return true;}
inline bool f_is_long(double  v) { return false;}
inline bool f_is_long(litstr  v) { return false;}
inline bool f_is_long(CStrRef v) { return false;}
inline bool f_is_long(CArrRef v) { return false;}
inline bool f_is_long(CObjRef v) { return false;}
inline bool f_is_long(CVarRef v) { return v.isInteger();}

inline bool f_is_double(bool    v) { return false;}
inline bool f_is_double(char    v) { return false;}
inline bool f_is_double(short   v) { return false;}
inline bool f_is_double(int     v) { return false;}
inline bool f_is_double(int64   v) { return false;}
inline bool f_is_double(double  v) { return true;}
inline bool f_is_double(litstr  v) { return false;}
inline bool f_is_double(CStrRef v) { return false;}
inline bool f_is_double(CArrRef v) { return false;}
inline bool f_is_double(CObjRef v) { return false;}
inline bool f_is_double(CVarRef v) { return v.is(KindOfDouble);}

inline bool f_is_float(bool    v) { return false;}
inline bool f_is_float(char    v) { return false;}
inline bool f_is_float(short   v) { return false;}
inline bool f_is_float(int     v) { return false;}
inline bool f_is_float(int64   v) { return false;}
inline bool f_is_float(double  v) { return true;}
inline bool f_is_float(litstr  v) { return false;}
inline bool f_is_float(CStrRef v) { return false;}
inline bool f_is_float(CArrRef v) { return false;}
inline bool f_is_float(CObjRef v) { return false;}
inline bool f_is_float(CVarRef v) { return v.is(KindOfDouble);}

inline bool f_is_numeric(bool    v) { return false;}
inline bool f_is_numeric(char    v) { return true;}
inline bool f_is_numeric(short   v) { return true;}
inline bool f_is_numeric(int     v) { return true;}
inline bool f_is_numeric(int64   v) { return true;}
inline bool f_is_numeric(double  v) { return true;}
inline bool f_is_numeric(litstr  v) { return String(v).isNumeric();}
inline bool f_is_numeric(CStrRef v) { return v.isNumeric();}
inline bool f_is_numeric(CArrRef v) { return false;}
inline bool f_is_numeric(CObjRef v) { return false;}
inline bool f_is_numeric(CVarRef v) { return v.isNumeric(true);}

inline bool f_is_real(bool    v) { return false;}
inline bool f_is_real(char    v) { return false;}
inline bool f_is_real(short   v) { return false;}
inline bool f_is_real(int     v) { return false;}
inline bool f_is_real(int64   v) { return false;}
inline bool f_is_real(double  v) { return true;}
inline bool f_is_real(litstr  v) { return false;}
inline bool f_is_real(CStrRef v) { return false;}
inline bool f_is_real(CArrRef v) { return false;}
inline bool f_is_real(CObjRef v) { return false;}
inline bool f_is_real(CVarRef v) { return v.is(KindOfDouble);}

inline bool f_is_string(bool    v) { return false;}
inline bool f_is_string(char    v) { return false;}
inline bool f_is_string(short   v) { return false;}
inline bool f_is_string(int     v) { return false;}
inline bool f_is_string(int64   v) { return false;}
inline bool f_is_string(double  v) { return false;}
inline bool f_is_string(litstr  v) { return true;}
inline bool f_is_string(CStrRef v) { return true;}
inline bool f_is_string(CArrRef v) { return false;}
inline bool f_is_string(CObjRef v) { return false;}
inline bool f_is_string(CVarRef v) { return v.isString();}

inline bool f_is_scalar(bool    v) { return true;}
inline bool f_is_scalar(char    v) { return true;}
inline bool f_is_scalar(short   v) { return true;}
inline bool f_is_scalar(int     v) { return true;}
inline bool f_is_scalar(int64   v) { return true;}
inline bool f_is_scalar(double  v) { return true;}
inline bool f_is_scalar(litstr  v) { return true;}
inline bool f_is_scalar(CStrRef v) { return true;}
inline bool f_is_scalar(CArrRef v) { return false;}
inline bool f_is_scalar(CObjRef v) { return false;}
inline bool f_is_scalar(CVarRef v) { return v.isScalar();}

inline bool f_is_array(bool    v) { return false;}
inline bool f_is_array(char    v) { return false;}
inline bool f_is_array(short   v) { return false;}
inline bool f_is_array(int     v) { return false;}
inline bool f_is_array(int64   v) { return false;}
inline bool f_is_array(double  v) { return false;}
inline bool f_is_array(litstr  v) { return false;}
inline bool f_is_array(CStrRef v) { return false;}
inline bool f_is_array(CArrRef v) { return true;}
inline bool f_is_array(CObjRef v) { return false;}
inline bool f_is_array(CVarRef v) { return v.is(KindOfArray);}

inline bool f_is_object(bool    v) { return false;}
inline bool f_is_object(char    v) { return false;}
inline bool f_is_object(short   v) { return false;}
inline bool f_is_object(int     v) { return false;}
inline bool f_is_object(int64   v) { return false;}
inline bool f_is_object(double  v) { return false;}
inline bool f_is_object(litstr  v) { return false;}
inline bool f_is_object(CStrRef v) { return false;}
inline bool f_is_object(CArrRef v) { return false;}
inline bool f_is_object(CObjRef v) { return !v.isResource();}
bool f_is_object(CVarRef v);

inline bool f_is_resource(bool    v) { return false;}
inline bool f_is_resource(char    v) { return false;}
inline bool f_is_resource(short   v) { return false;}
inline bool f_is_resource(int     v) { return false;}
inline bool f_is_resource(int64   v) { return false;}
inline bool f_is_resource(double  v) { return false;}
inline bool f_is_resource(litstr  v) { return false;}
inline bool f_is_resource(CStrRef v) { return false;}
inline bool f_is_resource(CArrRef v) { return false;}
inline bool f_is_resource(CObjRef v) { return v.isResource();}
inline bool f_is_resource(CVarRef v) { return v.isResource();}

inline bool f_is_null(bool    v) { return false;}
inline bool f_is_null(char    v) { return false;}
inline bool f_is_null(short   v) { return false;}
inline bool f_is_null(int     v) { return false;}
inline bool f_is_null(int64   v) { return false;}
inline bool f_is_null(double  v) { return false;}
inline bool f_is_null(litstr  v) { return false;}
inline bool f_is_null(CStrRef v) { return v.isNull();}
inline bool f_is_null(CArrRef v) { return v.isNull();}
inline bool f_is_null(CObjRef v) { return v.isNull();}
inline bool f_is_null(CVarRef v) { return v.isNull();}

inline String f_gettype(bool    v) { return "boolean";}
inline String f_gettype(char    v) { return "integer";}
inline String f_gettype(short   v) { return "integer";}
inline String f_gettype(int     v) { return "integer";}
inline String f_gettype(int64   v) { return "integer";}
inline String f_gettype(double  v) { return "double";}
inline String f_gettype(litstr  v) { return "string";}
inline String f_gettype(CStrRef v) { return "string";}
inline String f_gettype(CArrRef v) { return "array";}
inline String f_gettype(CObjRef v) { return "object";}
String f_gettype(CVarRef v);
String f_get_resource_type(CObjRef handle);

///////////////////////////////////////////////////////////////////////////////
// type conversion

inline int64 f_intval(CVarRef v, int64 base = 10) { return v.toInt64(base);}
inline double f_doubleval(CVarRef v) { return v.toDouble();}
inline double f_floatval(CVarRef v) { return v.toDouble();}
inline String f_strval(CVarRef v) { return v.toString();}

bool f_settype(Variant var, CStrRef type);

///////////////////////////////////////////////////////////////////////////////
// input/output

Variant f_print_r(CVarRef expression, bool ret = false);
Variant f_var_export(CVarRef expression, bool ret = false);
void f_var_dump(CVarRef v);
void f_var_dump(int _argc, CVarRef expression, CArrRef _argv = null_array);
void f_debug_zval_dump(CVarRef variable);
String f_serialize(CVarRef value);
Variant f_unserialize(CStrRef str);

///////////////////////////////////////////////////////////////////////////////
// variable table

Array f_get_defined_vars();
Array get_defined_vars(LVariableTable *variables);

bool f_import_request_variables(CStrRef types, CStrRef prefix = "");

#define EXTR_OVERWRITE          0
#define EXTR_SKIP               1
#define EXTR_PREFIX_SAME        2
#define EXTR_PREFIX_ALL         3
#define EXTR_PREFIX_INVALID     4
#define EXTR_PREFIX_IF_EXISTS   5
#define EXTR_IF_EXISTS          6
#define EXTR_REFS               0x100

/**
 * LVariableTable parameter is added by HPHP.
 */
int f_extract(CArrRef var_array, int extract_type = EXTR_OVERWRITE,
              CStrRef prefix = "");
int extract(LVariableTable *variables, CArrRef var_array,
            int extract_type = EXTR_OVERWRITE, String prefix = "");

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_VARIABLE_H__
