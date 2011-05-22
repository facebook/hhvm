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

#ifndef __FUNCTION_SCOPE_H__
#define __FUNCTION_SCOPE_H__

#include <compiler/analysis/block_scope.h>
#include <util/json.h>
#include <compiler/option.h>

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

class CodeGenerator;

typedef ExpressionPtr (*FunctionOptPtr)(CodeGenerator *cg,
                                        AnalysisResultConstPtr ar,
                                        SimpleFunctionCallPtr, int);

/**
 * A FunctionScope corresponds to a function declaration. We store all
 * inferred types and analyzed results here, so not to pollute syntax trees.
 */
class FunctionScope : public BlockScope,
                      public JSON::ISerializable {
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
                bool inPseudoMain = false);

  /**
   * System functions.
   */
  FunctionScope(bool method, const std::string &name, bool reference);
  void setParamCounts(AnalysisResultConstPtr ar, int minParam, int maxParam);
  void setParamSpecs(AnalysisResultPtr ar);
  void setParamName(int index, const std::string &name);
  void setParamDefault(int index, const std::string &value,
                       const std::string &text);
  void setRefParam(int index);
  bool hasRefParam(int max) const;

  void addModifier(int mod);

  /**
   * What kind of function this is.
   */
  bool isUserFunction() const { return !m_system;}
  bool isDynamic() const { return m_dynamic; }
  bool isPublic() const;
  bool isProtected() const;
  bool isPrivate() const;
  bool isStatic() const;
  bool isAbstract() const;
  bool isFinal() const;
  bool isMagic() const;
  bool isRefParam(int index) const;
  bool isRefReturn() const { return m_refReturn;}
  bool isDynamicInvoke() const { return m_dynamicInvoke; }
  bool hasImpl() const;
  void setDirectInvoke() { m_directInvoke = true; }
  bool hasDirectInvoke() const { return m_directInvoke; }
  bool isGenerator() const;
  void setNeedsRefTemp() { m_needsRefTemp = true; }
  bool needsRefTemp() const { return m_needsRefTemp; }
  void setNeedsCheckMem() { m_needsCheckMem = true; }
  bool needsCheckMem() const { return m_needsCheckMem; }
  void setClosureGenerator() { m_closureGenerator = true; }
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

  /**
   * Either __construct or a class-name constructor.
   */
  bool isConstructor(ClassScopePtr cls) const;

  const std::string &getParamName(int index) const;

  const std::string &name() const {
    return getName();
  }

  virtual std::string getId(CodeGenerator &cg) const;

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
   * Get original name of the function, without case being lowered.
   */
  const std::string &getOriginalName() const;

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
  /**
   * Whether this function is a runtime helper function
   */
  bool isHelperFunction() const;
  void setHelperFunction();

  /**
   * Whether this function returns reference or has reference parameters.
   */
  bool containsReference() const;

  /**
   * Whether this function contains a usage of $this
   */
  bool containsThis() const { return m_containsThis;}
  void setContainsThis(bool f=true) { m_containsThis = f;}
  bool containsBareThis() const { return m_containsBareThis; }
  void setContainsBareThis(bool f=true) { m_containsBareThis = f; }
  /**
   * How many parameters a caller should provide.
   */
  int getMinParamCount() const { return m_minParam;}
  int getMaxParamCount() const { return m_maxParam;}
  int getOptionalParamCount() const { return m_maxParam - m_minParam;}

  /**
   * What is the inferred type of this function's return.
   */
  void pushReturnType();
  void setReturnType(AnalysisResultConstPtr ar, TypePtr type);
  TypePtr getReturnType() const {
    return m_prevReturn ? m_prevReturn : m_returnType;
  }
  void popReturnType();
  void addRetExprToFix(ExpressionPtr e);
  void clearRetExprs();
  void fixRetExprs();

  bool needsTypeCheckWrapper() const;
  const char *getPrefix(ExpressionListPtr params);

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

  /* For function_exists */
  void setVolatile() { m_volatile = true;}
  bool isVolatile() const { return m_volatile;}

  bool isInlined() const { return m_inlineable; }
  void disableInline() { m_inlineable = false; }

  /* Whether this function is brought in by a separable extension */
  void setSepExtension() { m_sep = true;}
  bool isSepExtension() const { return m_sep;}

  /* Whether we need to worry about the named return value optimization
     for this function */
  void setNRVOFix(bool flag) { m_nrvoFix = flag; }
  bool getNRVOFix() const { return m_nrvoFix; }

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

  /**
   * Override BlockScope::outputPHP() to generate return type.
   */
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * Override to preface with call temps.
   */
  virtual void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar);
  /**
   * Generate parameter declaration.
   */
  void outputCPPParamsDecl(CodeGenerator &cg, AnalysisResultPtr ar,
                           ExpressionListPtr params, bool showDefault);
  /**
   * This one is a special version that doesn't require a params expression.
   * It's for use with extension functions. It only works for the
   * implementation since it ignores optional arguments.
   */
  void outputCPPParamsImpl(CodeGenerator &cg, AnalysisResultPtr ar);
  /**
   * If inside this function, we have to make a call to an implementation
   * function that has the same signature, how does the parameter list
   * look like?
   */
  void outputCPPParamsCall(CodeGenerator &cg, AnalysisResultPtr ar,
                           bool aggregateParams);

  /**
   * How does a caller prepare parameters.
   */
  static void OutputCPPArguments(ExpressionListPtr params,
                                 FunctionScopePtr func,
                                 CodeGenerator &cg, AnalysisResultPtr ar,
                                 int extraArg, bool variableArgument,
                                 int extraArgArrayId = -1,
                                 int extraArgArrayHash = -1,
                                 int extraArgArrayIndex = -1);

  /**
   * Only generate arguments that have effects. This is for keeping those
   * parameters around when generating a error-raising function call, so to
   * avoid "unused" variable compiler warnings.
   */
  static void OutputCPPEffectiveArguments(ExpressionListPtr params,
                                          CodeGenerator &cg,
                                          AnalysisResultPtr ar);

  /**
   * Generate invoke proxy.
   */
  static void OutputCPPDynamicInvokeCount(CodeGenerator &cg);
  void outputCPPDynamicInvoke(CodeGenerator &cg, AnalysisResultPtr ar,
                              const char *funcPrefix,
                              const char *name,
                              bool voidWrapperOff = false,
                              bool fewArgs = false,
                              bool ret = true,
                              const char *extraArg = NULL,
                              bool constructor = false,
                              const char *instance = NULL,
                              const char *class_name = "");

  /**
   * ...so ClassStatement can call them for classes that don't have
   * constructors defined
   */
  void outputCPPCreateDecl(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPCreateImpl(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * output functions
   */
  void outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputMethodWrapper(CodeGenerator &cg, AnalysisResultPtr ar,
                           const char *clsToConstruct);

  /**
   * Output CallInfo instance for this function.
   */
  void outputCPPCallInfo(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * Serialize the iface, not everything.
   */
  void serialize(JSON::OutputStream &out) const;
  bool inPseudoMain() {
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

  void addCaller(BlockScopePtr caller);
  ReadWriteMutex &getInlineMutex() { return m_inlineMutex; }

  DECLARE_BOOST_TYPES(FunctionInfo);

  static void RecordFunctionInfo(std::string fname, FunctionScopePtr func);

  static FunctionInfoPtr GetFunctionInfo(std::string fname);

  class FunctionInfo {
  public:
    FunctionInfo(int rva = -1) : m_maybeStatic(false), m_refVarArg(rva) { }

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

  private:
    bool m_maybeStatic; // this could be a static method
    int m_refVarArg; // -1: no ref varargs;
                     // otherwise, any arg >= m_refVarArg is a reference
    std::set<int> m_refParams; // set of ref arg positions
  };

private:
  bool outputCPPArrayCreate(CodeGenerator &cg,
                            AnalysisResultPtr ar,
                            int m_maxParam);

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
  unsigned m_pseudoMain : 1;
  unsigned m_magicMethod : 1;
  unsigned m_system : 1;
  unsigned m_inlineable : 1;
  unsigned m_sep : 1;
  unsigned m_containsThis : 1; // contains a usage of $this?
  unsigned m_containsBareThis : 1; // $this outside object-context
  unsigned m_nrvoFix : 1;
  unsigned m_inlineAsExpr : 1;
  unsigned m_inlineSameContext : 1;
  unsigned m_contextSensitive : 1;
  unsigned m_directInvoke : 1;
  unsigned m_needsRefTemp : 1;
  unsigned m_needsCheckMem : 1;
  unsigned m_closureGenerator : 1;
  unsigned m_noLSB : 1;
  unsigned m_nextLSB : 1;

  int m_redeclaring; // multiple definition of the same function
  StatementPtr m_stmtCloned; // cloned method body stmt
  int m_inlineIndex;
  FunctionOptPtr m_optFunction;
  int outputCPPInvokeArgCountCheck(CodeGenerator &cg, AnalysisResultPtr ar,
                                    bool ret, bool constructor, int maxCount);
  ExpressionPtrVec m_retExprsToFix;
  ExpressionListPtr m_closureVars;
  ReadWriteMutex m_inlineMutex;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __FUNCTION_SCOPE_H__
