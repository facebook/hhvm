/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_FUNCTION_CONTAINER_H_
#define incl_HPHP_FUNCTION_CONTAINER_H_

#include "hphp/compiler/hphp.h"
#include <map>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class CodeGenerator;
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_EXTENDED_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FunctionContainer);

/**
 * Base class of both FileScope and ClassScope that can contain functions.
 */
class FunctionContainer {
public:
  FunctionContainer();

  /**
   * Functions this container has.
   */
  int getFunctionCount() const { return m_functions.size(); }

  const StringToFunctionScopePtrMap &getFunctions() const {
    return m_functions;
  }
  void getFunctionsFlattened(const StringToFunctionScopePtrVecMap *redec,
                             FunctionScopePtrVec &funcs,
                             bool excludePseudoMains = false) const;

protected:
  // name => functions. Order of declaration
  StringToFunctionScopePtrMap m_functions;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_FUNCTION_CONTAINER_H_
