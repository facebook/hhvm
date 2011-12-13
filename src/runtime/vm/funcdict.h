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

#ifndef incl_VM_FUNC_DICT_H_
#define incl_VM_FUNC_DICT_H_

#include <runtime/vm/func.h>

namespace HPHP {
namespace VM {

/*
 * Abstraction around the Name -> Function mapping.
 */
class FuncDict {
public:
  struct InterceptData : public Countable {
    Variant m_handler;
    Variant m_data;
    String m_name;
    InterceptData(CVarRef handler, CVarRef data, CStrRef name)
        : m_handler(handler), m_data(data), m_name(name) {}
    void release() { delete this; }
  };
  typedef SmartPtr<InterceptData> InterceptDataPtr;

private:

  // Names to BuiltinFunction's.
  typedef hphp_hash_map<const StringData*, Func::BuiltinFunction,
          string_data_hash, string_data_isame> ExtFuncMap;

  // Names to Func's.
  typedef hphp_hash_map<const StringData*, Func*,
          string_data_hash, string_data_isame> FuncMap;

  typedef hphp_hash_set<const StringData*, string_data_hash, string_data_isame>
          NameSet;

  // Names to InterceptData.
  typedef hphp_hash_map<const Func*, InterceptDataPtr,
                        pointer_hash<Func> > InterceptDataMap;

  // s_extFuncHash and s_builtinFuncs have Process scope.
  static ExtFuncMap s_extFuncHash;
  static FuncMap s_builtinFuncs;

  // m_funcs: request scope.
  FuncMap m_funcs;

  // m_builtinBlackList: names that should not be looked up in s_builtinFuncs.
  // Request scope.
  NameSet m_builtinBlackList;

  // Some PHP programs opt into restricting the use of function renaming.
  bool m_restrictRenameableFunctions;
  NameSet m_renameableFunctions;

  Func* getBuiltin(const StringData*) const;

  // Maps intercepted names (which can include ::-style method names) to their
  // handlers and "data" parameters.
  InterceptDataMap m_interceptHandlers;

 public:
  FuncDict();

  static void ProcessInit();

  void insert(const StringData* name, Func* val);
  Func* get(const StringData* name) const {
    Func* retval;
    if (mapGet(m_funcs, name, &retval)) {
      return retval;
    }
    return getBuiltin(name);
  }

  bool rename(const StringData* old, const StringData* n3w);
  bool isFunctionRenameable(const StringData* name);
  void addRenameableFunctions(ArrayData* arr);

  bool interceptFunction(CStrRef name, CVarRef handler, CVarRef data);
  bool hasAnyIntercepts();  // does not involve a hashtable lookup
  InterceptDataPtr getInterceptData(const Func* func);

  Array getUserFunctions();
};

}
} // HPHP::VM

#endif
