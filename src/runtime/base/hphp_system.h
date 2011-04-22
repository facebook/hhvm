/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/macros.h>
#include <runtime/base/zend/zend_functions.h>
#include <util/exception.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/comparisons.h>
#include <runtime/base/string_offset.h>
#include <runtime/base/util/smart_object.h>
#include <runtime/base/list_assignment.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/variable_table.h>
#include <runtime/base/string_util.h>
#include <util/util.h>
#include <runtime/base/file/plain_file.h>
#include <runtime/base/class_info.h>
#include <runtime/base/externals.h>
#include <runtime/base/class_statics.h>
#include <runtime/base/dynamic_object_data.h>
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/util/simple_counter.h>
#include <util/shm_counter.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Globals : public LVariableTable {
public:
  Globals() : FVF(__autoload)(false) {}
  CVarRef declareConstant(CStrRef name, Variant &constant, CVarRef value);
  void declareFunction(const char *name);
  void declareFunctionLit(CStrRef name);
  bool defined(CStrRef name);
  Variant getConstant(CStrRef name);
  Array getDynamicConstants() const;
  bool function_exists(CStrRef name);

  virtual bool class_exists(CStrRef name);

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

  bool isHead(ssize_t pos) const;
  bool isTail(ssize_t pos) const;

  virtual void getFullPos(FullPos &pos);
  virtual bool setFullPos(const FullPos &pos);

  virtual Array getDefinedVars();

  /**
   * Marshaling/Unmarshaling between request thread and fiber thread.
   * Called by generated fiber_un/marshal_global_state().
   */
  void fiberMarshal(Globals *src, FiberReferenceMap &refMap);
  void fiberUnmarshal(Globals *src, FiberReferenceMap &refMap);

public:
  Variant __lvalProxy;
  Variant __realPropProxy;
  bool FVF(__autoload);

private:
  Array m_dynamicConstants;  // declared constants
  Array m_volatileFunctions; // declared functions

  ssize_t wrapIter(ssize_t it) const;
};

const char* getHphpCompilerVersion();
const char* getHphpCompilerId();

///////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#define DECLARE_SYSTEM_GLOBALS(sg)                      \
  SystemGlobals *sg ATTRIBUTE_UNUSED =       \
    (SystemGlobals*)get_global_variables();

#define DECLARE_GLOBAL_VARIABLES(g)                     \
  GlobalVariables *g ATTRIBUTE_UNUSED =      \
    get_global_variables();

// code generated file that defines all system global variables
#include <system/gen/sys/system_globals.h>

///////////////////////////////////////////////////////////////////////////////

#endif // __CPP_BASE_HPHP_SYSTEM_H__
