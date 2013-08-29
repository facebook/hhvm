/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
                     m_static_string, m_string, m_array, m_object,
                     m_resource, m_ref;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_DATATYPE_PROFILER_H_
