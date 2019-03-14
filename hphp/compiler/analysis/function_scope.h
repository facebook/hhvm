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

#ifndef incl_HPHP_FUNCTION_SCOPE_H_
#define incl_HPHP_FUNCTION_SCOPE_H_

#include "hphp/compiler/expression/user_attribute.h"
#include <list>
#include <set>
#include <utility>
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/option.h"

#include "hphp/util/hash-map.h"
#include "hphp/parser/parser.h"

#include "hphp/runtime/base/static-string-table.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Type);
DECLARE_BOOST_TYPES(Construct);
DECLARE_BOOST_TYPES(ExpressionList);
DECLARE_BOOST_TYPES(FileScope);
DECLARE_BOOST_TYPES(AnalysisResult);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(SimpleFunctionCall);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(ParameterExpression);
DECLARE_BOOST_TYPES(MethodStatement);

struct CodeGenerator;

typedef ExpressionPtr (*FunctionOptPtr)(CodeGenerator *cg,
                                        AnalysisResultConstRawPtr ar,
                                        SimpleFunctionCallPtr, int);

typedef std::pair< ParameterExpressionPtr, int >
        ParameterExpressionPtrIdxPair;

typedef std::vector< ParameterExpressionPtrIdxPair >
        ParameterExpressionPtrIdxPairVec;

/**
 * A FunctionScope corresponds to a function declaration. We store all
 * inferred types and analyzed results here, so not to pollute syntax trees.
 */
struct FunctionScope : BlockScope {
  /**
   * User defined functions.
   */
  FunctionScope(AnalysisResultConstRawPtr ar, bool method,
                const std::string &originalName, StatementPtr stmt,
                bool reference, int minParam, int maxParam,
                ModifierExpressionPtr modifiers, int attribute,
                const std::string &docComment,
                FileScopePtr file,
                const std::vector<UserAttributePtr> &attrs,
                bool inPseudoMain = false);

  FunctionScope(FunctionScopePtr orig, AnalysisResultConstRawPtr ar,
                const std::string &originalName,
                StatementPtr stmt, ModifierExpressionPtr modifiers, bool user);

  /**
   * System functions.
   */
  FunctionScope(bool method, const std::string &name, bool reference);
  void setParamCounts(AnalysisResultConstRawPtr ar,
                      int minParam, int numDeclParam);
  void setRefParam(int index);
  void clearInOutParam(int index);
  bool isInOutParam(int index) const;

  bool hasUserAttr(const char *attr) const;
  bool hasMemoize() const;
  bool hasMemoizeLSB() const;

  /**
   * What kind of function this is.
   */
  bool isUserFunction() const { return !m_system && !isNative(); }
  bool isSystem() const { return m_system; }
  bool isPublic() const;
  bool isProtected() const;
  bool isPrivate() const;
  bool isStatic() const;
  bool isAbstract() const;
  bool isNative() const;
  bool isFinal() const;
  bool isMagic() const;
  bool isBuiltin() const override { return !getStmt() || isNative(); }
  bool isRefParam(int index) const;
  bool isRefReturn() const { return m_refReturn;}
  bool isDynamicInvoke() const { return m_dynamicInvoke; }
  void setDynamicInvoke();
  bool mayContainThis();
  bool isClosure() const;
  bool isLambdaClosure() const;
  bool isGenerator() const { return m_generator; }
  void setGenerator(bool f) { m_generator = f; }
  bool isAsync() const { return m_async; }
  void setAsync(bool f) { m_async = f; }

  bool needsLocalThis() const;
  bool hasInOutParams() const;
  bool hasRefParams() const;

  bool isNamed(const char* n) const;
  bool isNamed(const std::string& n) const {
    return isNamed(n.c_str());
  }

  const std::string &getParamName(int index) const;

  void setSystem() {
    m_system = true;
  }

  /**
   * Get/set original name of the function, without case being lowered.
   */
  const std::string &getOriginalName() const;
  void setOriginalName(const std::string &name) { m_scopeName = name; }

  std::string getDocName() const;
  std::string getDocFullName() const;

  /**
   * If class method, returns class::name, otherwise just name.
   */
  std::string getOriginalFullName() const;

  /**
   * Whether this function can take variable number of arguments.
   */
  bool allowsVariableArguments() const;
  bool hasVariadicParam() const;
  bool hasRefVariadicParam() const;
  bool usesVariableArgumentFunc() const;
  bool isReferenceVariableArgument() const;
  void setVariableArgument(int reference);

  /**
   * Whether this function can be constant folded
   */
  bool isFoldable() const;

  bool needsFinallyLocals() const;

  /**
   * Whether this function contains a usage of $this
   */
  bool containsThis() const { return m_containsThis; }
  void setContainsThis(bool f = true);
  bool containsBareThis() const { return m_containsBareThis; }
  bool containsRefThis() const { return m_containsBareThis & 2; }
  void setContainsBareThis(bool f, bool ref = false);
  int getDeclParamCount() const { return m_numDeclParams; }
  int getMaxParamCount() const {
    return hasVariadicParam() ? (m_numDeclParams-1) : m_numDeclParams;
  }

  void setLocalRedeclaring() { m_localRedeclaring = true; }
  bool isLocalRedeclaring() const { return m_localRedeclaring; }

  void setContainsDynamicVar() { m_containsDynamicVar = true; }
  bool containsDynamicVar() const { return m_containsDynamicVar; }

  using UserAttributeMap = hphp_hash_map<
    std::string, ExpressionPtr, string_hashi, string_eqstri
  >;

  UserAttributeMap& userAttributes() { return m_userAttributes;}

  std::vector<ScalarExpressionPtr> getUserAttributeParams(
      const std::string& key);

  /**
   * Override BlockScope::outputPHP() to generate return type.
   */
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) override;

  bool inPseudoMain() const override {
    return m_pseudoMain;
  }

  const StringData* getFatalMessage() const {
    return m_fatal_error_msg;
  }

  void setFatal(const std::string& msg) {
    assert(m_fatal_error_msg == nullptr);
    m_fatal_error_msg = makeStaticString(msg);
    assert(m_fatal_error_msg != nullptr);
  }

  void recordParams();

  void addLocal(const std::string& name);
  bool hasLocal(const std::string& name) const {
    return m_localsSet.count(name);
  }
  const std::unordered_set<std::string>& getLocals() const {
    return m_localsSet;
  }
  std::vector<std::string> getLocalVariableNames();
private:
  void init(AnalysisResultConstRawPtr ar);

  int m_minParam;
  int m_numDeclParams;
  int m_attribute;
  std::vector<std::string> m_paramNames;
  boost::dynamic_bitset<> m_refs;
  boost::dynamic_bitset<> m_inOuts;
  ModifierExpressionPtr m_modifiers;
  UserAttributeMap m_userAttributes;
  std::vector<std::string> m_localsVec;
  std::unordered_set<std::string> m_localsSet;

  unsigned m_method : 1;
  unsigned m_refReturn : 1; // whether it's "function &get_reference()"
  unsigned m_dynamicInvoke : 1;
  unsigned m_pseudoMain : 1;
  unsigned m_system : 1;
  unsigned m_containsThis : 1;     // contains a usage of $this?
  unsigned m_containsBareThis : 2; // $this outside object-context,
                                   // 2 if in reference context
  unsigned m_generator : 1;
  unsigned m_async : 1;
  unsigned m_localRedeclaring : 1;
  unsigned m_containsDynamicVar : 1;

  // holds the fact that defining this function is a fatal error
  const StringData* m_fatal_error_msg = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_FUNCTION_SCOPE_H_
