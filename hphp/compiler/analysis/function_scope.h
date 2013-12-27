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

#ifndef incl_HPHP_FUNCTION_SCOPE_H_
#define incl_HPHP_FUNCTION_SCOPE_H_

#include "hphp/compiler/expression/user_attribute.h"
#include "hphp/compiler/analysis/block_scope.h"
#include "hphp/compiler/option.h"

#include "hphp/util/json.h"
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
                const std::string &name, StatementPtr stmt,
                bool reference, int minParam, int maxParam,
                ModifierExpressionPtr modifiers, int attribute,
                const std::string &docComment,
                FileScopePtr file,
                const std::vector<UserAttributePtr> &attrs,
                bool inPseudoMain = false);

  FunctionScope(FunctionScopePtr orig, AnalysisResultConstPtr ar,
                const std::string &name, const std::string &originalName,
                StatementPtr stmt, ModifierExpressionPtr modifiers, bool user);

  /**
   * System functions.
   */
  FunctionScope(bool method, const std::string &name, bool reference);
  void setParamCounts(AnalysisResultConstPtr ar, int minParam, int maxParam);
  void setParamSpecs(AnalysisResultPtr ar);
  void setParamName(int index, const std::string &name);
  void setParamDefault(int index, const char* value, int64_t len,
                       const std::string &text);
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
  bool isRefParam(int index) const;
  bool isRefReturn() const { return m_refReturn;}
  bool isDynamicInvoke() const { return m_dynamicInvoke; }
  void setDynamicInvoke();
  bool hasImpl() const;
  void setDirectInvoke() { m_directInvoke = true; }
  bool hasDirectInvoke() const { return m_directInvoke; }
  bool isZendParamMode() const;
  bool mayContainThis();
  bool isClosure() const;
  bool isGenerator() const { return m_generator; }
  void setGenerator(bool f) { m_generator = f; }
  int allocYieldLabel();
  int getYieldLabelCount() const;
  int getYieldLabelGeneration() const;
  void resetYieldLabelCount();
  bool isAsync() const { return m_async; }
  void setAsync(bool f) { m_async = f; }

  bool needsClassParam();

  void setInlineSameContext(bool f) { m_inlineSameContext = f; }
  bool getInlineSameContext() const { return m_inlineSameContext; }
  void setContextSensitive(bool f) { m_contextSensitive = f; }
  bool getContextSensitive() const { return m_contextSensitive; }
  void setInlineAsExpr(bool f) { m_inlineAsExpr = f; }
  bool getInlineAsExpr() const { return m_inlineAsExpr; }
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

  /**
   * Either __construct or a class-name constructor.
   */
  bool isConstructor(ClassScopePtr cls) const;

  const std::string &getParamName(int index) const;

  const std::string &name() const {
    return getName();
  }

  virtual std::string getId() const;

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
  void setOriginalName(const std::string &name) { m_originalName = name; }

  std::string getDocName() const;
  std::string getDocFullName() const;

  /**
   * If class method, returns class::name, otherwise just name.
   */
  std::string getFullName() const;
  std::string getOriginalFullName() const;

  /**
   * Whether this function can take variable number of arguments.
   */
  bool isVariableArgument() const;
  bool isReferenceVariableArgument() const;
  void setVariableArgument(int reference);
  bool isMixedVariableArgument() const;

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
  bool needsActRec() const;
  void setNeedsActRec();

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
  int getMinParamCount() const { return m_minParam;}
  int getMaxParamCount() const { return m_maxParam;}
  int getOptionalParamCount() const { return m_maxParam - m_minParam;}

  /**
   * What is the inferred type of this function's return.
   * Note that for generators and async functions, this is different
   * from what caller actually gets when calling the function.
   */
  void pushReturnType();
  void setReturnType(AnalysisResultConstPtr ar, TypePtr type);
  TypePtr getReturnType() const {
    return m_prevReturn ? m_prevReturn : m_returnType;
  }
  bool popReturnType();
  void resetReturnType();

  void addRetExprToFix(ExpressionPtr e);
  void clearRetExprs();
  void fixRetExprs();

  void setOptFunction(FunctionOptPtr fn) { m_optFunction = fn; }
  FunctionOptPtr getOptFunction() const { return m_optFunction; }

  /**
   * Whether this is a virtual function that needs to go through invoke().
   * A perfect virtual will be generated as C++ virtual function without
   * going through invoke(), but rather directly generated as obj->foo().
   * "Overriding" is only being used by magic methods, enforcing parameter
   * and return types.
   */
  void setVirtual() { m_virtual = true;}
  bool isVirtual() const { return m_virtual;}
  void setHasOverride() { m_hasOverride = true; }
  bool hasOverride() const { return m_hasOverride; }
  void setPerfectVirtual();
  bool isPerfectVirtual() const { return m_perfectVirtual;}
  void setOverriding(TypePtr returnType, TypePtr param1 = TypePtr(),
                     TypePtr param2 = TypePtr());
  bool isOverriding() const { return m_overriding;}

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

  bool isInlined() const { return m_inlineable; }
  void disableInline() { m_inlineable = false; }

  /* Whether we need to worry about the named return value optimization
     for this function */
  void setNRVOFix(bool flag) { m_nrvoFix = flag; }
  bool getNRVOFix() const { return m_nrvoFix; }

  /* Indicates if a function may need to use a VarEnv or varargs (aka
   * extraArgs) at run time */
  bool mayUseVV() const;

  /**
   * Whether this function matches the specified one with same number of
   * parameters and types and defaults, so to qualify for perfect virtuals.
   */
  bool matchParams(FunctionScopePtr func);

  /**
   * What is the inferred type of this function's parameter at specified
   * index. Returns number of extra arguments to put into ArgumentArray.
   */
  int inferParamTypes(AnalysisResultPtr ar, ConstructPtr exp,
                      ExpressionListPtr params, bool &valid);

  TypePtr setParamType(AnalysisResultConstPtr ar, int index, TypePtr type);
  TypePtr getParamType(int index);
  TypePtr getParamTypeSpec(int index) { return m_paramTypeSpecs[index]; }

  typedef hphp_hash_map<std::string, ExpressionPtr, string_hashi,
    string_eqstri> UserAttributeMap;

  UserAttributeMap& userAttributes() { return m_userAttributes;}

  /**
   * Override BlockScope::outputPHP() to generate return type.
   */
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);
  /**
   * Serialize the iface, not everything.
   */
  void serialize(JSON::CodeError::OutputStream &out) const;
  void serialize(JSON::DocTarget::OutputStream &out) const;

  bool inPseudoMain() const {
    return m_pseudoMain;
  }

  void setMagicMethod() {
    m_magicMethod = true;
  }
  bool isMagicMethod() const {
    return m_magicMethod;
  }

  void setStmtCloned(StatementPtr stmt) {
    m_stmtCloned = stmt;
  }

  void setClosureVars(ExpressionListPtr closureVars) {
    m_closureVars = closureVars;
  }

  ExpressionListPtr getClosureVars() const {
    return m_closureVars;
  }

  void getClosureUseVars(ParameterExpressionPtrIdxPairVec &useVars,
                         bool filterUsed = true);

  bool needsAnonClosureClass(ParameterExpressionPtrVec &useVars);

  bool needsAnonClosureClass(ParameterExpressionPtrIdxPairVec &useVars);

  void addCaller(BlockScopePtr caller, bool careAboutReturn = true);
  void addNewObjCaller(BlockScopePtr caller);

  ReadWriteMutex &getInlineMutex() { return m_inlineMutex; }

  DECLARE_BOOST_TYPES(FunctionInfo);

  static void RecordFunctionInfo(std::string fname, FunctionScopePtr func);

  static FunctionInfoPtr GetFunctionInfo(std::string fname);

  class FunctionInfo {
  public:
    explicit FunctionInfo(int rva = -1)
      : m_maybeStatic(false)
      , m_maybeRefReturn(false)
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

private:
  void init(AnalysisResultConstPtr ar);

  static StringToFunctionInfoPtrMap s_refParamInfo;

  int m_minParam;
  int m_maxParam;
  int m_attribute;
  std::vector<std::string> m_paramNames;
  TypePtrVec m_paramTypes;
  TypePtrVec m_paramTypeSpecs;
  std::vector<std::string> m_paramDefaults;
  std::vector<std::string> m_paramDefaultTexts;
  std::vector<bool> m_refs;
  TypePtr m_returnType;
  TypePtr m_prevReturn;
  ModifierExpressionPtr m_modifiers;
  UserAttributeMap m_userAttributes;

  unsigned m_hasVoid : 1;
  unsigned m_method : 1;
  unsigned m_refReturn : 1; // whether it's "function &get_reference()"
  unsigned m_virtual : 1;
  unsigned m_hasOverride : 1;
  unsigned m_perfectVirtual : 1;
  unsigned m_dynamic : 1;
  unsigned m_dynamicInvoke : 1;
  unsigned m_overriding : 1; // overriding a virtual function
  unsigned m_volatile : 1; // for function_exists
  unsigned m_persistent : 1;
  unsigned m_pseudoMain : 1;
  unsigned m_magicMethod : 1;
  unsigned m_system : 1;
  unsigned m_inlineable : 1;
  unsigned m_containsThis : 1; // contains a usage of $this?
  unsigned m_containsBareThis : 2; // $this outside object-context,
                                   // 2 if in reference context
  unsigned m_nrvoFix : 1;
  unsigned m_inlineAsExpr : 1;
  unsigned m_inlineSameContext : 1;
  unsigned m_contextSensitive : 1;
  unsigned m_directInvoke : 1;
  unsigned m_generator : 1;
  unsigned m_async : 1;
  unsigned m_noLSB : 1;
  unsigned m_nextLSB : 1;
  unsigned m_hasTry : 1;
  unsigned m_hasGoto : 1;
  unsigned m_localRedeclaring : 1;

  int m_redeclaring; // multiple definition of the same function
  StatementPtr m_stmtCloned; // cloned method body stmt
  int m_inlineIndex;
  FunctionOptPtr m_optFunction;
  ExpressionPtrVec m_retExprsToFix;
  ExpressionListPtr m_closureVars;
  ExpressionListPtr m_closureValues;
  ReadWriteMutex m_inlineMutex;
  unsigned m_nextID; // used when cloning generators for traits
  int m_yieldLabelCount; // number of allocated yield labels
  int m_yieldLabelGen;   // generation counter for yield labels
  std::list<FunctionScopeRawPtr> m_clonedTraitOuterScope;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_FUNCTION_SCOPE_H_
