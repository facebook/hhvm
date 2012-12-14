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

#ifndef __TEST_CODE_RUN_H__
#define __TEST_CODE_RUN_H__

#include <test/test_base.h>

///////////////////////////////////////////////////////////////////////////////

class VCRInfo {
public:
  VCRInfo(const char *i, const char *o, const char *f = "", int l = 0,
          bool nw = false, bool fo = false)
  : input(i), output(o), file(f), line(l), nowarnings(nw), fileoutput(fo) { }

  const char *input;
  const char *output;
  const char *file;
  int line;
  bool nowarnings;
  bool fileoutput;
};

typedef std::vector<VCRInfo> VCRInfoVec;

class OptionSetter;
/**
 * Testing PHP -> C++ -> execution.
 */
class TestCodeRun : public TestBase {
  friend class OptionSetter;
 public:
  TestCodeRun();

  virtual bool preTest();
  virtual bool postTest();
  virtual bool RunTests(const std::string &which);

  // test test harness
  bool TestSanity();

  // test code generation process
  bool TestInnerFunction();
  bool TestInnerClass();
  bool TestVariableArgument();
  bool TestArgumentHandling();
  bool TestListAssignment();
  bool TestExceptions();
  bool TestPredefined();
  bool TestLabels();
  bool TestPerfectVirtual();

  // test types
  bool TestBoolean();
  bool TestInteger();
  bool TestDouble();
  bool TestString();
  bool TestArray();
  bool TestArrayInit();
  bool TestArrayCopy();
  bool TestArrayEscalation();
  bool TestArrayOffset();
  bool TestArrayAccess();
  bool TestArrayIterator();
  bool TestArrayForEach();
  bool TestArrayAssignment();
  bool TestArrayFunctions();
  bool TestArrayCSE();
  bool TestScalarArray();
  bool TestRange();
  bool TestVariant();
  bool TestObject();
  bool TestObjectProperty();
  bool TestObjectMethod();
  bool TestClassMethod();
  bool TestObjectMagicMethod();
  bool TestObjectInvokeMethod();
  bool TestObjectAssignment();
  bool TestNewObjectExpression();
  bool TestObjectPropertyExpression();
  bool TestCollectionClasses();
  bool TestComparisons();
  bool TestTernary();
  bool TestTypes();
  bool TestSwitchStatement();

  // test semantics
  bool TestUnset();
  bool TestReference();
  bool TestDynamicConstants();
  bool TestDynamicVariables();
  bool TestDynamicProperties();
  bool TestDynamicFunctions();
  bool TestDynamicMethods();
  bool TestVolatile();
  bool TestSuperGlobals();
  bool TestGlobalStatement();
  bool TestStaticStatement();
  bool TestIfStatement();
  bool TestBreakStatement();
  bool TestContinueStatement();
  bool TestReturnStatement();
  bool TestAdd();
  bool TestMinus();
  bool TestMultiply();
  bool TestDivide();
  bool TestModulus();
  bool TestOperationTypes();
  bool TestUnaryOperators();
  bool TestSilenceOperator();
  bool TestLogicalOperators();
  bool TestGetClass();
  bool TestGetParentClass();
  bool TestRedeclaredFunctions();
  bool TestRedeclaredClasses();
  bool TestReassignThis();
  bool TestClone();
  bool TestEvalOrder();
  bool TestGetObjectVars();
  bool TestSerialization();
  bool TestJson();
  bool TestExit();
  bool TestConstructorDestructor();
  bool TestConcat();
  bool TestConstant();
  bool TestClassConstant();
  bool TestConstantFunction();
  bool TestDefined();
  bool TestAssignment();
  bool TestPrint();
  bool TestVarExport();
  bool TestLocale();
  bool TestBadFunctionCalls();
  bool TestConstructor();
  bool TestIntIsset();

  // misc
  bool TestHereDoc();
  bool TestProgramFunctions();
  bool TestCompilation();
  bool TestReflection();
  bool TestReflectionClasses();
  bool TestErrorHandler();
  bool TestAssertOptions();
  bool TestExtMisc();
  bool TestInvalidArgument();
  bool TestThrift();
  bool TestCreateFunction();
  bool TestSimpleXML();
  bool TestXML();
  bool TestDOMDocument();
  bool TestDirectory();
  bool TestFile();
  bool TestUselessAssignment();
  bool TestExtString();
  bool TestExtArray();
  bool TestExtFile();
  bool TestExtDate();
  bool TestExtImage();
  bool TestExtSplFile();
  bool TestExtIterator();
  bool TestExtSoap();
  bool TestExtCollator();
  bool TestExtSocket();
  bool TestAPC();
  bool TestInlining();
  bool TestCopyProp();
  bool TestRenameFunction();
  bool TestIntercept();
  bool TestMaxInt();
  bool TestParser();
  bool TestTypeAssertions();
  bool TestSerialize();
  bool TestHoisting();

  // PHP 5.3
  bool TestVariableClassName();
  bool TestLateStaticBinding();
  bool TestCallStatic();
  bool TestNowDoc();
  bool TestTernaryShortcut();
  bool TestGoto();
  bool TestClosure();
  bool TestNamespace();

  // PHP 5.4
  bool TestTraits();

  // PHP 5.5
  bool TestUConverter();

  // HipHop specific
  bool TestYield();
  bool TestHint();
  bool TestUserAttributes();
#ifdef TAINTED
  bool TestTaint();
  bool TestTaintExt();
#endif
  bool TestStrictMode();

  // debugging purpose
  bool TestAdHoc();

  static bool FastMode;
  static const char *Filter;

 protected:
  bool CleanUp();
  bool GenerateFiles(const char *input, const char *subdir = "");
  bool CompileFiles();
  bool RecordMulti(const char *input, const char *output, const char *file,
                   int line, bool nowarnings, bool fileoutput);

  bool MultiVerifyCodeRun();
  bool VerifyCodeRun(const char *input, const char *output,
                     const char *file = "", int line = 0,
                     bool nowarnings = false, bool fileoutput = false);

  bool m_perfMode;
  VCRInfoVec m_infos;
  std::string m_compileOptions;
  std::string m_runtimeOptions;
  std::string m_envVars;
  int m_test;
};

class OptionSetter {
public:
  enum Kind { CompileTime, RunTime, Env };
  OptionSetter(TestCodeRun *tcr, Kind k, const char *opt) :
    m_str(Get(tcr,k)), m_saved(m_str) {
    m_str += opt;
    m_str += "\n";
  }
  ~OptionSetter() {
    m_str = m_saved;
  }
private:
  std::string &Get(TestCodeRun *tcr, Kind k) {
    if (k == CompileTime) return tcr->m_compileOptions;
    if (k == RunTime) return tcr->m_runtimeOptions;
    return tcr->m_envVars;
  }
  std::string &m_str;
  std::string m_saved;
};

class HipHopSyntax {
public:
  HipHopSyntax(TestCodeRun *tcr) :
    m_compile(tcr, OptionSetter::CompileTime, "-vEnableHipHopSyntax=1"),
    m_runtime(tcr, OptionSetter::RunTime, "-vEval.EnableHipHopSyntax=1") {}
private:
  OptionSetter m_compile;
  OptionSetter m_runtime;
};

class FinallyStatement {
public:
  FinallyStatement(TestCodeRun *tcr) :
    m_compile(tcr, OptionSetter::CompileTime, "-vEnableFinallyStatement=1"),
    m_runtime(tcr, OptionSetter::RunTime, "-vEval.EnableFinallyStatement=1") {}
private:
  OptionSetter m_compile;
  OptionSetter m_runtime;
};

///////////////////////////////////////////////////////////////////////////////
// macros

#define VCR(a)                                                          \
  if (!Count(VerifyCodeRun(a,NULL,__FILE__,__LINE__,false))) return false;

#define VCRO(a, b)                                                      \
  if (!Count(VerifyCodeRun(a,b,__FILE__,__LINE__,false))) return false;

#define VCRNW(a)                                                        \
  if (!Count(VerifyCodeRun(a,NULL,__FILE__,__LINE__,true))) return false;

// Multi VCR
#define MVCR(a)                                                         \
  if (!RecordMulti(a,NULL,__FILE__,__LINE__,false,false)) return false;

#define MVCRO(a, b)                                                     \
  if (!RecordMulti(a, b, __FILE__,__LINE__,false,false)) return false;

#define MVCROF(a, b)                                                     \
  if (!RecordMulti(a, b, __FILE__,__LINE__,false,true)) return false;

#define MVCRNW(a)                                                       \
  if (!RecordMulti(a,NULL,__FILE__,__LINE__,true,false)) return false;

#define MVCRONW(a,b)                                                     \
  if (!RecordMulti(a,b,__FILE__,__LINE__,true,false)) return false;

///////////////////////////////////////////////////////////////////////////////

#endif // __TEST_CODE_RUN_H__
