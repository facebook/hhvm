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

#include "hphp/runtime/base/datatype-profiler.h"
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

DataTypeProfiler::DataTypeProfiler(std::string name)
  : m_name(name)
  , m_uninit(name + "=KindOfUninit")
  , m_null(name + "=KindOfNull")
  , m_boolean(name + "=KindOfBoolean")
  , m_int(name + "=KindOfInt64")
  , m_double(name + "=KindOfDouble")
  , m_persistent_string(name + "=KindOfPersistentString")
  , m_string(name + "=KindOfString")
  , m_persistent_vec(name + "=KindOfPersistentVec")
  , m_vec(name + "=KindOfVec")
  , m_persistent_dict(name + "=KindOfPersistentDict")
  , m_dict(name + "=KindOfDict")
  , m_persistent_keyset(name + "=KindOfPersistentKeyset")
  , m_keyset(name + "=KindOfKeyset")
  , m_object(name + "=KindOfObject")
  , m_resource(name + "=KindOfResource")
  , m_func(name + "=KindOfFunc")
  , m_rfunc(name + "=KindOfRFunc")
  , m_class(name + "=KindOfClass")
  , m_clsmeth(name + "=KindOfClsMeth")
  , m_rclsmeth(name + "=KindOfRClsMeth")
  , m_record(name + "=KindOfRecord")
  , m_lclass(name + "=KindOfLazyClass")
{}

DataType DataTypeProfiler::operator()(DataType type) {
  switch (type) {
    case KindOfUninit:        m_uninit.count(); break;
    case KindOfNull:          m_null.count(); break;
    case KindOfBoolean:       m_boolean.count(); break;
    case KindOfInt64:         m_int.count(); break;
    case KindOfDouble:        m_double.count(); break;
    case KindOfPersistentString:  m_persistent_string.count(); break;
    case KindOfString:        m_string.count(); break;
    case KindOfPersistentVec:   m_persistent_vec.count(); break;
    case KindOfVec:           m_vec.count(); break;
    case KindOfPersistentDict:   m_persistent_dict.count(); break;
    case KindOfDict:          m_dict.count(); break;
    case KindOfPersistentKeyset: m_persistent_keyset.count(); break;
    case KindOfKeyset:        m_keyset.count(); break;
    case KindOfObject:        m_object.count(); break;
    case KindOfResource:      m_resource.count(); break;
    case KindOfRFunc:         m_rfunc.count(); break;
    case KindOfFunc:          m_func.count(); break;
    case KindOfClass:         m_class.count(); break;
    case KindOfLazyClass:     m_lclass.count(); break;
    case KindOfClsMeth:       m_clsmeth.count(); break;
    case KindOfRClsMeth:      m_rclsmeth.count(); break;
    case KindOfRecord:        m_record.count(); break;
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
               m_persistent_string.hits() +
               m_string.hits() +
               m_persistent_vec.hits() +
               m_vec.hits() +
               m_persistent_dict.hits() +
               m_dict.hits() +
               m_persistent_keyset.hits() +
               m_keyset.hits() +
               m_object.hits() +
               m_resource.hits() +
               m_func.hits() +
               m_rfunc.hits() +
               m_class.hits() +
               m_clsmeth.hits() +
               m_rclsmeth.hits() +
               m_record.hits() +
               m_lclass.hits();
  if (!total) return;
  fprintf(stderr, "%s: total=%zu KindOfUninit=%.1f%% "
                  "KindOfNull=%.1f%% "
                  "KindOfBoolean=%.1f%% "
                  "KindOfInt64=%.1f%% "
                  "KindOfDouble=%.1f%% "
                  "KindOfPersistentString=%.1f%% "
                  "KindOfString=%.1f%% "
                  "KindOfPersistentVec=%.1f%% "
                  "KindOfVec=%.1f%% "
                  "KindOfPersistentDict=%.1f%% "
                  "KindOfDict=%.1f%% "
                  "KindOfPersistentKeyset=%.1f%% "
                  "KindOfKeyset=%.1f%% "
                  "KindOfObject=%.1f%% "
                  "KindOfResource=%.1f%% "
                  "KindOfFunc=%.1f%% "
                  "KindOfRFunc=%.1f%%"
                  "KindOfClass=%.1f%% "
                  "KindOfClsMeth=%.1f%% "
                  "KindOfRClsMeth=%.1f%% "
                  "KindOfRecord=%.1f%% "
                  "KindOfLazyClass=%.1f%% ",
          m_name.c_str(), total,
          100.0 * m_uninit.hits() / total,
          100.0 * m_null.hits() / total,
          100.0 * m_boolean.hits() / total,
          100.0 * m_int.hits() / total,
          100.0 * m_double.hits() / total,
          100.0 * m_persistent_string.hits() / total,
          100.0 * m_string.hits() / total,
          100.0 * m_persistent_vec.hits() / total,
          100.0 * m_vec.hits() / total,
          100.0 * m_persistent_dict.hits() / total,
          100.0 * m_dict.hits() / total,
          100.0 * m_persistent_keyset.hits() / total,
          100.0 * m_keyset.hits() / total,
          100.0 * m_object.hits() / total,
          100.0 * m_resource.hits() / total,
          100.0 * m_func.hits() / total,
          100.0 * m_rfunc.hits() / total,
          100.0 * m_class.hits() / total,
          100.0 * m_clsmeth.hits() / total,
          100.0 * m_rclsmeth.hits() / total,
          100.0 * m_record.hits() / total,
          100.0 * m_lclass.hits() / total);
}

}
