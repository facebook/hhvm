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

#ifndef incl_HPHP_FUNCTION_SCOPE_H_
#define incl_HPHP_FUNCTION_SCOPE_H_

#include "hphp/compiler/expression/user_attribute.h"
#include <list>
#include <set>
#include <utility>
#include <vector>
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/json.h"

#include "hphp/util/hash-map-typedefs.h"
#include "hphp/parser/parser.h"

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

class CodeGenerator;

typedef ExpressionPtr (*FunctionOptPtr)(CodeGenerator *cg,
                                        AnalysisResultConstPtr ar,
                                        SimpleFunctionCallPtr, int);

typedef std::pair< ParameterExpressionPtr, int >
        ParameterExpressionPtrIdxPair;

typedef std::vector< ParameterExpressionPtrIdxPair >
        ParameterExpressionPtrIdxPairVec;

/**
 * A FunctionScope corresponds to a function declaration. We store all
 * inferred types and analyzed results here, so not to pollute syntax trees.
 */
class FunctionScope : public BlockScope,
                      public JSON::CodeError::ISerializable,
                      public JSON::DocTarget::ISerializable {
public:
  /**
   * User defined functions.
   */
  FunctionScope(AnalysisResultConstPtr ar, bool method,
                const std::string &originalName, StatementPtr stmt,
                bool reference, int minParam, int maxParam,
                ModifierExpressionPtr modifiers, int attribute,
                const std::string &docComment,
                FileScopePtr file,
                const std::vector<UserAttributePtr> &attrs,
                bool inPseudoMain = false);

  FunctionScope(FunctionScopePtr orig, AnalysisResultConstPtr ar,
                const std::string &originalName,
                StatementPtr stmt, ModifierExpressionPtr modifiers, bool user);

  /**
   * System functions.
   */
  FunctionScope(bool method, const std::string &name, bool reference);
  void setParamCounts(AnalysisResultConstPtr ar,
                      int minParam, int numDeclParam);
  void setParamName(int index, const std::string &name);
  void setRefParam(int index);
  bool hasRefParam(int max) const;

  void addModifier(int mod);

  bool hasUserAttr(const char *attr) const;

  /**
   * What kind of function this is.
   */
  bool isUserFunction() const { return !m_system && !isNative(); }
  bool isSystem() const { return m_system; }
  bool isDynamic() const { return m_dynamic; }
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
  bool hasImpl() const;
  bool isParamCoerceMode() const;
  bool mayContainThis();
  bool isClosure() const;
  bool isGenerator() const { return m_generator; }
  void setGenerator(bool f) { m_generator = f; }
  bool isAsync() const { return m_async; }
  void setAsync(bool f) { m_async = f; }

  int nextInlineIndex() { return ++m_inlineIndex; }

  bool usesLSB() const { return !m_noLSB; }
  void clearUsesLSB() { m_noLSB = true; }
  bool nextLSB() const { return m_nextLSB; }
  void setNextLSB(bool f) { m_nextLSB = f; }

  void setHasGoto() { m_hasGoto = true; }
  void setHasTry() { m_hasTry = true; }
  bool hasGoto() const { return m_hasGoto; }
  bool hasTry() const { return m_hasTry; }
  unsigned getNewID() { return m_nextID++; }

  bool needsLocalThis() const;

  bool isNamed(const char* n) const;
  bool isNamed(const std::string& n) const {
    return isNamed(n.c_str());
  }

  /**
   * Either __construct or a class-name constructor.
   */
  bool isConstructor(ClassScopePtr cls) const;

  const std::string &getParamName(int index) const;

//  const std::string &name() const {
//    return getName();
//  }

  int getRedeclaringId() const {
    return m_redeclaring;
  }

  void setDynamic() {
    m_dynamic = true;
  }

  void setSystem() {
    m_system = true;
    m_volatile = false;
  }

  /**
   * Tell this function about another outer scope that contains it.
   */
  void addClonedTraitOuterScope(FunctionScopePtr scope) {
    m_clonedTraitOuterScope.push_back(scope);
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
  bool usesVariableArgumentFunc() const;
  bool isReferenceVariableArgument() const;
  void setVariableArgument(int reference);

  /**
   * Whether this function has no side effects
   */
  bool hasEffect() const;
  void setNoEffect();

  /**
   * Whether this function can be constant folded
   */
  bool isFoldable() const;
  void setIsFoldable();

  /*
   * If this is a builtin function and does not need an ActRec
   */
  bool noFCallBuiltin() const;
  void setNoFCallBuiltin();

  /*
   * If this is a builtin (C++ or PHP) and can be redefined
   */
  bool allowOverride() const;
  void setAllowOverride();

  bool needsFinallyLocals() const;

  /**
   * Whether this function is a runtime helper function
   */
  void setHelperFunction();

  /**
   * Whether this function returns reference or has reference parameters.
   */
  bool containsReference() const;

  /**
   * Whether this function contains a usage of $this
   */
  bool containsThis() const { return m_containsThis;}
  void setContainsThis(bool f = true);
  bool containsBareThis() const { return m_containsBareThis; }
  bool containsRefThis() const { return m_containsBareThis & 2; }
  void setContainsBareThis(bool f, bool ref = false);
  /**
   * How many parameters a caller should provide.
   */
  int getMinParamCount() const { return m_minParam; }
  int getDeclParamCount() const { return m_numDeclParams; }
  int getMaxParamCount() const {
    return hasVariadicParam() ? (m_numDeclParams-1) : m_numDeclParams;
  }
  int getOptionalParamCount() const { return getMaxParamCount() - m_minParam;}

  void setOptFunction(FunctionOptPtr fn) { m_optFunction = fn; }
  FunctionOptPtr getOptFunction() const { return m_optFunction; }

  /**
   * Whether this is a virtual function that needs dynamic dispatch
   */
  void setVirtual() { m_virtual = true;}
  bool isVirtual() const { return m_virtual;}
  void setHasOverride() { m_hasOverride = true; }
  bool hasOverride() const { return m_hasOverride; }

  /**
   * Whether same function name was declared twice or more.
   */
  void setRedeclaring(int redecId) {
    m_redeclaring = redecId;
    setVolatile(); // redeclared function is also volatile
  }
  bool isRedeclaring() const { return m_redeclaring >= 0;}

  void setLocalRedeclaring() { m_localRedeclaring = true; }
  bool isLocalRedeclaring() const { return m_localRedeclaring; }

  /* For function_exists */
  void setVolatile() { m_volatile = true; }
  bool isVolatile() const { return m_volatile; }
  bool isPersistent() const { return m_persistent; }
  void setPersistent(bool p) { m_persistent = p; }

  /* Indicates if a function may need to use a VarEnv or varargs (aka
   * extraArgs) at run time */
  bool mayUseVV() const;

  typedef hphp_hash_map<std::string, ExpressionPtr, string_hashi,
    string_eqstri> UserAttributeMap;

  UserAttributeMap& userAttributes() { return m_userAttributes;}

  std::vector<std::string> getUserAttributeStringParams(const std::string& key);

  /**
   * Override BlockScope::outputPHP() to generate return type.
   */
  void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) override;
  /**
   * Serialize the iface, not everything.
   */
  void serialize(JSON::CodeError::OutputStream &out) const override;
  void serialize(JSON::DocTarget::OutputStream &out) const override;

  bool inPseudoMain() const override {
    return m_pseudoMain;
  }

  void setClosureVars(ExpressionListPtr closureVars) {
    m_closureVars = closureVars;
  }

  ExpressionListPtr getClosureVars() const {
    return m_closureVars;
  }

  void getClosureUseVars(ParameterExpressionPtrIdxPairVec &useVars,
                         bool filterUsed = true);

  void addCaller(BlockScopePtr caller, bool careAboutReturn = true);
  void addNewObjCaller(BlockScopePtr caller);

  class FunctionInfo {
  public:
    explicit FunctionInfo(int rva = -1)
      : m_maybeStatic(false)
      /*
       * Note: m_maybeRefReturn used to implement an optimization to
       * avoid unbox checks when we call functions where we know no
       * function with that name returns by reference.  This isn't
       * correct, however, because __call can return by reference, so
       * it's disabled here.  (The default to enable it should be
       * 'false'.)
       */
      , m_maybeRefReturn(true)
      , m_refVarArg(rva)
    {}

    bool isRefParam(int p) const {
      if (m_refVarArg >= 0 && p >= m_refVarArg) return true;
      return (m_refParams.find(p) != m_refParams.end());
    }

    void setRefVarArg(int rva) {
      if (rva > m_refVarArg) m_refVarArg = rva;
    }

    void setRefParam(int p) {
      m_refParams.insert(p);
    }

    void setMaybeStatic() { m_maybeStatic = true; }
    bool getMaybeStatic() { return m_maybeStatic; }

    void setMaybeRefReturn() { m_maybeRefReturn = true; }
    bool getMaybeRefReturn() { return m_maybeRefReturn; }

  private:
    bool m_maybeStatic; // this could be a static method
    bool m_maybeRefReturn;
    int m_refVarArg; // -1: no ref varargs;
                     // otherwise, any arg >= m_refVarArg is a reference
    std::set<int> m_refParams; // set of ref arg positions
  };

  using FunctionInfoPtr = std::shared_ptr<FunctionInfo>;
  using StringToFunctionInfoPtrMap = hphp_string_imap<FunctionInfoPtr>;
  static void RecordFunctionInfo(std::string fname, FunctionScopePtr func);
  static FunctionInfoPtr GetFunctionInfo(std::string fname);

private:
  void init(AnalysisResultConstPtr ar);

  static StringToFunctionInfoPtrMap s_refParamInfo;

  int m_minParam;
  int m_numDeclParams;
  int m_attribute;
  std::vector<std::string> m_paramNames;
  std::vector<bool> m_refs;
  ModifierExpressionPtr m_modifiers;
  UserAttributeMap m_userAttributes;

  unsigned m_hasVoid : 1;
  unsigned m_method : 1;
  unsigned m_refReturn : 1; // whether it's "function &get_reference()"
  unsigned m_virtual : 1;
  unsigned m_hasOverride : 1;
  unsigned m_dynamic : 1;
  unsigned m_dynamicInvoke : 1;
  unsigned m_volatile : 1; // for function_exists
  unsigned m_persistent : 1;
  unsigned m_pseudoMain : 1;
  unsigned m_system : 1;
  unsigned m_containsThis : 1; // contains a usage of $this?
  unsigned m_containsBareThis : 2; // $this outside object-context,
                                   // 2 if in reference context
  unsigned m_generator : 1;
  unsigned m_async : 1;
  unsigned m_noLSB : 1;
  unsigned m_nextLSB : 1;
  unsigned m_hasTry : 1;
  unsigned m_hasGoto : 1;
  unsigned m_localRedeclaring : 1;

  int m_redeclaring; // multiple definition of the same function
  int m_inlineIndex;
  FunctionOptPtr m_optFunction;
  ExpressionListPtr m_closureVars;
  ExpressionListPtr m_closureValues;
  unsigned m_nextID; // used when cloning generators for traits
  std::list<FunctionScopeRawPtr> m_clonedTraitOuterScope;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_FUNCTION_SCOPE_H_
