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
class RenamedFuncDict {
private:

  // Names to Func's.
  typedef hphp_hash_map<const StringData*, Func*,
          string_data_hash, string_data_isame> FuncMap;

  typedef hphp_hash_set<const StringData*, string_data_hash, string_data_isame>
          NameSet;

  // Some PHP programs opt into restricting the use of function renaming.
  bool m_restrictRenameableFunctions;
  NameSet m_renameableFunctions;

 public:
  RenamedFuncDict();

  bool rename(const StringData* old, const StringData* n3w);
  bool isFunctionRenameable(const StringData* name);
  void addRenameableFunctions(ArrayData* arr);

  //Array getUserFunctions();
};

}
} // HPHP::VM

#endif
