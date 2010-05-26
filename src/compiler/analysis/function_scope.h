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
  FunctionScope(AnalysisResultPtr ar, bool method,
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
  void setParamCounts(AnalysisResultPtr ar, int minParam, int maxParam);
  void setParamName(int index, const std::string &name);
  void setRefParam(int index);
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
  bool isStaticMethodAutoFixed() const { return m_staticMethodAutoFixed;}
  bool isAbstract() const;
  bool isFinal() const;
  bool isRefParam(int index) const;
  bool isRefReturn() const { return m_refReturn;}
  bool hasImpl() const;

  /**
   * Either __construct or a class-name constructor.
   */
  bool isConstructor(ClassScopePtr cls) const;

  const std::string &getParamName(int index) const;

  const std::string &name() const {
    return getName();
  }

  std::string getId() const {
    if (m_redeclaring < 0) {
      return getName();
    }
    return getName() + Option::IdPrefix +
      boost::lexical_cast<std::string>(m_redeclaring);
  }

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

  void setStaticMethodAutoFixed();

  /**
   * Get original name of the function, without case being lowered.
   */
  std::string getOriginalName() const;

  /**
   * If class method, returns class::name, otherwise just name.
   */
  std::string getFullName() const;

  /**
   * Whether this function can take variable number of arguments.
   */
  bool isVariableArgument() const;
  bool isReferenceVariableArgument() const;
  void setVariableArgument(bool reference);
  bool isMixedVariableArgument() const;

  /**
   * Whether this function has no side effects
   */
  bool hasEffect() const;
  void setNoEffect();

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
  void setContainsThis() { m_containsThis = true;}

  /**
   * How many parameters a caller should provide.
   */
  int getMinParamCount() const { return m_minParam;}
  int getMaxParamCount() const { return m_maxParam;}
  int getOptionalParamCount() const { return m_maxParam - m_minParam;}

  /**
   * What is the inferred type of this function's return.
   */
  void setReturnType(AnalysisResultPtr ar, TypePtr type);
  TypePtr getReturnType() const { return m_returnType;}

  /**
   * Whether this is a virtual function.
   */
  void setVirtual() { m_virtual = true;}
  bool isVirtual() const { return m_virtual;}
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

  /**
   * What is the inferred type of this function's parameter at specified
   * index. Returns number of extra arguments to put into ArgumentArray.
   */
  int inferParamTypes(AnalysisResultPtr ar, ConstructPtr exp,
                      ExpressionListPtr params, bool &valid);
  TypePtr setParamType(AnalysisResultPtr ar, int index, TypePtr type);
  TypePtr getParamType(int index);

  int requireCallTemps(int count) {
    int ret = m_callTempCountCurrent;
    m_callTempCountCurrent += count;
    if (m_callTempCountMax < m_callTempCountCurrent) {
      m_callTempCountMax = m_callTempCountCurrent;
    }
    return ret;
  }
  void endRequireCallTemps(int old) {
    m_callTempCountCurrent = old;
  }

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
  static void outputCPPArguments(ExpressionListPtr params,
                                 CodeGenerator &cg, AnalysisResultPtr ar,
                                 int extraArg, bool variableArgument,
                                 int extraArgArrayId = -1);

  /**
   * Only generate arguments that have effects. This is for keeping those
   * parameters around when generating a error-raising function call, so to
   * avoid "unused" variable compiler warnings.
   */
  static void outputCPPEffectiveArguments(ExpressionListPtr params,
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
                              bool constructor = false);
  void outputCPPEvalInvoke(CodeGenerator &cg, AnalysisResultPtr ar,
      const char *funcPrefix, const char *name, const char *extraArg = NULL,
      bool ret = true, bool constructor = false);

  /**
   * ...so ClassStatement can call them for classes that don't have
   * constructors defined
   */
  void outputCPPCreateDecl(CodeGenerator &cg, AnalysisResultPtr ar);
  void outputCPPCreateImpl(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * Output ClassInfo entry of this function.
   */
  void outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar);

  FileScopePtr getFileScope() {
    FileScopePtr fs = m_file.lock();
    return fs;
  }

  /**
   * Serialize the iface, not everything.
   */
  void serialize(JSON::OutputStream &out) const;
  bool inPseudoMain() {
    return m_pseudoMain;
  }
  void setFileScope(FileScopePtr fs) {
    m_file = fs;
  }

  void setMagicMethod() {
    m_magicMethod = true;
  }
  bool isMagicMethod() const {
    return m_magicMethod;
  }

  void setIgnored() {
    m_ignored = true;
  }
  bool isIgnored() const {
    return m_ignored;
  }

  void setStmtCloned(StatementPtr stmt) {
    m_stmtCloned = stmt;
  }

  DECLARE_BOOST_TYPES(RefParamInfo);

  static void RecordRefParamInfo(std::string fname, FunctionScopePtr func);

  static RefParamInfoPtr GetRefParamInfo(std::string fname) {
    StringToRefParamInfoPtrMap::iterator it = s_refParamInfo.find(fname);
    if (it == s_refParamInfo.end()) {
      return RefParamInfoPtr();
    }
    return it->second;
  }

  class RefParamInfo {
  public:
    RefParamInfo(int rva = -1) : m_refVarArg(rva) { }

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

  private:
    int m_refVarArg; // -1: no ref varargs;
                     // otherwise, any arg >= m_refVarArg is a reference
    std::set<int> m_refParams; // set of ref arg positions
  };

private:
  static StringToRefParamInfoPtrMap s_refParamInfo;

  bool m_method;
  FileScopeWeakPtr m_file;
  int m_minParam;
  int m_maxParam;
  int m_attribute;
  std::vector<std::string> m_paramNames;
  TypePtrVec m_paramTypes;
  TypePtrVec m_paramTypeSpecs;
  bool m_refReturn; // whether it's "function &get_reference()"
  std::vector<bool> m_refs;
  TypePtr m_returnType;
  ModifierExpressionPtr m_modifiers;
  bool m_virtual;
  bool m_dynamic;
  bool m_overriding; // overriding a virtual function
  int m_redeclaring; // multiple definition of the same function
  bool m_volatile; // for function_exists
  bool m_ignored; // redeclared system functions are ignored automatically
  bool m_pseudoMain;
  bool m_magicMethod;
  bool m_system;
  bool m_inlineable;
  bool m_sep;
  bool m_containsThis; // contains a usage of $this?
  bool m_staticMethodAutoFixed; // auto fixed static
  int m_callTempCountMax;
  int m_callTempCountCurrent;
  StatementPtr m_stmtCloned; // cloned method body stmt

  void outputCPPInvokeArgCountCheck(CodeGenerator &cg, AnalysisResultPtr ar,
      bool ret, bool constructor);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __FUNCTION_SCOPE_H__
