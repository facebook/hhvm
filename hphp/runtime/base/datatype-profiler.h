/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_DATATYPE_PROFILER_H_
#define incl_HPHP_DATATYPE_PROFILER_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/util/stacktrace-profiler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// definitions

struct DataTypeProfiler {
  explicit DataTypeProfiler(std::string name);
  ~DataTypeProfiler();
  DataType operator()(DataType type);
private:
  const std::string m_name;
  StackTraceProfiler m_uninit, m_null, m_boolean, m_int, m_double,
                     m_persistent_string, m_string,
                     m_persistent_darray, m_darray,
                     m_persistent_varray, m_varray,
                     m_persistent_array, m_array,
                     m_persistent_vec, m_vec,
                     m_persistent_dict, m_dict,
                     m_persistent_keyset, m_keyset,
                     m_object, m_resource, m_func, m_class, m_clsmeth,
                     m_record;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_DATATYPE_PROFILER_H_
