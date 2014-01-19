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

#include "hphp/runtime/base/datatype-profiler.h"

namespace HPHP {

DataTypeProfiler::DataTypeProfiler(std::string name)
  : m_name(name)
  , m_uninit(name + "=KindOfUninit")
  , m_null(name + "=KindOfNull")
  , m_boolean(name + "=KindOfBoolean")
  , m_int(name + "=KindOfInt64")
  , m_double(name + "=KindOfDouble")
  , m_static_string(name + "=KindOfStaticString")
  , m_string(name + "=KindOfString")
  , m_array(name + "=KindOfArray")
  , m_object(name + "=KindOfObject")
  , m_resource(name + "=KindOfResource")
  , m_ref(name + "=KindOfRef")
{}

DataType DataTypeProfiler::operator()(DataType type) {
  switch (type) {
  case KindOfUninit:        m_uninit.count(); break;
  case KindOfNull:          m_null.count(); break;
  case KindOfBoolean:       m_boolean.count(); break;
  case KindOfInt64:         m_int.count(); break;
  case KindOfDouble:        m_double.count(); break;
  case KindOfStaticString:  m_static_string.count(); break;
  case KindOfString:        m_string.count(); break;
  case KindOfArray:         m_array.count(); break;
  case KindOfObject:        m_object.count(); break;
  case KindOfResource:      m_resource.count(); break;
  case KindOfRef:           m_ref.count(); break;
  default: assert(false); break;
  }
  return type;
}

DataTypeProfiler::~DataTypeProfiler() {
  if (!enable_stacktrace_profiler) return;
  auto total = m_uninit.hits() +
               m_null.hits() +
               m_boolean.hits() +
               m_int.hits() +
               m_double.hits() +
               m_static_string.hits() +
               m_string.hits() +
               m_array.hits() +
               m_object.hits() +
               m_resource.hits() +
               m_ref.hits();
  if (!total) return;
  fprintf(stderr, "%s: total=%lu KindOfUninit=%.1f%% "
                  "KindOfNull=%.1f%% "
                  "KindOfBoolean=%.1f%% "
                  "KindOfInt64=%.1f%% "
                  "KindOfDouble=%.1f%% "
                  "KindOfStaticString=%.1f%% "
                  "KindOfString=%.1f%% "
                  "KindOfArray=%.1f%% "
                  "KindOfObject=%.1f%% "
                  "KindOfResource=%.1f%% "
                  "KindOfRef=%.1f%%\n",
          m_name.c_str(), total,
          100.0 * m_uninit.hits() / total,
          100.0 * m_null.hits() / total,
          100.0 * m_boolean.hits() / total,
          100.0 * m_int.hits() / total,
          100.0 * m_double.hits() / total,
          100.0 * m_static_string.hits() / total,
          100.0 * m_string.hits() / total,
          100.0 * m_array.hits() / total,
          100.0 * m_object.hits() / total,
          100.0 * m_resource.hits() / total,
          100.0 * m_ref.hits() / total);
}

}
