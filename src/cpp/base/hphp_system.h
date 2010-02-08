/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __CPP_BASE_HPHP_SYSTEM_H__
#define __CPP_BASE_HPHP_SYSTEM_H__

///////////////////////////////////////////////////////////////////////////////

/**
 * This is the file that's included at top of a code generated system file.
 */

#include <cpp/base/macros.h>
#include <cpp/base/zend/zend_functions.h>
#include <util/exception.h>
#include <util/logger.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/comparisons.h>
#include <cpp/base/string_offset.h>
#include <cpp/base/util/smart_object.h>
#include <cpp/base/array/array_element.h>
#include <cpp/base/list_assignment.h>
#include <cpp/base/resource_data.h>
#include <cpp/base/variable_table.h>
#include <cpp/base/string_util.h>
#include <util/util.h>
#include <cpp/base/file/plain_file.h>
#include <cpp/base/class_info.h>
#include <cpp/base/externals.h>
#include <cpp/base/class_statics.h>
#include <cpp/base/dynamic_object_data.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Globals : public LVariableTable {
public:
  CVarRef declareConstant(const char *name, Variant &constant, CVarRef value) {
    if (!m_dynamicConstants.exists(name)) {
      m_dynamicConstants.set(String(name, CopyString), value);
      constant = value;
    }
    return value;
  }

  void declareFunction(const char *name) {
    m_volatileFunctions.set(String(Util::toLower(name)), true);
  }

  void declareClass(const char *name) {
    m_volatileClasses.set(String(Util::toLower(name)), true);
  }

  void declareInterface(const char *name) {
    m_volatileInterfaces.set(String(Util::toLower(name)), true);
  }

  bool defined(const char *name) {
    return m_dynamicConstants.exists(name);
  }

  Variant getConstant(const char *name) {
    return m_dynamicConstants[name];
  }

  Array getDynamicConstants() const {
    return m_dynamicConstants;
  }

  bool function_exists(const char *name) {
    return m_volatileFunctions.exists(Util::toLower(name).c_str());
  }

  bool class_exists(const char *name) {
    return m_volatileClasses.exists(Util::toLower(name).c_str());
  }

  bool interface_exists(const char *name) {
    return m_volatileInterfaces.exists(Util::toLower(name).c_str());
  }

  virtual Variant getByIdx(ssize_t pos, Variant& k);
  virtual CVarRef getRefByIdx(ssize_t pos, Variant& k);
  virtual ssize_t getIndex(const char* s, int64 prehash) const;
  virtual ssize_t size() const;
  virtual bool empty() const;
  virtual ssize_t staticSize() const;

  ssize_t iter_begin() const;
  ssize_t iter_end() const;
  ssize_t iter_advance(ssize_t prev) const;
  ssize_t iter_rewind(ssize_t prev) const;

  virtual void getFullPos(FullPos &pos);
  virtual bool setFullPos(const FullPos &pos);

  virtual Array getDefinedVars();
public:
  Variant __lvalProxy;

private:
  Array m_dynamicConstants;  // declared constants
  Array m_volatileFunctions; // declared functions
  Array m_volatileClasses;   // declared classes
  Array m_volatileInterfaces; // declared interfaces

  ssize_t wrapIter(ssize_t it) const;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#define DECLARE_SYSTEM_GLOBALS(sg)                      \
  SystemGlobals *sg __attribute__((__unused__)) =       \
    (SystemGlobals*)get_global_variables();

#define DECLARE_GLOBAL_VARIABLES(g)                     \
  GlobalVariables *g __attribute__((__unused__)) =      \
    get_global_variables();

// code generated file that defines all system global variables
#include <lib/system/gen/sys/system_globals.h>

///////////////////////////////////////////////////////////////////////////////

#endif // __CPP_BASE_HPHP_SYSTEM_H__
