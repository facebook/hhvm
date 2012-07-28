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

#ifndef __FUNCTION_CONTAINER_H__
#define __FUNCTION_CONTAINER_H__

#include <compiler/hphp.h>
#include <compiler/util/jump_table.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class CodeGenerator;
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(FunctionScope);
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
  void countReturnTypes(std::map<std::string, int> &counts,
                        const StringToFunctionScopePtrVecMap *redec);

  /**
   * Code generation functions.
   */
  void outputCPPJumpTable(CodeGenerator &cg, AnalysisResultPtr ar,
                          const StringToFunctionScopePtrVecMap *redec);
  const StringToFunctionScopePtrMap &getFunctions() const {
    return m_functions;
  }
  void getFunctionsFlattened(const StringToFunctionScopePtrVecMap *redec,
                             FunctionScopePtrVec &funcs,
                             bool excludePseudoMains = false) const;
  void outputCPPCodeInfoTable(
    CodeGenerator &cg, AnalysisResultPtr ar, bool useSwitch,
    const StringToFunctionScopePtrMap &functions);

protected:
  // name => functions. Order of declaration
  StringToFunctionScopePtrMap m_functions;
  void outputCPPJumpTableSupport(CodeGenerator &cg, AnalysisResultPtr ar,
                                 const StringToFunctionScopePtrVecMap *redec,
                                 bool &hasRedeclared,
                                 std::vector<const char *> *funcs = NULL);
  void outputCPPJumpTableEvalSupport(
    CodeGenerator &cg, AnalysisResultPtr ar,
    const StringToFunctionScopePtrVecMap *redec, bool &hasRedeclared);
  void outputCPPCallInfoTableSupport(
    CodeGenerator &cg, AnalysisResultPtr ar,
    const StringToFunctionScopePtrVecMap *redec,
    bool &hasRedeclared, std::vector<const char *> *funcs = NULL);
  void outputCPPJumpTableSupportMethod(CodeGenerator &cg, AnalysisResultPtr ar,
                                       FunctionScopePtr func,
                                       const char *funcPrefix);
  void outputCPPHelperClassAllocSupport(
    CodeGenerator &cg, AnalysisResultPtr ar,
    const StringToFunctionScopePtrVecMap *redec);
private:
  void outputGetCallInfoHeader(CodeGenerator &cg, const char *suffix,
                               bool needGlobals);
  void outputGetCallInfoTail(CodeGenerator &cg, bool system);
  void outputCPPHashTableGetCallInfo(
    CodeGenerator &cg, bool system, bool noEval,
    const StringToFunctionScopePtrMap *functions,
    const std::vector<const char *> &funcs);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __FUNCTION_CONTAINER_H__
