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

#include <test/test_code_run.h>
#include <compiler/parser/parser.h>
#include <compiler/builtin_symbols.h>
#include <compiler/code_generator.h>
#include <compiler/analysis/analysis_result.h>
#include <util/util.h>
#include <util/process.h>
#include <compiler/option.h>
#include <runtime/base/fiber_async_func.h>
#include <runtime/base/runtime_option.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

static const char *php_path = "/usr/local/php/bin/php";

// By default, use shared linking for faster testing.
bool TestCodeRun::FastMode = true;

TestCodeRun::TestCodeRun() : m_perfMode(false) {
  Option::GenerateCPPMain = true;
  Option::GenerateCPPMetaInfo = true;
  Option::GenerateCPPMacros = true;
  Option::GenerateCPPComments = true;
  Option::GenerateCPPNameSpace = true;
  Option::KeepStatementsWithNoEffect = false;
  Option::ParserThreadCount = 4;
  Option::SplitDynamicClassTable = false;
}

bool TestCodeRun::preTest() {
  if (!CleanUp()) return false;
  m_infos.clear();
  Type::ResetTypeHintTypes();
  return true;
}

bool TestCodeRun::postTest() {
  if (!MultiVerifyCodeRun()) return false;
  return true;
}

bool TestCodeRun::CleanUp() {
  string out, err;
  const char *argv[] = {"", NULL};
  Process::Exec("runtime/tmp/cleanup.sh", argv, NULL, out, &err);
  if (!err.empty()) {
    printf("Failed to clean up runtime/tmp: %s\n", err.c_str());
    return false;
  }
  return true;
}

static bool GenerateMainPHP(const std::string &fullPath, const char *input) {
  Util::mkdir(fullPath.c_str());
  ofstream f(fullPath.c_str());
  if (!f) {
    printf("Unable to open %s for write. Run this test from src/.\n",
           fullPath.c_str());
    return false;
  }

  f << input;
  f.close();
  return true;
}

bool TestCodeRun::GenerateFiles(const char *input,
                                const char *subdir) {
  ASSERT(subdir && subdir[0]);

  // generate main.php early, so if we fail, we have a PHP file to debug with
  string fullPath = "runtime/tmp";
  if (subdir && subdir[0]) fullPath = fullPath + "/" + subdir;
  fullPath += "/main.php";
  if (!GenerateMainPHP(fullPath, input)) return false;

  AnalysisResultPtr ar(new AnalysisResult());
  string path = string("runtime/tmp/") + subdir;
  ar->setOutputPath(path.c_str());
  Compiler::Parser::ParseString(input, ar);
  BuiltinSymbols::Load(ar);
  ar->loadBuiltins();
  ar->analyzeProgram();
  ar->preOptimize();
  ar->inferTypes();
  ar->postOptimize();
  ar->analyzeProgramFinal();
  ar->outputAllCPP(CodeGenerator::ClusterCPP, 0, NULL);

  string target = path + "/Makefile";
  const char *argv1[] = {"", "runtime/tmp/single.mk", target.c_str(), NULL};
  string out, err;
  Process::Exec("cp", argv1, NULL, out, &err);
  if (!err.empty()) {
    printf("Failed to copy runtime/tmp/single.mk: %s\n", err.c_str());
    return false;
  }

  if (FastMode) {
    string sys = string(subdir) + "/sys";
    const char *argv2[] = {"", sys.c_str(), NULL};
    Process::Exec("runtime/tmp/mergecpp.sh", argv2, NULL, out, &err);
    if (!err.empty()) {
      printf("Failed to merge runtime/tmp/%s/*.cpp: %s\n",
             sys.c_str(), err.c_str());
      return false;
    }
  }

  return true;
}

static string filter_distcc(string &msg) {
  istringstream is(msg);
  ostringstream os;
  string line;
  while (getline(is, line)) {
    if (line.compare(0, 6, "distcc")) {
      os << line << endl;
    }
  }
  return os.str();
}

bool TestCodeRun::CompileFiles() {
  string out, err;
  const char *argv[] = {"", FastMode ? "SHARED" : "LINK", NULL};
  Process::Exec("runtime/makeall.sh", argv, NULL, out, &err);
  err = filter_distcc(err);
  if (!err.empty()) {
    printf("Failed to compile files:\n");

    istringstream is(err);
    string line;
    string buffer;
    while (getline(is, line)) {
      buffer += line + "\n";
      if (!line.compare(0, 4, "make")) {
        unsigned int start = 0;
        while (start < line.length() && line.compare(start, 4, "Test")) {
          start++;
        }
        if (start < line.length()) {
          istringstream isl(line.substr(start + 4));
          int seqno;
          if (isl >> seqno) {
            printf("======================================\n"
                   "runtime/tmp/Test%d/main.php:\n"
                   "======================================\n",
                  seqno);
            printf("\n%s:%d\nParsing: [%s]\n%s\n", m_infos[seqno].file,
                   m_infos[seqno].line, m_infos[seqno].input, buffer.c_str());
            buffer = "";
          }
        }
      }
    }
    if (!buffer.empty()) {
      // leftover error messages, if any
      printf("\n%s\n", buffer.c_str());
    }

    return false;
  }
  return true;
}

static string escape(const std::string &s) {
  string ret;
  ret.reserve(s.size() + 20);
  for (unsigned int i = 0; i < s.length(); i++) {
    char ch = s[i];
    if (isprint(ch) || ch == '\n') {
      ret += ch;
    } else {
      char buf[10];
      snprintf(buf, sizeof(buf), "{\\x%02X}", (unsigned char)ch);
      ret += buf;
    }
  }
  return ret;
}

static bool verify_result(const char *input, const char *output, bool perfMode,
                          const char *file = "", int line = 0,
                          bool nowarnings = false, const char *subdir = "",
                          bool fastMode = false) {
  // generate main.php
  string fullPath = "runtime/tmp";
  if (subdir && subdir[0]) fullPath = fullPath + "/" + subdir;
  fullPath += "/main.php";
  if (!GenerateMainPHP(fullPath, input)) return false;

  // get PHP's output if "output" is NULL
  string expected;
  if (output) {
    expected = output;
  } else {
    const char *argv1[] = {"", fullPath.c_str(), NULL};
    const char *argv2[] = {"", "-n", fullPath.c_str(), NULL};
    string err;
    Process::Exec(php_path, nowarnings ? argv2 : argv1, NULL, expected, &err);
    if (!err.empty() && nowarnings) {
      printf("%s:%d\nParsing: [%s]\nFailed to run %s: %s\n",
             file, line, input, fullPath.c_str(), err.c_str());
      return false;
    }
  }

  // run and verify output
  {
    string actual, err;
    if (Option::EnableEval < Option::FullEval) {
      if (fastMode) {
        string path = "runtime/tmp/";
        if (subdir) path = path + subdir + "/";
        path += "libtest.so";
        const char *argv[] = {"", "--file=string", "--config=test/config.hdf",
                              path.c_str(), NULL};
        Process::Exec("runtime/tmp/run.sh", argv, NULL, actual, &err);
      } else {
        const char *argv[] = {"", "--file=string", "--config=test/config.hdf",
                              NULL};
        string path = "runtime/tmp/";
        if (subdir) path = path + subdir + "/";
        path += "test";
        Process::Exec(path.c_str(), argv, NULL, actual, &err);
      }
    } else {
      string filearg = "--file=runtime/tmp/";
      if (subdir) filearg = filearg + subdir + "/";
      filearg += "main.php";
      const char *argv[] = {"", filearg.c_str(),
                            "--config=test/config.hdf",
                            "-v Fiber.ThreadCount=5",
                            "-v Eval.EnableObjDestructCall=true",
                            NULL};
      Process::Exec("hphpi/hphpi", argv, NULL, actual, &err);
    }

    if (perfMode) {
      string sinput = input;
      const char *marker = "/* INPUT */";
      int pos1 = sinput.find(marker);
      int pos2 = sinput.find(marker, pos1+1);
      pos1 += strlen(marker);
      sinput = sinput.substr(pos1, pos2 - pos1);
      if (sinput.size() > 1000) sinput = "(long program)";

      // we have to adjust timing by removing loop cost, which is the 1st test
      static int adj1 = -1;
      static int adj2 = -1;
      int ms1 = atoi(expected.c_str());
      int ms2 = atoi(actual.c_str());
      if (adj1 == -1) adj1 = ms1;
      if (adj2 == -1) adj2 = ms2;
      int msAdj1 = ms1 - adj1;
      int msAdj2 = ms2 - adj2;
      double x = 0.0; // how many times faster
      double p = 0.0; // percentage
      if (msAdj2 != 0) {
        x = ((double)(int)(msAdj1 * 100 / msAdj2)) / 100;
      }
      if (msAdj1 != 0) {
        p = ((double)(int)(msAdj2 * 10000 / msAdj1)) / 100;
      }

      printf("----------------------------------------------------------\n"
             "%s\n\n"
             "        PHP        C++\n"
             "===========================================\n"
             "  %6d ms  %6d ms\n"
             " -%6d ms  %6d ms\n"
             "===========================================\n"
             "  %6d ms  %6d ms   =   %2.4gx  or  %2.4g%%\n\n",
             sinput.c_str(), ms1, ms2, adj1, adj2, msAdj1, msAdj2, x, p);
      return true;
    }

    bool out_ok = actual == expected;
    if (!out_ok || !err.empty()) {
      if (out_ok &&
          err.find("symbol lookup error:") != string::npos &&
          err.find("undefined symbol: ") != string::npos) {
        printf("%s: Ignoring loader error: %s\n",
               fullPath.c_str(), err.c_str());
      } else {
        printf("======================================\n"
               "%s:\n"
               "======================================\n"
               "%s:%d\nParsing: [%s]\nBet %d:\n"
               "--------------------------------------\n"
               "%s"
               "--------------------------------------\n"
               "Got %d:\n"
               "--------------------------------------\n"
               "%s"
               "--------------------------------------\n"
               "Err: [%s]\n", fullPath.c_str(), file, line, input,
               (int)expected.length(), escape(expected).c_str(),
               (int)actual.length(), escape(actual).c_str(),
               err.c_str());
        return false;
      }
    }
  }

  return true;
}

bool TestCodeRun::RecordMulti(const char *input, const char *output,
                              const char *file, int line, bool flag) {
  size_t i = m_infos.size();
  m_infos.push_back(VCRInfo(input, output, file, line, flag));

  if (Option::EnableEval < Option::FullEval) {
    ASSERT(m_infos[i].input);
    ostringstream os;
    os << "Test" << i;
    if (!GenerateFiles(m_infos[i].input, os.str().c_str())) return false;
  }

  return true;
}

bool TestCodeRun::MultiVerifyCodeRun() {
  if (Option::EnableEval < Option::FullEval) {
    CompileFiles();
  }

  bool ret = true;
  for (unsigned i = 0; i < m_infos.size(); i++) {
    ASSERT(m_infos[i].input);
    ostringstream os;
    os << "Test" << i;
    if (!Count(verify_result(m_infos[i].input, m_infos[i].output, m_perfMode,
                             m_infos[i].file, m_infos[i].line,
                             m_infos[i].nowarnings, os.str().c_str(),
                             FastMode))) {
      ret = false;
    }
  }
  return ret;
}

bool TestCodeRun::VerifyCodeRun(const char *input, const char *output,
                                const char *file /* = "" */,
                                int line /* = 0 */,
                                bool nowarnings /* = false */) {
  ASSERT(input);
  if (!CleanUp()) return false;
  if (Option::EnableEval < Option::FullEval) {
    if (!GenerateFiles(input, "Test0") || !CompileFiles()) {
      return false;
    }
  }

  return verify_result(input, output, m_perfMode,
                       file, line, nowarnings, "Test0", FastMode);
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestSanity);
  RUN_TEST(TestInnerFunction);
  RUN_TEST(TestInnerClass);
  RUN_TEST(TestVariableArgument);
  RUN_TEST(TestArgumentHandling);
  RUN_TEST(TestListAssignment);
  RUN_TEST(TestExceptions);
  RUN_TEST(TestPredefined);
  RUN_TEST(TestLabels);
  RUN_TEST(TestPerfectVirtual);
  RUN_TEST(TestBoolean);
  RUN_TEST(TestInteger);
  RUN_TEST(TestDouble);
  RUN_TEST(TestString);
  RUN_TEST(TestLocale);
  RUN_TEST(TestArray);
  RUN_TEST(TestArrayInit);
  RUN_TEST(TestArrayCopy);
  RUN_TEST(TestArrayEscalation);
  RUN_TEST(TestArrayOffset);
  RUN_TEST(TestArrayAccess);
  RUN_TEST(TestArrayIterator);
  RUN_TEST(TestArrayForEach);
  RUN_TEST(TestArrayAssignment);
  RUN_TEST(TestArrayFunctions);
  RUN_TEST(TestScalarArray);
  RUN_TEST(TestRange);
  RUN_TEST(TestVariant);
  RUN_TEST(TestObject);
  RUN_TEST(TestObjectProperty);
  RUN_TEST(TestObjectMethod);
  RUN_TEST(TestClassMethod);
  RUN_TEST(TestObjectMagicMethod);
  RUN_TEST(TestObjectAssignment);
  RUN_TEST(TestNewObjectExpression);
  RUN_TEST(TestObjectPropertyExpression);
  RUN_TEST(TestComparisons);
  RUN_TEST(TestReference);
  RUN_TEST(TestUnset);
  RUN_TEST(TestDynamicConstants);
  RUN_TEST(TestDynamicVariables);
  RUN_TEST(TestDynamicProperties);
  RUN_TEST(TestDynamicFunctions);
  RUN_TEST(TestRenameFunction);
  //RUN_TEST(TestIntercept); // requires ENABLE_INTERCEPT
  RUN_TEST(TestMaxInt);
  RUN_TEST(TestDynamicMethods);
  RUN_TEST(TestVolatile);
  RUN_TEST(TestHereDoc);
  RUN_TEST(TestProgramFunctions);
  RUN_TEST(TestCompilation);
  RUN_TEST(TestReflection);
  RUN_TEST(TestReflectionClasses);
  RUN_TEST(TestErrorHandler);
  RUN_TEST(TestAssertOptions);
  RUN_TEST(TestExtMisc);
  RUN_TEST(TestInvalidArgument);
  RUN_TEST(TestSuperGlobals);
  RUN_TEST(TestGlobalStatement);
  RUN_TEST(TestStaticStatement);
  RUN_TEST(TestIfStatement);
  RUN_TEST(TestBreakStatement);
  RUN_TEST(TestContinueStatement);
  RUN_TEST(TestReturnStatement);
  RUN_TEST(TestAdd);
  RUN_TEST(TestMinus);
  RUN_TEST(TestMultiply);
  RUN_TEST(TestDivide);
  RUN_TEST(TestModulus);
  RUN_TEST(TestOperationTypes);
  RUN_TEST(TestUnaryOperators);
  RUN_TEST(TestSilenceOperator);
  RUN_TEST(TestPrint);
  RUN_TEST(TestVarExport);
  RUN_TEST(TestLogicalOperators);
  RUN_TEST(TestGetClass);
  RUN_TEST(TestGetParentClass);
  RUN_TEST(TestRedeclaredFunctions);
  RUN_TEST(TestRedeclaredClasses);
  RUN_TEST(TestClone);
  RUN_TEST(TestEvalOrder);
  RUN_TEST(TestGetObjectVars);
  RUN_TEST(TestSerialization);
  RUN_TEST(TestJson);
  RUN_TEST(TestThrift);
  RUN_TEST(TestExit);
  RUN_TEST(TestCreateFunction);
  RUN_TEST(TestConstructorDestructor);
  RUN_TEST(TestConcat);
  RUN_TEST(TestConstant);
  RUN_TEST(TestClassConstant);
  RUN_TEST(TestConstantFunction);
  RUN_TEST(TestDefined);
  RUN_TEST(TestSimpleXML);
  RUN_TEST(TestXML);
  RUN_TEST(TestDOMDocument);
  RUN_TEST(TestFile);
  RUN_TEST(TestDirectory);
  RUN_TEST(TestAssignment);
  RUN_TEST(TestBadFunctionCalls);
  RUN_TEST(TestConstructor);
  RUN_TEST(TestTernary);
  RUN_TEST(TestUselessAssignment);
  RUN_TEST(TestTypes);
  RUN_TEST(TestSwitchStatement);
  RUN_TEST(TestExtString);
  RUN_TEST(TestExtArray);
  RUN_TEST(TestExtFile);
  RUN_TEST(TestExtDate);
  RUN_TEST(TestExtImage);
  RUN_TEST(TestExtSplFile);
  RUN_TEST(TestExtIterator);
  RUN_TEST(TestExtSoap);
  RUN_TEST(TestExtCollator);
  RUN_TEST(TestExtSocket);
  RUN_TEST(TestFiber);
  RUN_TEST(TestAPC);
  RUN_TEST(TestInlining);
  RUN_TEST(TestParser);

  // PHP 5.3 features
  RUN_TEST(TestVariableClassName);
  RUN_TEST(TestLateStaticBinding); // requires ENABLE_LATE_STATIC_BINDING
  RUN_TEST(TestCallStatic);
  RUN_TEST(TestNowDoc);
  RUN_TEST(TestTernaryShortcut);
  RUN_TEST(TestGoto);
  RUN_TEST(TestClosure);
  RUN_TEST(TestNamespace);

  // HipHop features
  RUN_TEST(TestYield);
  RUN_TEST(TestHint);

  RUN_TEST(TestAdHoc);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// code generation

bool TestCodeRun::TestSanity() {
  MVCR("<?php print 'Hello, World!';");
  MVCR("Hello, World!");
  MVCR("#!/usr/bin/php\n"
       "<?php\n");
  MVCR("#!/usr/bin/php\n"
       "\n"
       "<?php\n");
  MVCR("#!/usr/bin/env <?php\n"
       "#!/usr/bin/env php\n"
       "a /* show or not */ b\n"
       "Hello, World! # comments\n"
       "<?php\n"
       "print 'hello'; # comments");
  MVCR("<?php if (true) { ?>#<?php } ?>");
  return true;
}

bool TestCodeRun::TestInnerFunction() {
  MVCR("<?php function test() { print 'test';} test();");
  MVCR("<?php function test() { function inner() { print 'test';} inner();} "
      "test();");
  return true;
}

bool TestCodeRun::TestInnerClass() {
  MVCR("<?php class test { function p() { print 'test';} } "
      "$obj = new Test(); $obj->p();");
  MVCR("<?php class test { function p() { function inner() { print 'test';} "
      "inner();} } $obj = new Test(); $obj->p();");
  MVCR("<?php function test() { class test { function p() { print 'test';} }} "
      "test(); $obj = new Test(); $obj->p();");
  return true;
}

bool TestCodeRun::TestVariableArgument() {
  MVCR("<?php function test() { "
      "  $n = func_num_args(); "
      "  var_dump($n);"
      "  $args = func_get_args();"
      "  var_dump($args);"
      "} "
      "test(); test(1); test(1, 2);");
  MVCR("<?php function test() { "
      "  var_dump(func_get_arg(0));"
      "  var_dump(func_get_arg(1));"
      "} "
      "test(2, 'ok');");

  MVCR("<?php function test($a) { "
      "  $n = func_num_args(); "
      "  var_dump($n);"
      "  $args = func_get_args();"
      "  var_dump($args);"
      "} "
      "test(1); test(1, 2); test(1, 2, 3);");
  MVCR("<?php function test($a) { "
      "  var_dump(func_get_arg(0));"
      "  var_dump(func_get_arg(1));"
      "  var_dump(func_get_arg(2));"
      "} "
      "test(2, 'ok', array(1));");

  MVCR("<?php function test($a, $b) { "
      "  $n = func_num_args(); "
      "  var_dump($n);"
      "  $args = func_get_args();"
      "  var_dump($args);"
      "} "
      "test(1, 2); test(1, 2, 3); test(1, 2, 3, 4);");
  MVCR("<?php function test() { "
      "  var_dump(func_get_arg(0));"
      "  var_dump(func_get_arg(1));"
      "  var_dump(func_get_arg(2));"
      "  var_dump(func_get_arg(3));"
      "} "
      "test(2, 'ok', 0, 'test');");

  MVCR("<?php function test($a) { "
      "  $n = func_num_args(); "
      "  var_dump($n);"
      "  $args = func_get_args();"
      "  var_dump($args);"
      "} "
      "test('test'); test(1, 2); test(1, 2, 3);");

  MVCR("<?php class A { public function test($a) {"
      "  var_dump(func_num_args());"
      "  var_dump(func_get_args());"
      "}} $obj = new A(); $obj->test('test'); $obj->test(1, 2, 3);");

  MVCR("<?php class A { public function __construct($a) {"
      "  var_dump(func_num_args());"
      "  var_dump(func_get_args());"
      "}} $obj = new A(1, 2, 3); $obj = new A('test');");

  MVCR("<?php function test($a = 10) { "
      "  var_dump($a);"
      "  var_dump(func_get_args());"
      "} "
      "test(); test(1); test(1, 2);");

  MVCR("<?php function test($a, $b = 10) { "
      "  var_dump($a);"
      "  var_dump($b);"
      "  var_dump(func_get_args());"
      "} "
      "test(1); test(1, 2); test(1, 2, 3);");

  // testing variable argument + reference parameter
  MVCR("<?php $ar1 = array(10, 100, 100, 0); $ar2 = array(1, 3, 2, 4);"
      "array_multisort($ar1, $ar2); var_dump($ar1, $ar2);");

  MVCR("<?php "
       "class Foo {"
       "  public static function Add($x, $y) {}"
       "}"
       "$x = 0;"
       "if (!call_user_func(array('Foo', 'add'), $x, 0)) {"
       "  echo 'foo';"
       "}");
  MVCR("<?php "
       "function f1($a, $b) {"
       "  $c = func_num_args();"
       "  $args = func_get_args();"
       "  $args[0] = 5;"
       "  $args[1] = 6;"
       "  $args[2] = 7;"
       "  var_dump($c);"
       "  var_dump($args);"
       "}"
       "function f2($a, &$b) {"
       "  $c = func_num_args();"
       "  $args = func_get_args();"
       "  $args[0] = 5;"
       "  $args[1] = 6;"
       "  $args[2] = 7;"
       "  var_dump($c);"
       "  var_dump($args);"
       "}"
       "function f3(&$a, $b) {"
       "  $c = func_num_args();"
       "  $args = func_get_args();"
       "  $args[0] = 5;"
       "  $args[1] = 6;"
       "  $args[2] = 7;"
       "  var_dump($c);"
       "  var_dump($args);"
       "}"
       "function f4(&$a, &$b) {"
       "  $c = func_num_args();"
       "  $args = func_get_args();"
       "  var_dump($args);"
       "  $args[0] = 5;"
       "  $args[1] = 6;"
       "  $args[2] = 7;"
       "  var_dump($c);"
       "  var_dump($args);"
       "}"
       "function f5($a, $b) {"
       "  $arg0 = func_get_arg(0);"
       "  $arg1 = func_get_arg(1);"
       "  $arg2 = func_get_arg(2);"
       "  $arg0 = 5;"
       "  $arg1 = 6;"
       "  $arg2 = 7;"
       "  var_dump($arg0, $arg1, $arg2);"
       "}"
       "function f6($a, &$b) {"
       "  $arg0 = func_get_arg(0);"
       "  $arg1 = func_get_arg(1);"
       "  $arg2 = func_get_arg(2);"
       "  $arg0 = 5;"
       "  $arg1 = 6;"
       "  $arg2 = 7;"
       "  var_dump($arg0, $arg1, $arg2);"
       "}"
       "function f7(&$a, $b) {"
       "  $arg0 = func_get_arg(0);"
       "  $arg1 = func_get_arg(1);"
       "  $arg2 = func_get_arg(2);"
       "  $arg0 = 5;"
       "  $arg1 = 6;"
       "  $arg2 = 7;"
       "  var_dump($arg0, $arg1, $arg2);"
       "}"
       "function f8(&$a, &$b) {"
       "  $arg0 = func_get_arg(0);"
       "  $arg1 = func_get_arg(1);"
       "  $arg2 = func_get_arg(2);"
       "  $arg0 = 5;"
       "  $arg1 = 6;"
       "  $arg2 = 7;"
       "  var_dump($arg0, $arg1, $arg2);"
       "}"
       "function bar() {"
       "  $a = 1;"
       "  f1($a, $a, $a);"
       "  var_dump($a);"
       "  $a = 1;"
       "  f2($a, $a, $a);"
       "  var_dump($a);"
       "  $a = 1;"
       "  f3($a, $a, $a);"
       "  var_dump($a);"
       "  $a = 1;"
       "  f4($a, $a, $a);"
       "  var_dump($a);"
       "  $a = 1;"
       "  f5($a, $a, $a);"
       "  var_dump($a);"
       "  $a = 1;"
       "  f6($a, $a, $a);"
       "  var_dump($a);"
       "  $a = 1;"
       "  f7($a, $a, $a);"
       "  var_dump($a);"
       "  $a = 1;"
       "  f8($a, $a, $a);"
       "  var_dump($a);"
       "}"
       "bar();");
  MVCR("<?php "
       "class Y { function __destruct() { } };"
       "class X extends Y {"
       "  function __get($a) {"
       "    var_dump(func_get_args());"
       "    var_dump(func_get_arg(0));"
       "    var_dump(func_num_args());"
       "    return 42;"
       "  }"
       "  function __destruct() {"
       "    var_dump(func_get_args());"
       "    var_dump(func_get_arg(0));"
       "    var_dump(func_num_args());"
       "    return 2442;"
       "  }"
       "}"
       "$x = new X;"
       "var_dump($x->buz);"
       "unset($x);");

  return true;
}

bool TestCodeRun::TestArgumentHandling() {
  MVCR("<?php\n"
       "function test($str) {\n"
       "  return strlen($str);\n"
       "}\n"
       "var_dump(strlen());\n"
       "var_dump(test());\n"
       "\n"
       "var_dump(strlen('test'));\n"
       "var_dump(test('test'));\n"
       "\n"
       "var_dump(strlen('test', 123));\n"
       "var_dump(test('test', 123));\n"
      );

  MVCR("<?php "
       "function f($x) { $b = $x; $b++; }"
       "$a = 1;"
       "f(&$a);"
       "var_dump($a);");

  return true;
}

bool TestCodeRun::TestListAssignment() {
  MVCR("<?php $a = 'old'; var_dump(list($a) = false); var_dump($a);");
  MVCR("<?php $a = 'old'; var_dump(list($a) = 'test'); var_dump($a);");
  MVCR("<?php $a = 'old'; var_dump(list($a) = 123); var_dump($a);");
  MVCR("<?php list() = array(1,2,3);");
  MVCR("<?php list(,) = array(1,2,3);");
  MVCR("<?php var_dump(list($a,) = array(1,2,3)); var_dump($a);");
  MVCR("<?php var_dump(list(,$b) = array(1,2,3)); var_dump($b);");
  MVCR("<?php var_dump(list($b) = array(1,2,3)); var_dump($b);");
  MVCR("<?php var_dump(list($a,$b) = array(1,2,3)); "
      "var_dump($a); var_dump($b);");
  MVCR("<?php var_dump(list($a,list($c),$b) = array(1,array(2),3));"
      "var_dump($a); var_dump($b); var_dump($c);");
  MVCR("<?php $c = 'old'; var_dump(list($a,list($c),$b) = array(1,'test',3));"
      "var_dump($a); var_dump($b); var_dump($c);");
  MVCR("<?php var_dump(list($a,list(),$b) = array(1,array(2),3));"
      "var_dump($a); var_dump($b);");
  MVCR("<?php $info = array('coffee', 'brown', 'caffeine');"
      "list($a[0], $a[1], $a[2]) = $info;"
      "var_dump($a);");
  MVCR("<?php "
      "class obj implements arrayaccess {"
      "    private $container = array();"
      "    public function __construct() {"
      "        $this->container = array("
      "            'one'   => 1,"
      "            'two'   => 2,"
      "            'three' => 3,"
      "        );"
      "    }"
      "    public function offsetSet($offset, $value) {"
      "        $this->container[$offset] = $value;"
      "    }"
      "    public function offsetExists($offset) {"
      "        return isset($this->container[$offset]);"
      "    }"
      "    public function offsetUnset($offset) {"
      "        unset($this->container[$offset]);"
      "    }"
      "    public function offsetGet($offset) {"
      "        return isset($this->container[$offset]) ? $this->container[$offset] : null;"
      "    }"
      "}"
      "class SetTest {"
      "  private $_vals = array("
      "      'one'   => 1,"
      "      'two'   => 2,"
      "      'three' => 3,"
      "      );"
      "  public function __set($name, $value) {"
      "    $this->_vals[$name] = $value;"
      "  }"
      "}"
      "$o = new obj;"
      "$q = list($o['one'], $o['two'], list($o['three'])) ="
      "  array('eins', 'zwei', array('drei'));"
      "var_dump($o);"
      "var_dump($q);"
      "$x = new SetTest;"
      "$qq = list($x->one, $x->two, list($x->three)) = 1;"
      "var_dump($x);"
      "$qq = list($x->one, $x->two, list($x->three)) = $q;"
      "var_dump($x);"
      "var_dump($qq);");

  MVCR("<?php "
       "function test($a) {"
       "  list($a[0], $a[1], $a) = $a;"
       "  var_dump($a);"
       "  }"
       "test(array('abc', 'cde', 'fgh'));");

  MVCR("<?php "
       "function test($a, $b, $i) {"
       "  list($a[$i++], $a[$i++], $a[$i++]) = $b;"
       "  var_dump($a);"
       "  }"
       "test(array(), array('x', 'y', 'z'), 0);");

  MVCR("<?php "
       "$i = 0; "
       "list($a[$i++], list($a[$i++], $a[$i++]), $a[$i++]) = "
       "    array('x', array('y1', 'y2'), 'z'); "
       "var_dump($a);");

  MVCR("<?php "
       "function foo($a) {"
       "  list($x, $y) = 'x'.$a;"
       "  return $x + $y;"
       "}");

  MVCR("<?php "
       "class X implements ArrayAccess {"
       "  function offsetget($n) { return $n; }"
       "  function offsetset($n,$v) { }"
       "  function offsetexists($n) { return true; }"
       "  function offsetunset($n) {}"
       "}"
       "list($a,$b) = new X;"
       "var_dump($a, $b);"
       "$x = 'foo';"
       "$y = 'bar';"
       "list($a, $b) = $x.$y;"
       "var_dump($a,$b);"
       "$z = $x.$y;"
       "list($a, $b) = $z;"
       "var_dump($a, $b);");

  return true;
}

bool TestCodeRun::TestExceptions() {
  MVCR("<?php\n"
       "class MyException extends Exception {\n"
       "  public function __construct() {\n"
       "  }\n"
       "}\n"
       "function thrower() {\n"
       "  throw new MyException();\n"
       "}\n"
       "try {\n"
       "  thrower();\n"
       "} catch (Exception $exn) {\n"
       "  $a = $exn->getTrace(); foreach($a as &$b) $b['file'] = 'string';\n"
       "  var_dump($a);\n"
       "  var_dump($exn->getLine());\n"
       "}\n"
      );

  MVCR("<?php\n"
       "\n"
       "class Exception1 extends Exception {\n"
       "  public function __Construct() {\n"
       "    parent::__construct();\n"
       "  }\n"
       "}\n"
       "\n"
       "class Exception2 extends Exception1 {\n"
       "  public function exception2() {\n"
       "    parent::__construct();\n"
       "  }\n"
       "}\n"
       "\n"
       "function foo() {\n"
       "  throw new Exception2();\n"
       "}\n"
       "\n"
       "function bar() {\n"
       "  try {\n"
       "    foo();\n"
       "  } catch (Exception $exn) {\n"
       "    $a = $exn->getTrace(); foreach($a as &$b) $b['file'] = 'string';\n"
       "    var_dump($a);\n"
       "    var_dump($exn->getLine());\n"
       "  }\n"
       "}\n"
       "\n"
       "bar();\n");

  MVCR("<?php try { throw new Exception('test');} "
      "catch (Exception $e) {}");
  MVCR("<?php try { try { throw new Exception('test');} "
      "catch (InvalidArgumentException $e) {} } "
      "catch (Exception $e) { print 'ok';}");
  MVCR("<?php class E extends Exception {} "
      "try { throw new E(); } catch (E $e) { print 'ok';}");
  MVCR("<?php class E extends Exception {} class F extends E {}"
      "try { throw new F(); } catch (E $e) { print 'ok';}");
  MVCR("<?php class E extends Exception { function __toString(){ return 'E';}} "
      "class F extends E { function __toString() { return 'F';}}"
      "try { throw new F(); } catch (E $e) { print $e;}");
  MVCR("<?php "
      "class a extends Exception {"
      "  function __destruct() {"
      "    var_dump('__destruct');"
      "  }"
      "};"
      "function foo() {"
      "  $ex = null;"
      "  try {"
      "    throw new A;"
      "  } catch (Exception $ex) {"
      "    var_dump(1);"
      "  }"
      "  var_dump(2);"
      "}"
      "foo();");
  MVCR("<?php "
      "function foo($a, $b) { return $a + $b; }"
      "function myErrorHandler($errno, $errstr, $errfile, $errline) {"
      "  var_dump($errstr, $errline);"
      "}"
      "$old_error_handler = set_error_handler('myErrorHandler');"
      ""
      "function bar($a, $b) {"
      "  if ($a) {"
      "    $value = $a * foo(1, 2);"
      "  }"
      "  return 1 / $b;"
      "}"
      "set_error_handler('myErrorHandler');"
      "$r = bar(1, 0);");
  MVCR("<?php "
       "class a extends Exception {};"
       "class b extends a {"
       "  function dump() {"
       "    echo 'c:', $this->code, '\nm:', $this->message, '\n';"
       "    echo 'x:', $this->x, '\ny:', $this->y, '\n';"
       "  }"
       "}"
       "if (0) { class a extends Exception {} }"
       "try {"
       "  throw(new b(1, 2));"
       "} catch (b $e) {"
       "  $e->dump();"
       "}");
  MVCR("<?php "
       "class X {"
       "  static function eh($errno, $errstr) {"
       "    echo \"eh: $errno\\n\";"
       "    die;"
       "  }};"
       "set_error_handler(array('X', 'eh'));"
       "$g = array();"
       "echo $g['foobar'];");

  return true;
}

bool TestCodeRun::TestPredefined() {
  MVCR("<?php \n\n\nvar_dump(/*__FILE__, */__LINE__);");
  MVCR("<?php function Test() { var_dump(__FUNCTION__);} "
       "var_dump(__FUNCTION__); test();");
  MVCR("<?php class A { "
       "function TestR() { var_dump(__CLASS__, __METHOD__);} "
       "static function Testm() { var_dump(__CLASS__, __METHOD__);}} "
       "function Testf() { var_dump(__CLASS__, __METHOD__);} "
       "testf(); A::testm(); $obj = new A(); $obj->testr();");
  MVCR("<?php class A { const Foo = __METHOD__;} var_dump(A::Foo);");
  return true;
}

bool TestCodeRun::TestLabels() {
  MVCR("<?php\n"
       "$modalit\xe9 = 'extended ASCII'; var_dump($modalit\xe9);\n"
       "${\"a-b\"} = 'dash'; var_dump(${\"a-b\"});\n"
       "${'a\"b'} = 'quote'; var_dump(${'a\"b'});\n"
       "${'a$b'} = 'dollar'; var_dump(${'a$b'});\n");

  MVCR("<?php\n"
       "define('modalit\xe9', 123); var_dump(modalit\xe9);");

  MVCR("<?php\n"
       "function modalit\xe9($a) { var_dump($a);} modalit\xe9(123);");

  MVCR("<?php\n"
       "class modalit\xe9 { static function odalit\xe9() { var_dump(123);} } "
       "modalit\xe9::odalit\xe9();");

  return true;
}

bool TestCodeRun::TestPerfectVirtual() {
  MVCR("<?php "
       "class A { function foo() { var_dump(__CLASS__);}} "
       "class B extends A { function foo() { var_dump(__CLASS__);}} "
       "function bar() { "
       "  $obj = new A; $obj->foo();"
       "  $obj = new B; $obj->foo();"
       "} bar();"
      );
  MVCR("<?php "
       "class A { function foo($a = 123) { var_dump(__CLASS__);}} "
       "class B extends A { function foo($b = 123) { var_dump(__CLASS__);}} "
       "function bar() { "
       "  $obj = new A; $obj->foo();"
       "  $obj = new B; $obj->foo();"
       "} bar();"
      );
  MVCR("<?php "
       "class A { function foo() { "
       "$args = func_get_args(); var_dump(__CLASS__, $args);}} "
       "class B extends A { function foo() { "
       "$args = func_get_args(); var_dump(__CLASS__, $args);}} "
       "function bar() { "
       "  $obj = new A; $obj->foo(123);"
       "  $obj = new B; $obj->foo(123, 456);"
       "} bar();"
      );
  MVCR("<?php "
       "class A { const X=1; function foo($a = self::X) { var_dump($a);}} "
       "class B extends A { const X=2; function foo($b = self::X) {"
       " var_dump($b);}} "
       "function bar() { "
       "  $obj = new A; $obj->foo();"
       "  $obj = new B; $obj->foo();"
       "} bar();"
      );
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// type system

bool TestCodeRun::TestBoolean() {
  MVCR("<?php var_dump(true);");
  MVCR("<?php var_dump(false);");
  MVCR("<?php $a = 1; $b = ($a == 1); var_dump($b);");
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestInteger() {
  MVCR("<?php var_dump(1);");
  MVCR("<?php var_dump(0);");
  MVCR("<?php var_dump(-1);");
  MVCR("<?php var_dump(8589934592);");
  MVCR("<?php var_dump(-8589934592);");
  MVCR("<?php $a = 1;           var_dump($a);");
  MVCR("<?php $a = 0;           var_dump($a);");
  MVCR("<?php $a = -1;          var_dump($a);");
  MVCR("<?php $a = 8589934592;  var_dump($a);");
  MVCR("<?php $a = -8589934592; var_dump($a);");

  MVCR("<?php $a = 10; var_dump(~$a);");
  MVCR("<?php $a = 10; $b = 9; var_dump($a & $b);");
  MVCR("<?php $a = 10; $b = 9; var_dump($a | $b);");
  MVCR("<?php $a = 10; $b = 9; var_dump($a ^ $b);");
  MVCR("<?php $a = 10; var_dump($a << 2);");
  MVCR("<?php $a = 10; var_dump($a >> 2);");

  MVCR("<?php $a = 10; $b = 9; $a &= $b; var_dump($a);");
  MVCR("<?php $a = 10; $b = 9; $a |= $b; var_dump($a);");
  MVCR("<?php $a = 10; $b = 9; $a ^= $b; var_dump($a);");
  MVCR("<?php $a = 10; $b = 9; $a <<= 2; var_dump($a);");
  MVCR("<?php $a = 10; $b = 9; $a >>= 2; var_dump($a);");

  MVCR("<?php "
      "var_dump((integer)'10');"
      "var_dump((integer)'0x10');"
      "var_dump((integer)'010');"
      "var_dump(10 + 0x10);"
      "var_dump(10 + 010);");

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestDouble() {
  MVCR("<?php var_dump(1.0);");
  MVCR("<?php var_dump(0.0);");
  MVCR("<?php var_dump(-1.0);");
  MVCR("<?php var_dump(1/3);");
  MVCR("<?php $a = 1/3; var_dump($a);");
  MVCR("<?php $a = 1/3; $b = $a; var_dump($b);");

  MVCR("<?php "
      "$a = 1.234;"
      "echo $a;"
      "$b = 1.2e3;"
      "echo $b;"
      "$c = 7E-10;"
      "echo $c;"
      "echo floor((0.1+0.7)*10);"
      "echo round((69.1-floor(69.1)));"
      "echo round(69.1-floor(69.1));"
      "echo (63.1-floor(63.1));"
      "echo (64.0-floor(64.0));"
      "echo round(64.1-floor(64.1));"
      "echo round(3.1415927,2);"
      "echo round(1092,-2);");
  MVCR("<?php "
      "$foo = 1 + \"10.5\";"
      "print(\"$foo \");"
      "$foo = 1 + \"-1.3e3\";"
      "print(\"$foo \");"
      "$foo = 1 + \"bob-1.3e3\";"
      "print(\"$foo \");"
      "$foo = 1 + \"bob3\";"
      "print(\"$foo \");"
      "$foo = 1 + \"10 Small Pigs\";"
      "print(\"$foo \");"
      "$foo = 4 + \"10.2 Little Piggies\";"
      "print(\"$foo \");"
      "$foo = \"10.0 pigs \" + 1;"
      "print(\"$foo \");"
      "$foo = \"10.0 pigs \" + 1.0;"
      "print(\"$foo \");");
  MVCR("<?php "
      "var_dump(1E1);"
      "var_dump(1E2);"
      "var_dump(1E3);"
      "var_dump(1E4);"
      "var_dump(1E5);"
      "var_dump(1E6);"
      "var_dump(1E7);"
      "var_dump(1E8);"
      "var_dump(1E9);"
      "var_dump(1E10);"
      "var_dump(1E11);"
      "var_dump(1E12);"
      "var_dump(1E13);"
      "var_dump(1E14);"
      "var_dump(1E15);"
      "var_dump(1E16);"
      "var_dump(1E17);"
      "var_dump(1E18);"
      "var_dump(1E19);"
      "var_dump(1E20);");
  MVCR("<?php "
      "var_dump((double)'10');"
      "var_dump((double)'0x10');"
      "var_dump((double)'010');"
      "var_dump(10.0 + 0x10);"
      "var_dump(10.0 + 010.0);");

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestString() {
  MVCR("<?php print '\\'\\\\\"';");
  MVCR("<?php print 'test\nok';");
  MVCR("<?php print \"test\nok\";");
  MVCR("<?php print \"test\\n\\r\\t\\v\\f\\\\\\$\\\"\";");
  MVCR("<?php print \"\\1\\12\\123\\1234\\xA\\xAB\";");
  MVCR("<?php print 'test\\n';");

  MVCR("<?php $a = 'test'; $b = $a; print $b;");
  MVCR("<?php $a = 'test'; $b = $a; $a = 'changed'; print $b;");
  MVCR("<?php $a = 'test'; $b = $a; $a = 'changed'; print $a;");
  MVCR("<?php $a = 'test'; $b = $a; $b = 'changed'; print $a;");

  MVCR("<?php $a = 'a'; $b = 'b'; $c = 'a' . 'b'; print $c;");
  MVCR("<?php $a = 'a'; $b = 'b'; $c = $a  . 'b'; print $c;");
  MVCR("<?php $a = 'a'; $b = 'b'; $c = 'a' . $b ; print $c;");
  MVCR("<?php $a = 'a'; $b = 'b'; $b .= $a;       print $b;");

  MVCR("<?php $a = 'test'; print $a{0};");
  MVCR("<?php $a = 'test'; print '['.$a{-1}.']';");
  MVCR("<?php $a = 'test'; print '['.$a{100}.']';");

  MVCR("<?php $a = 'test'; print $a[0];");
  MVCR("<?php $a = 'test'; print $a['junk'];");
  MVCR("<?php $a = 'test'; print '['.$a[-1].']';");
  MVCR("<?php $a = 'test'; print '['.$a[100].']';");

  MVCR("<?php $a = 'test'; $a[0] = 'ABC'; var_dump($a);")
  MVCR("<?php $a = 'test'; $a[10] = 'ABC'; var_dump($a);")
  MVCR("<?php $a = 'test'; $b = $a; $a[0] = 'ABC'; var_dump($a); var_dump($b);")
  MVCR("<?php $a = 'test'; $b = $a; $a[10] = 'ABC'; var_dump($a);var_dump($b);")
  MVCR("<?php $a = 'test'; $b = $a; $b[0] = 'ABC'; var_dump($a); var_dump($b);")
  MVCR("<?php $a = 'test'; $b = $a; $b[10] = 'ABC'; var_dump($a);var_dump($b);")

  MVCR("<?php $a = 'test'; var_dump(~$a);");
  MVCR("<?php $a = 'test'; $b = 'zzz'; var_dump($a & $b);");
  MVCR("<?php $a = 'test'; $b = 'zzz'; var_dump($a | $b);");
  MVCR("<?php $a = 'test'; $b = 'zzz'; var_dump($a ^ $b);");
  MVCR("<?php $a = 'test'; $b = 'zzz'; $a &= $b; var_dump($a);");
  MVCR("<?php $a = 'test'; $b = 'zzz'; $a |= $b; var_dump($a);");
  MVCR("<?php $a = 'test'; $b = 'zzz'; $a ^= $b; var_dump($a);");
  MVCR("<?php $a = 'zzz'; $b = 'test'; var_dump($a & $b);");
  MVCR("<?php $a = 'zzz'; $b = 'test'; var_dump($a | $b);");
  MVCR("<?php $a = 'zzz'; $b = 'test'; var_dump($a ^ $b);");
  MVCR("<?php $a = 'zzz'; $b = 'test'; $a &= $b; var_dump($a);");
  MVCR("<?php $a = 'zzz'; $b = 'test'; $a |= $b; var_dump($a);");
  MVCR("<?php $a = 'zzz'; $b = 'test'; $a ^= $b; var_dump($a);");
  MVCR("<?php $a = 'zzz'; $a++; var_dump($a);");
  MVCR("<?php $a = 'zzz'; ++$a; var_dump($a);");
  MVCR("<?php $a = 'zzz'; $a--; var_dump($a);");
  MVCR("<?php $a = 'zzz'; --$a; var_dump($a);");

  MVCR("<?php $a = 'abc'; var_dump(isset($a[1], $a[2], $a[3]));");

  // serialization of binary string
  MVCR("<?php var_dump(bin2hex(serialize(\"a\\x00b\")));");
  MVCR("<?php var_dump(json_encode(\"\\0001\"));");

  MVCR("<?php "
      "$a = array('x'=>'foo');"
      "$b = 'qqq';"
      "class c {}"
      "$c = new c;"
      "$c->p = 'zzz';"
      "var_dump(\"AAA ${a['x']} $a[x] $b $c->p\");");

  MVCR("<?php "
       "function test($a, $b) {"
       "  $buf = 'hello';"
       "  foreach ($a as $v) {"
       "    $buf .= $v . ';';"
       "    foreach ($b as $w) {"
       "      $buf .= $w;"
       "    }"
       "  }"
       "  var_dump($buf);"
       "}"
       "test(array('a', 'b', 'c'), array('u', 'v'));"
       "function test2($a, $b) {"
       "  $buf = 'hello';"
       "  foreach ($a as $v) {"
       "    $buf .= $v . ';';"
       "    foreach ($b as $w) {"
       "      $buf .= $w;"
       "    }"
       "    echo $buf;"
       "  }"
       "  var_dump($buf);"
       "}"
       "test2(array('a', 'b', 'c'), array('u', 'v'));"
       "function test3($a, $b) {"
       "  $buf = 'hello';"
       "  foreach ($a as $v) {"
       "    $buf .= $v . ';';"
       "    foreach ($b as $w) {"
       "      echo ($buf .= $w);"
       "    }"
       "  }"
       "  var_dump($buf);"
       "}"
       "test3(array('a', 'b', 'c'), array('u', 'v'));");

  MVCR("<?php "
       "function f() { return 'x'; }"
       "function g() {}"
       "function test1($a) {"
       "  $buf = '';"
       "  foreach ($a as $s) {"
       "     $buf .= f() . g() . 'h' . f() . 'h' . g();"
       "  }"
       "  foreach ($a as $s) {"
       "    $buf .= ($s . 'h' . $s);"
       "  }"
       "  return $buf;"
       "}"
       "var_dump(test1(array(1)));"
       "function test2() {"
       "  return f() . g() . f() . g();"
       "}"
       "var_dump(test2());"
       "function test3() {"
       "  return f() . g() . f() . g() . f() . g() . f() . g() . f();"
       "}"
       "var_dump(test3());"
       "function test4() {"
       "  $s = f();"
       "  $s .="
       "    ('foo'."
       "    'bar'."
       "    f()."
       "    'foo'."
       "    'baz'."
       "    f()."
       "    'fuz'."
       "    'boo'."
       "    f()."
       "    'fiz'."
       "     'faz');"
       ""
       "  $s .= f();"
       "  return $s;"
       "}"
       "var_dump(test4());"
       "function test5() {"
       "  return g().g().g().g();"
       "}"
       "var_dump(test5());"
       "function test6() {"
       "  return g().f().g();"
       "}"
       "var_dump(test6());");

  MVCR("<?php "
       "class X {"
       " static function g() {}"
       "};"
       "echo 'abc' . X::g() . 'efg';");

  MVCR("<?php "
       "$s = 'x';"
       "var_dump(strrpos($s.'0', $s));"
       "for ($i = -7; $i < 7; $i++) {"
       "  echo $i,':';var_dump(strrpos('xabcay', 'a',$i));"
       "}");

  MVCR("<?php\n"
       "$a = 'zz';\n"
       "$a++;\n"
       "$b = 'zz';\n" // $b should remain 'zz'
       "var_dump($a, $b);\n");

  MVCR("<?php\n"
       "function test($s) {\n"
       "  $a = array('abc' => 1, 'abcd' => 2);\n"
       "  $s .= 'c'; var_dump($a[$s]);\n"
       "  $s .= 'd'; var_dump($a[$s]);\n" // should find 'abcd' in $a
       "}\n"
       "test('ab');\n");
  MVCR("<?php\n"
       "function foo() {"
       "  $a = '';"
       "  $a++;"
       "  var_dump($a);"
       "  $a = '';"
       "  ++$a;"
       "  var_dump($a);"
       "  $a = '';"
       "  $a--;"
       "  var_dump($a);"
       "  $a = '';"
       "  --$a;"
       "  var_dump($a);"
       "  $a = '@';"
       "  $a++;"
       "  var_dump($a);"
       "  $a = '@';"
       "  ++$a;"
       "  var_dump($a);"
       "  $a = '@';"
       "  $a--;"
       "  var_dump($a);"
       "  $a = '@';"
       "  --$a;"
       "  var_dump($a);"
       "}"
       "foo();");
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestArray() {
  MVCR("<?php var_dump(array('b' => '2', 'a' => '1'));");
  MVCR("<?php var_dump(array(1 => 'a', 0 => 'b'));");

  MVCR("<?php $a = array();                       var_dump($a);");
  MVCR("<?php $a = array(1);                      var_dump($a);");
  MVCR("<?php $a = array(2, 1);                   var_dump($a);");
  MVCR("<?php $a = array('1');                    var_dump($a);");
  MVCR("<?php $a = array('2', '1');               var_dump($a);");
  MVCR("<?php $a = array('a' => 1);               var_dump($a);");
  MVCR("<?php $a = array('b' => 2, 'a' => 1);     var_dump($a);");
  MVCR("<?php $a = array('a' => '1');             var_dump($a);");
  MVCR("<?php $a = array('b' => '2', 'a' => '1'); var_dump($a);");

  MVCR("<?php $a = array('a' => 1, 'a' => 2); var_dump($a);");
  MVCR("<?php $a = array('a' => 1, 'b' => 2, 'a' => 3); var_dump($a);");

  MVCR("<?php $a = array(1); $b = $a;                var_dump($b);");
  MVCR("<?php $a = array(1); $b = $a; $a = array(2); var_dump($b);");
  MVCR("<?php $a = array(1); $b = $a; $a = array(2); var_dump($a);");
  MVCR("<?php $a = array(1); $b = $a; $b = array(2); var_dump($a);");

  MVCR("<?php $a = array(); foreach ($a as $item) print '['.$item.']';");
  MVCR("<?php $a = array(1); foreach ($a as $item) print '['.$item.']';");
  MVCR("<?php $a = array(2,1); foreach ($a as $item) print '['.$item.']';");
  MVCR("<?php $a = array('b' => 2, 'a' => 1); "
      "foreach ($a as $item) print '['.$item.']';");
  MVCR("<?php $a = array('b' => 2, 'a' => 1); "
      "foreach ($a as $name => $item) print '['.$name.'=>'.$item.']';");

  MVCR("<?php $a = array(2,1); var_dump($a[0]);");
  MVCR("<?php $a = array(2,1); var_dump($a[-1]);");
  MVCR("<?php $a = array(2,1); var_dump($a[3]);");
  MVCR("<?php $a = array('b' => 2, 'a' => 1); var_dump($a['b']);");
  MVCR("<?php $a = array('b' => 2, 'a' => 1); var_dump($a['bogus']);");
  MVCR("<?php $a = array(2, 'test' => 1); var_dump($a);");
  MVCR("<?php $a = array(1.2 => 'test'); var_dump($a[1]);");

  MVCR("<?php $a = array(1, 'test'); var_dump($a);");
  MVCR("<?php $a = array(); $a[] = 3; $a[] = 'test'; var_dump($a);");

  MVCR("<?php $a = array();     $a['test'] = 3; var_dump($a);");
  MVCR("<?php $a = array(1);    $a['test'] = 3; var_dump($a);");
  MVCR("<?php $a = array(1, 2); $a[10] = 3;     var_dump($a);");
  MVCR("<?php $a = array(1, 2); $a['10'] = 3;   var_dump($a);");
  MVCR("<?php $a = array(1, 2); $b = $a; $a['10'] = 3; var_dump($b);");
  MVCR("<?php $a = array(1, 2); $b = $a; $a['10'] = 3; var_dump($a);");

  MVCR("<?php $a[] = 3; var_dump($a);");
  MVCR("<?php $a = array(); $a[] = 3; var_dump($a);");
  MVCR("<?php $a = array(1); $a[] = 3; var_dump($a);");
  MVCR("<?php $a = array(1,2); $a[] = 3; var_dump($a);");
  MVCR("<?php $a = ''; $a[] = 'test'; var_dump($a);");

  MVCR("<?php $a = array(1, 2); "
      "foreach ($a as $item) { "
      "  print 'A['.$item.']'; "
      "  if ($item == 1) $a[] = 'new item'; "
      "} "
      "foreach ($a as $item) { "
      "  print 'B['.$item.']'; "
      "}"
      "var_dump($a);");

  MVCR("<?php $a = array(1); $b = array(2); $c = $a + $b; var_dump($c);");
  MVCR("<?php $a = array(1,2); $b = array(2,3); $c = $a + $b; var_dump($c);");
  MVCR("<?php $a = array(1,2); $b = array(2); $c = $a + $b; var_dump($c);");
  MVCR("<?php $a = array(1); $b = array(2,3); $c = $a + $b; var_dump($c);");

  MVCR("<?php "
      "$array_variables = array("
      "  array(),"
      "  array(NULL),"
      "  array("")"
      ");"
      "foreach ($array_variables as $array_var) {"
      "  $keys = array_keys($array_var);"
      "  foreach ($keys as $key_value) {"
      "    echo $key_value;"
      "  }"
      "}");

  MVCR("<?php $a = array('a' => 1, 'b' => 2);"
      "foreach ($a as $b => $c) {"
      "  var_dump($b);"
      "  unset($a['b']);"
      "}");

  MVCR("<?php $a = array('a' => 1, 'b' => 2);"
      "foreach ($a as $b => &$c) {"
      "  var_dump($b);"
      "  unset($a['b']);"
      "}");

  MVCR("<?php "
      "$foo = array(1,2,3,4);"
      "foreach ($foo as $key => $val) {"
      "  if($val == 2) {"
      "    $foo[$key] = 0;"
      "  } else if($val == 3) {"
      "    unset($foo[$key]);"
      "  } else {"
      "    $foo[$key] = $val + 1;"
      "  }"
      "}"
      "var_dump($foo);");

  MVCR("<?php "
      "$foo = array(1,2,3,4);"
      "foreach ($foo as $key => &$val) {"
      "  if($val == 2) {"
      "    $foo[$key] = 0;"
      "  } else if($val == 3) {"
      "    unset($foo[$key]);"
      "  } else {"
      "    $val++;"
      "  }"
      "}"
      "var_dump($foo);");

  MVCR("<?php "
      "$a = array('a' => 'apple', 'b' => 'banana', 'c' => 'citrus');"
      "foreach ($a as $k1 => &$v1) {"
      "  foreach ($a as $k2 => &$v2) {"
      "    if ($k2 == 'a') {"
      "      unset($a[$k2]);"
      "    }"
      "    var_dump($v1, $v2);"
      "  }"
      "}");
  MVCR("<?php "
      "$stack = array();"
      "function push_stack()"
      "{"
      "  global $stack;"
      "  static $index = 0;"
      "  $val = $index++;"
      "  array_push($stack, $val);"
      "}"
      "function pop_stack()"
      "{"
      "  global $stack;"
      "  if ($stack) {"
      "    array_pop($stack);"
      "  }"
      "}"
      "push_stack();"
      "pop_stack();"
      "pop_stack();"
      "pop_stack();"
      "push_stack();"
      "pop_stack();"
      "push_stack();"
      "$info = array(count($stack), $stack[count($stack)-1], $stack);"
      "var_dump($info);");
  MVCR("<?php "
      "class A { function f($a) { var_dump($a === null); } }"
      "$a = true; $a = new A();"
      "$a->f(array());");

  MVCR("<?php "
       "function test($a) {"
       "  var_dump(current($a));"
       "  while (next($a)) echo '.';"
       "  $a[] = 2;"
       "  $a[] = 3;"
       "  var_dump(current($a));"
       "  var_dump(next($a));"
       "  var_dump(next($a));"
       "  var_dump(current($a));"
       "  $a[17] = 4;"
       "  var_dump(current($a));"
       "  $a[18] = 5;"
       "  var_dump(current($a));"
       "  while(next($a)) echo '.';"
       "  var_dump(current($a));"
       "  $a[1] = 5;"
       "  var_dump(current($a));"
       "}"
       "test(array(1));"
       "test(array(1,2,3,4,5,6,7,8,9));");

  MVCR("<?php "
      "function foo() {"
      "  $a = array();"
      "  $a[] = '1.1';"
      "  $a[] = '2.2';"
      "  $a[] = '3.3';"
      "  var_dump(array_sum($a));"
      "  var_dump(array_product($a));"
      "}"
      "foo();");

  MVCR("<?php "
      "$array = array('1' => array(2 => 'test'));"
      "unset($array['1'][2]);"
      "var_dump($array['1']);");

  MVCR("<?php "
      "$x['1'] += 1;"
      "var_dump($x);");

  MVCR("<?php "
       "function foo($x) { var_dump($x); }"
       "function test() {"
       "  $data = null;"
       "  $data['bar']['baz'] = 1;"
       "  foo($data);"
       "}"
       "test();");

  return true;
}

bool TestCodeRun::TestArrayInit() {
  MVCR("<?php\n"
       "class MyClass { function __toString() { return 'foo'; } }\n"
       "$obj = new MyClass();\n"
       "$arr = array($obj => 1, new MyClass() => 2, "
       "false => 3, true => 4, count(array(1,2,3)) => 5);\n"
       "var_dump($arr);\n");
  MVCR("<?php\n"
       "function f() { throw new Exception(); }\n"
       "function test() {\n"
       "  $a = array(1, f(), 2, f(), 3);\n"
       "  var_dump($a);\n"
       "}\n"
       "try { test(); } catch (Exception $e) { }\n");
  MVCR("<?php\n"
       "function test($x, $y) {"
       "$a = array($x, $y);"
       "$a[] = 3;"
       "return $a;"
       "}"
       "var_dump(test(1,2));");
  MVCR("<?php\n"
       "function foo($p) {"
       "  $a = array('a', 'b', $p);"
       "  $a[] = 'd';"
       "  var_dump($a);"
       "  $a = array(0 => 'a', 1 => 'b', 2 => $p);"
       "  $a[] = 'd';"
       "  var_dump($a);"
       "  $a = array(2 => 'a', 4 => 'b', 6 => $p);"
       "  $a[] = 'd';"
       "  var_dump($a);"
       "  $a = array(-2 => 'a', -4 => 'b', -6 => $p);"
       "  $a[] = 'd';"
       "  var_dump($a);"
       "  $a = array(0 => 'a');"
       "  $a[] = 'b';"
       "  var_dump($a);"
       "}"
       "foo('c');");
  MVCR("<?php\n"
      "$v = 1;"
      "function foo($a) {"
      "  $arr = array($a, $a++, $a);"
      "  var_dump($arr);"
      "}"
      "foo($v);");
  return true;
}

bool TestCodeRun::TestArrayCopy() {
  MVCR("<?php function h1() {\n"
       "  $x = array(1,2,3,4);\n"
       "  next($x);\n"
       "  $y = $x;\n"
       "  unset($y[2]);\n"
       "  var_dump(current($x));\n"
       "  var_dump(current($y));\n"
       "}\n"
       "h1();\n");

  MVCR("<?php function h2() {\n"
       "  $x = array(1,2,3,4);\n"
       "  next($x);\n"
       "  $y = $x;\n"
       "  $y[] = 4;\n"
       "  var_dump(current($x));\n"
       "  var_dump(current($y));\n"
       "}\n"
       "h2();\n");

  MVCR("<?php function h3() {\n"
       "  $x = array(1,2,3,4);\n"
       "  next($x);\n"
       "  $y = $x;\n"
       "  array_pop($y);\n"
       "  var_dump(current($x));\n"
       "  var_dump(current($y));\n"
       "}\n"
       "h3();\n");

  /**
   * Zend PHP 5.2 outputs:
   *   bool(false)
   *   int(1)
   *
   * The difference in behavior is intentional. Under Zend PHP, the first call
   * to current triggers an array copy, and because the original array's
   * internal iterator points past the end, the copy's internal iterator is
   * reset. This behavior exposes information to user code about when array
   * copies are triggered.
   *
   * Under HPHP, we always leave the internal iterator intact when making an
   * array copy. The advantage here is that we do not expose information about
   * when array copies are triggered to user code.
   */
  MVCRO("<?php function h4() {\n"
       "  $x = array(1,2,3,4);\n"
       "  end($x);\n"
       "  next($x);\n"
       "  $y = $x;\n"
       "  unset($y[2]);\n"
       "  var_dump(current($x));\n"
       "  var_dump(current($y));\n"
       "}\n"
       "h4();\n"
       ,
       "bool(false)\n"
       "bool(false)\n"
       );

  /**
   * Zend PHP 5.2 outputs:
   *   bool(false)
   *   int(1)
   *
   * The difference in behavior is intentional. Under Zend PHP, when 4 is
   * appended to $y, it triggers an array copy which resets $y's internal
   * iterator. This is why current($y) returns 1.
   *
   * Under HPHP, when 4 is appended to $y, it triggers an array copy. However,
   * $y's internal iterator is not reset; it continues to point past the last
   * element. Then when the append operation actually executes, it updates the
   * internal iterator to point to the newly appended element. For more info
   * see the h4 testcase.
   */
  MVCRO("<?php function h5() {\n"
       "  $x = array(1,2,3,4);\n"
       "  end($x);\n"
       "  next($x);\n"
       "  $y = $x;\n"
       "  $y[] = 4;\n"
       "  var_dump(current($x));\n"
       "  var_dump(current($y));\n"
       "}\n"
       "h5();\n"
       ,
       "bool(false)\n"
       "int(4)\n"
       );

  MVCR("<?php function h6() {\n"
       "  $x = array(1,2,3,4);\n"
       "  end($x);\n"
       "  next($x);\n"
       "  $y = $x;\n"
       "  array_pop($y);\n"
       "  var_dump(current($x));\n"
       "  var_dump(current($y));\n"
       "}\n"
       "h6();\n");

  /**
   * Zend PHP 5.2 outputs:
   *   int(0)
   *   bool(false)
   *
   * The difference in behavior is intentional. For more info see testcase h4.
   */
  MVCRO("<?php function h7() {\n"
       "  $arr = array(0,1,2,3,4);\n"
       "  end($arr);\n"
       "  next($arr);\n"
       "  $arr2 = $arr;\n"
       "  var_dump(current($arr));\n"
       "  var_dump(current($arr2));\n"
       "}\n"
       "h7();\n"
       ,
       "bool(false)\n"
       "bool(false)\n"
       );

  /**
   * Zend PHP 5.2 outputs:
   *   int(0)
   *   bool(false)
   *
   * The difference in behavior is intentional. For more info see testcase h4.
   */
  MVCRO("<?php function h8() {\n"
       "  $arr = array(0,1,2,3,4);\n"
       "  end($arr);\n"
       "  next($arr);\n"
       "  $arr2 = $arr;\n"
       "  var_dump(current($arr2));\n"
       "  var_dump(current($arr));\n"
       "}\n"
       "h8();\n"
       ,
       "bool(false)\n"
       "bool(false)\n"
       );

  /**
   * Zend PHP 5.2 outputs:
   *   array(1) {
   *     [0]=>
   *     array(1) {
   *       [0]=>
   *       *RECURSION*
   *     }
   *   }
   * The difference in behavior is intentional, and HPHP is consistent with
   * Zend PHP 5.3.
   */
  MVCRO("<?php\n"
        "function f($a) { $a[0] = $a; var_dump($a); }\n"
        "f(false);\n"
        ,
        "array(1) {\n"
        "  [0]=>\n"
        "  bool(false)\n"
        "}\n");

  /**
   * Zend PHP 5.2 outputs:
   *   array(2) {
   *     ["x"]=>
   *     bool(false)
   *     [0]=>
   *     array(2) {
   *       ["x"]=>
   *       bool(false)
   *       [0]=>
   *       *RECURSION*
   *     }
   *   }
   * The difference in behavior is intentional, and HPHP is consistent with
   * Zend PHP 5.3.
   */
  MVCRO("<?php\n"
        "function f($b) {\n"
        "  $a = $b ? 0 : array('x' => $b);\n"
        "  $a[0] = $a; var_dump($a);\n"
        "}\n"
        "f(false);\n"
        ,
        "array(2) {\n"
        "  [\"x\"]=>\n"
        "  bool(false)\n"
        "  [0]=>\n"
        "  array(1) {\n"
        "    [\"x\"]=>\n"
        "    bool(false)\n"
        "  }\n"
        "}\n");

  /**
   * Zend PHP 5.2 outputs:
   *   array(1) {
   *     [0]=>
   *     array(1) {
   *       [0]=>
   *       *RECURSION*
   *     }
   *   }
   * The difference in behavior is intentional, and HPHP is consistent with
   * Zend PHP 5.3.
   */
  MVCRO("<?php\n"
        "function f($a) { $a[] = $a; var_dump($a); }\n"
        "f(false);\n"
        ,
        "array(1) {\n"
        "  [0]=>\n"
        "  bool(false)\n"
        "}\n");

  /**
   * Even Zend PHP 5.3 does not get this right. It generates a recursive array.
   */
  MVCRO("<?php\n"
        "function f($b) {\n"
        "  $a = $b ? 0 : array('x' => $b);\n"
        "  $a2 = &$a; $a[] = $a2; var_dump($a);\n"
        "}\n"
        "f(false);\n"
        ,
        "array(2) {\n"
        "  [\"x\"]=>\n"
        "  bool(false)\n"
        "  [0]=>\n"
        "  array(1) {\n"
        "    [\"x\"]=>\n"
        "    bool(false)\n"
        "  }\n"
        "}\n");

  /**
   * Zend PHP 5.2 creates a recursive array, but HPHP and Zend PHP 5.3 do not.
   */
  MVCRO("<?php\n"
        "function f($b) {\n"
        "  $a = $b ? 0 : array($b);\n"
        "  $a[1][0] = $a; var_dump($a);\n"
        "}\n"
        "f(false);\n"
        ,
        "array(2) {\n"
        "  [0]=>\n"
        "  bool(false)\n"
        "  [1]=>\n"
        "  array(1) {\n"
        "    [0]=>\n"
        "    array(1) {\n"
        "      [0]=>\n"
        "      bool(false)\n"
        "    }\n"
        "  }\n"
        "}\n");

  return true;
}


bool TestCodeRun::TestScalarArray() {
  MVCR("<?php "
      "$a1 = array();"
      "$a2 = array(null);"
      "$a3 = array(true);"
      "$a4 = array(false);"
      "$a5 = array(0);"
      "$a6 = array(1);"
      "$a7 = array(1.0);"
      "$a8 = array('1.0');"
      "$a9 = array(1.23456789e+34);"
      "$a13 = array(1.7976931348623157e+308);"
      "$a10 = array(1E666);"
      "$a11 = array(1E666/1E666);"
      "$a12 = array(\"a bc\");"
      "$a13 = array(\"\\xc1 bc\");"
      "$a14 = array(null, true, false, 0, 1, 1.0,"
      "             '1.0', 1.23456789e+34,"
      "             1.7976931348623157e+308, 1E666,"
      "             1E666/1E666, \"a bc\","
      "             \"\\xc1 bc\");"
      "$a15 = array(null => true, false => 0, 1 => 1.0,"
      "             '1.0' => 1.23456789e+34,"
      "             1.7976931348623157e+308 => 1E666,"
      "             1E666/1E666 => \"a bc\","
      "             \"\\xc1 bc\");"
      "$a16 = array(null => true, false => 0, 1 => 1.0,"
      "             '1.0' => 1.23456789e+34,"
      "             1.7976931348623157e+308 => 1E666,"
      "             1E666/1E666 => \"a bc\","
      "             \"\\xc1 bc\","
      "             array(null => true, array(),"
      "                   false => 0, 1 => 1.0,"
      "                   '1.0' => 1.23456789e+34,"
      "                   1.7976931348623157e+308 => 1E666,"
      "                   1E666/1E666 => \"a bc\","
      "                   \"\\xc1 bc\"));"
      "var_dump($a1);"
      "var_dump($a2);"
      "var_dump($a3);"
      "var_dump($a4);"
      "var_dump($a5);"
      "var_dump($a6);"
      "var_dump($a7);"
      "var_dump($a8);"
      "var_dump($a9);"
      "var_dump($a10);"
      "var_dump($a11);"
      "var_dump($a12);"
      "var_dump($a13);"
      "var_dump($a14);"
      "var_dump($a15);"
      "var_dump($a16);");
  MVCR("<?php\n"
      "function test1() { $a = array(__FUNCTION__, __LINE__); return $a; }\n"
      "function test2() { $a = array(__FUNCTION__, __LINE__); return $a; }\n"
      "var_dump(test1()); var_dump(test2());");

  MVCR("<?php "
       "define('VALUE', 1);"
       "function func($params) {"
       " var_dump($params);"
       "}"
       "func(array('key' => @VALUE));");

  return true;
}

bool TestCodeRun::TestRange() {
  MVCR("<?php "
      "foreach (range(\"0 xxx\", \"12 yyy\") as $number) {"
      "  echo $number . \"\n\";"
      "}"
      "foreach (range(\"0\", \"12\") as $number) {"
      "  echo $number . \"\n\";"
      "}"
      "foreach (range('a', 'i') as $letter) {"
      "  echo $letter;"
      "}"
      "foreach (range('c', 'a') as $letter) {"
      "  echo $letter;"
      "}"
      "foreach (range('a xxx', 'i yyy') as $letter) {"
      "  echo $letter;"
      "}"
      "foreach (range('c xxx', 'a yyy') as $letter) {"
      "  echo $letter;"
      "}");

  return true;
}

#define TEST_ARRAY_CONVERT(exp)                                         \
  MVCR("<?php $a = " exp "; $a[] = 1;              var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[] = 'test';         var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[] = array(0);       var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[0] = 1;             var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[0] = 'test';        var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[0] = array(0);      var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[1] = 1;             var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[1] = 'test';        var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[1] = array(0);      var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[2] = 1;             var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[2] = 'test';        var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a[2] = array(0);      var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a['test'] = 1;        var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a['test'] = 'test';   var_dump($a);");      \
  MVCR("<?php $a = " exp "; $a['test'] = array(0); var_dump($a);");      \

#define TEST_ARRAY_PLUS(exp)                                            \
  MVCR("<?php $a = " exp "; $a += array();                var_dump($a);"); \
  MVCR("<?php $a = " exp "; $a += array(20);              var_dump($a);"); \
  MVCR("<?php $a = " exp "; $a += array('b');             var_dump($a);"); \
  MVCR("<?php $a = " exp "; $a += array(array(3));        var_dump($a);"); \
  MVCR("<?php $a = " exp "; $a += array('c' => 20);       var_dump($a);"); \
  MVCR("<?php $a = " exp "; $a += array('c' => 'b');      var_dump($a);"); \
  MVCR("<?php $a = " exp "; $a += array('c' => array(3)); var_dump($a);"); \

bool TestCodeRun::TestArrayEscalation() {
  TEST_ARRAY_CONVERT("array()");
  TEST_ARRAY_CONVERT("array(10)");
  TEST_ARRAY_CONVERT("array('test')");
  TEST_ARRAY_CONVERT("array(array(0))");
  TEST_ARRAY_CONVERT("array('a' => 10)");
  TEST_ARRAY_CONVERT("array('a' => 'va')");
  TEST_ARRAY_CONVERT("array('a' => array(1))");

  TEST_ARRAY_PLUS("array()");
  TEST_ARRAY_PLUS("array(10)");
  TEST_ARRAY_PLUS("array('test')");
  TEST_ARRAY_PLUS("array(array(0))");
  TEST_ARRAY_PLUS("array('a' => 10)");
  TEST_ARRAY_PLUS("array('a' => 'va')");
  TEST_ARRAY_PLUS("array('a' => array(1))");
  return true;
}

bool TestCodeRun::TestArrayOffset() {
  MVCR("<?php $a['test_cache_2'] = 10; print $a['test_cache_26'];");
  MVCR("<?php $a = array(10); $b = $a[0]; var_dump($b);");
  MVCR("<?php $a = array(10); $b = $a[0] + 15; var_dump($b);");
  MVCR("<?php $a = 1; $a = array($a); $a = array($a); var_dump($a);");
  MVCR("<?php $a['A'] = array(1, 2); foreach ($a['A'] as $item) print $item;");
  MVCR("<?php $a['A']['B'] = 1; var_dump($a);");
  MVCR("<?php $a['A'] = 10; $a['A']++; var_dump($a);");
  MVCR("<?php $a['A'] = 10; ++$a['A']; var_dump($a);");
  MVCR("<?php $a['A'] = 10; $a['A'] .= 'test'; var_dump($a);");
  MVCR("<?php $a['A'] = 10; $a['A'] += 25; var_dump($a);");
  MVCR("<?php $a[null] = 10;"
      "var_dump($a[null]);"
      "var_dump($a[\"\"]);"
      "var_dump($a['']);");
  MVCR("<?php "
      "$a = array();"
      "$a[0] = 1;"
      "$a[01] = 2;"
      "$a[007] = 3;"
      "$a[08] = 4;"
      "$a[0xa] = 5;"
      "var_dump($a);"
      "var_dump(\"$a[0]\");"
      "var_dump(\"$a[1]\");"
      "var_dump(\"$a[7]\");"
      "var_dump(\"$a[10]\");"
      "var_dump(\"$a[01]\");"
      "var_dump(\"$a[007]\");"
      "var_dump(\"$a[08]\");"
      "var_dump(\"$a[0xa]\");");

  MVCR("<?php "
       "class A {"
       "  const i1= -1;"
       "  const i2= -2;"
       "  static $s = -4;"
       "};"
       "class B {"
       "  static $s = -5;"
       "};"
       "$attr=array();"
       "$attr[a::i1]='abc';"
       "$attr[a::i2]='def';"
       "$attr[-3]='ghi';"
       "$attr[a::$s]='jkl';"
       "$attr[b::$s]='mno';"
       "var_dump($attr);");

  MVCR("<?php "
       "function foo(&$a) {}"
       "foo($a[array()]);"
       "foo($a[new StdClass]);"
       "var_dump($a);");

 MVCRO("<?php\n"
       "function f(&$elem) {\n"
       "$elem = 44;\n"
       "}\n"
       "$arr = array();\n"
       "$arr[PHP_INT_MAX-1] = 1;\n"
       "$arr[PHP_INT_MAX] = 2;\n"
       "var_dump($arr);\n"
       "f($arr[]);\n"
       "var_dump($arr);\n"
       "unset($arr[PHP_INT_MAX]);\n"
       "unset($arr[PHP_INT_MAX-1]);\n"
       "f($arr[]);\n"
       "var_dump($arr);\n"
       ,
       "array(2) {\n"
       "  [9223372036854775806]=>\n"
       "  int(1)\n"
       "  [9223372036854775807]=>\n"
       "  int(2)\n"
       "}\n"
       "array(2) {\n"
       "  [9223372036854775806]=>\n"
       "  int(1)\n"
       "  [9223372036854775807]=>\n"
       "  int(2)\n"
       "}\n"
       "array(1) {\n"
       "  [9223372036854775807]=>\n"
       "  int(44)\n"
       "}\n");

  return true;
}

bool TestCodeRun::TestArrayAccess() {
  MVCR("<?php\n"
      "class A implements ArrayAccess {"
      "  public $a;"
      "  public function offsetExists($offset) {"
      "    echo \"offsetExist\";"
      "    return false;"
      "  }"
      "  public function offsetGet($offset) {"
      "    echo \"offsetGet\";"
      "    return $this->$offset.'get';"
      "  }"
      "  public function offsetSet($offset, $value) {"
      "    $this->$offset = $value.'set';"
      "  }"
      "  public function offsetUnset($offset) {"
      "    $this->$offset = 'unset';"
      "  }"
      "}"
      "function f() { var_dump('f()'); return 1; }\n"
      "function test($a) {\n"
      "$a['foo'] .= f();\n"
      "$a['bar'] += f();\n"
      "$a['bar'] -= f();\n"
      "$a['bar'] *= f();\n"
      "$a['bar'] /= f();\n"
      "$a['bar'] %= f();\n"
      "$a['bar'] &= f();\n"
      "$a['bar'] |= f();\n"
      "$a['bar'] ^= f();\n"
      "$a['bar'] <<= f();\n"
      "$a['bar'] >>= f();\n"
      "}\n"
      "test(new A);\n"
      );

  MVCR("<?php "
      "class A implements ArrayAccess {"
      "  public $a;"
      "  public function offsetExists($offset) {"
      "    echo \"offsetExist\";"
      "    return false;"
      "  }"
      "  public function offsetGet($offset) {"
      "    echo \"offsetGet\";"
      "    return $this->$offset.'get';"
      "  }"
      "  public function offsetSet($offset, $value) {"
      "    $this->$offset = $value.'set';"
      "  }"
      "  public function offsetUnset($offset) {"
      "    $this->$offset = 'unset';"
      "  }"
      "}"
      "$obj = new A();"
      "if (!isset($obj['a'])) {"
      "  $obj['a'] = 'test';"
      "}"
      "if (!empty($obj['a'])) {"
      "  $obj['a'] = 'test2';"
      "}"
      "var_dump($obj['a']);"
      "unset($obj['a']);"
      "var_dump($obj['a']);"
      );

  MVCR("<?php "
      "function offsetGet($index) {"
      "  echo (\"GET0: $index\\n\");"
      "}"
      "class ArrayAccessImpl implements ArrayAccess {"
      "  public $data = array();"
      "  public function offsetUnset($index) { echo \"UNSET: $index\\n\"; }"
      "  public function offsetSet($index, $value) {"
      "    echo (\"SET: $index\\n\");"
      "    if(isset($data[$index])) {"
      "        unset($data[$index]);"
      "    }"
      "    $u = &$this->data[$index];"
      "    if(is_array($value)) {"
      "        $u = new ArrayAccessImpl();"
      "        foreach($value as $idx=>$e)"
      "            $u[$idx]=$e;"
      "    } else"
      "        $u=$value;"
      "  }"
      "  public function offsetGet($index) {"
      "    echo (\"GET: $index\\n\");"
      "    if(!isset($this->data[$index]))"
      "        $this->data[$index]=new ArrayAccessImpl();"
      "    return $this->data[$index];"
      "  }"
      "  public function offsetExists($index) {"
      "    echo (\"EXISTS: $index\\n\");"
      "    if(isset($this->data[$index])) {"
      "        if($this->data[$index] instanceof ArrayAccessImpl) {"
      "            if(count($this->data[$index]->data)>0)"
      "                return true;"
      "            else"
      "                return false;"
      "        } else"
      "            return true;"
      "    } else"
      "        return false;"
      "  }"
      "}"
      ""
      "class ArrayAccessImpl2 extends ArrayAccessImpl {"
      "  public function offsetUnset($index) { echo \"UNSET2: $index\\n\"; }"
      "  public function offsetSet($index, $value) {"
      "    echo (\"SET2: $index\\n\");"
      "    if(isset($data[$index])) {"
      "        unset($data[$index]);"
      "    }"
      "    $u = &$this->data[$index];"
      "    if(is_array($value)) {"
      "        $u = new ArrayAccessImpl();"
      "        foreach($value as $idx=>$e)"
      "            $u[$idx]=$e;"
      "    } else"
      "        $u=$value;"
      "  }"
      "  public function offsetGet($index) {"
      "    echo (\"GET2: $index\\n\");"
      "    if(!isset($this->data[$index]))"
      "        $this->data[$index]=new ArrayAccessImpl();"
      "    return $this->data[$index];"
      "  }"
      "  public function offsetExists($index) {"
      "    echo (\"EXISTS2: $index\\n\");"
      "    if(isset($this->data[$index])) {"
      "        if($this->data[$index] instanceof ArrayAccessImpl) {"
      "            if(count($this->data[$index]->data)>0)"
      "                return true;"
      "            else"
      "                return false;"
      "        } else"
      "            return true;"
      "    } else"
      "        return false;"
      "  }"
      "}"
      "offsetGet('foo');"
      "$data = new ArrayAccessImpl();"
      "$data['string']=\"Just a simple string\";"
      "$data['number']=33;"
      "$data['array']['another_string']=\"Alpha\";"
      "$data['array']['some_object']=new stdClass();"
      "$data['array']['another_array']['x']['y']=\"Beta\";"
      "$data['blank_array']=array();"
      "print_r(isset($data['array']));"
      "print_r($data['array']['non_existent']);"
      "print_r(isset($data['array']['non_existent']));"
      "print_r($data['blank_array']);"
      "print_r(isset($data['blank_array']));"
      "unset($data['blank_array']);"
      "print_r($data);"
      "$data2 = new ArrayAccessImpl2();"
      "$data2['string']=\"Just a simple string\";"
      "$data2['number']=33;"
      "$data2['array']['another_string']=\"Alpha\";"
      "$data2['array']['some_object']=new stdClass();"
      "$data2['array']['another_array']['x']['y']=\"Beta\";"
      "$data2['blank_array']=array();"
      "print_r(isset($data2['array']));"
      "print_r($data2['array']['non_existent']);"
      "print_r(isset($data2['array']['non_existent']));"
      "print_r($data2['blank_array']);"
      "print_r(isset($data2['blank_array']));"
      "unset($data2['blank_array']);"
      "print_r($data2);");

  MVCR("<?php "
       "class X implements ArrayAccess {"
       "  function offsetGet($f) { return $f; }"
       "  function offsetSet($f, $v) {}"
       "  function offsetUnset($f) {}"
       "  function offsetExists($f) { return false; }"
       "  }"
       "function test() {"
       "  $x = new X;"
       "  unset($x['a']);"
       "  return isset($x['b']);"
       "}"
       "var_dump(test());");

  MVCR("<php "
       "function test($x) {"
       "  $a = $x;"
       "  $b = $a;"
       "  $a[0]->foo = 1;"
       "  var_dump($a, $b);"
       "  $a = $x;"
       "  $b = $a;"
       "  $a[0][1] = 1;"
       "  var_dump($a, $b);"
       "  $a = $x;"
       "  $c = &$a[0];"
       "  $b = $a;"
       "  $a[0][1] = 1;"
       "  var_dump($a, $b);"
       "  }"
       "test(array(false));"
       "var_dump(array(false));");

  return true;
}

bool TestCodeRun::TestArrayIterator() {
  MVCR("<?php "
      "class MyIterator implements Iterator"
      "{"
      "  private $var = array();"
      "  public function __construct($array)"
      "  {"
      "    echo \"constructing\n\";"
      "    if (is_array($array)) {"
      "        $this->var = $array;"
      "    }"
      "  }"
      "  public function rewind() {"
      "    echo \"rewinding\n\";"
      "    reset($this->var);"
      "  }"
      "  public function current() {"
      "    $var = current($this->var);"
      "    echo \"current: $var\n\";"
      "    return $var;"
      "  }"
      "  public function key() {"
      "    $var = key($this->var);"
      "    echo \"key: $var\n\";"
      "    return $var;"
      "  }"
      "  public function next() {"
      "    $var = next($this->var);"
      "    echo \"next: $var\n\";"
      "    return $var;"
      "  }"
      "  public function valid() {"
      "    $var = $this->current() !== false;"
      "    echo \"valid: {$var}\n\";"
      "    return $var;"
      "  }"
      "}"
      "$values = array(1,2,3);"
      "$it = new MyIterator($values);"
      "foreach ($it as $a => $b) {"
      "  print \"$a: $b\n\";"
      "}"
      "$itp = \"it\";"
      "foreach ($$itp as $a => $b) {"
      "  print \"$a: $b\n\";"
      "}"
      "function getIter() {"
      "  $values = array(1,2,3);"
      "  $it = new MyIterator($values);"
      "  return $it;"
      "}"
      "foreach (getIter() as $a => $b) {"
      "  print \"$a: $b\n\";"
      "}"
      );
  MVCR("<?php\n"
      "$a = array(1, 2, 3, 4, 5, 6);\n"
      "while ($v = each($a)) { if ($v[1] < 4) $a[] = $v[1] + $v[1]; }\n"
      "var_dump($a);\n"
      "$a = array(1, 2, 3, 4, 5, 6);\n"
      "foreach ($a as $k => $v) { if ($v >= 4) $a = $v + $v; }\n"
      "var_dump($a);\n");
  MVCR("<?php\n"
      "$arr = array('bar', 'bar', 'bar', 'bar', 'bar', 'bar', 'foo');\n"
      "function foo() {\n"
      "  var_dump(__FUNCTION__); global $arr; $arr[] = 'bar';\n"
      "}\n"
      "function bar() { var_dump(__FUNCTION__); }\n"
      "reset($arr);\n"
      "while ($func = each($arr)) { $f = $func[1]; $f(); }\n");
  MVCR("<?php "
       "function test($a) {"
       "  $it = new ArrayIterator($a);"
       "  while ($it->valid()) {"
       "    var_dump($it->key());"
       "    var_dump($it->current());"
       "    $it->next();"
       "  }"
       "}"
       "test(array('a' => 'x',"
       "           false => 'y',"
       "           '1' => false,"
       "           null => 'z',"
       "           'c' => 'w'));");
  MVCR("<?php\n"
       "$a = array(1, 2, 3);\n"
       "$o = new ArrayIterator($a);\n"
       "var_dump($o->next());\n"
       "var_dump($o->rewind());\n"
       "var_dump($o->seek());\n"
       "var_dump($o->asort());\n"
       "var_dump($o->ksort());\n"
       "var_dump($o->natsort());\n"
       "var_dump($o->natcasesort());\n");

  // MutableArrayIterator
  MVCRO("<?php\n"
        "$a = array(1, 2, 3);\n"
        "foreach ($a as $k1 => &$v1) { $v1 += $k1; }\n"
        "var_dump($a);\n"
        "$a = array(1, 2, 3);\n"
        "for ($o = new MutableArrayIterator($a); $o->valid(); $o->next()) {\n"
        "  $k2 = $o->key();\n"
        "  $v2 = &$o->currentRef();\n"
        "  $v2 += $k2;\n"
        "}\n"
        "var_dump($a);\n",

        "array(3) {\n"
        "  [0]=>\n"
        "  int(1)\n"
        "  [1]=>\n"
        "  int(3)\n"
        "  [2]=>\n"
        "  &int(5)\n"
        "}\n"
        "array(3) {\n"
        "  [0]=>\n"
        "  int(1)\n"
        "  [1]=>\n"
        "  int(3)\n"
        "  [2]=>\n"
        "  &int(5)\n"
        "}\n");

  return true;
}

bool TestCodeRun::TestArrayForEach() {
  MVCR("<?php\n"
       "function f1() {\n"
       "  $i = 0;\n"
       "  $foo = array(1,2,3,4);\n"
       "  foreach ($foo as $key => &$val) {\n"
       "    echo \"key=$key val=$val\\n\";\n"
       "    if($val == 2) {\n"
       "      $foo[$key] = 0;\n"
       "    } else if($val == 3) {\n"
       "      unset($foo[$key]);\n"
       "    } else {\n"
       "      $val++;\n"
       "    }\n"
       "    ++$i;\n"
       "    if ($i >= 20)\n"
       "      break;\n"
       "  }\n"
       "  var_dump($foo);\n"
       "}\n"
       "f1();\n");

  MVCR("<?php\n"
       "function f2() {\n"
       "  $i = 0;\n"
       "  $foo = array(1,2,3,4);\n"
       "  foreach ($foo as $key => &$val) {\n"
       "    echo \"key=$key val=$val\\n\";\n"
       "    if($val == 2) {\n"
       "      $foo[$key] = 0;\n"
       "    } else if($val == 3) {\n"
       "      $foo['a'] = 7;\n"
       "    } else {\n"
       "      $val++;\n"
       "    }\n"
       "    ++$i;\n"
       "    if ($i >= 20)\n"
       "      break;\n"
       "  }\n"
       "  var_dump($foo);\n"
       "}\n"
       "f2();\n");

  /**
   * Zend PHP 5.2 outputs:
   *   key=a val=1
   *   key=b val=2
   *   key=c val=333
   *   array(6) {
   *     ["a"]=>
   *     int(1)
   *     ["b"]=>
   *     int(2)
   *     ["d"]=>
   *     int(4)
   *     ["e"]=>
   *     int(5)
   *     ["f"]=>
   *     int(6)
   *     ["c"]=>
   *     &int(333)
   *   }
   *
   * The difference in behavior is intentional. Under PHP, when the next
   * element is unset inside a foreach by reference loop, a heuristic is used
   * to figure out which element should be visited next. In this specific
   * example, the loop resumes at key 'c', skipping over keys 'd', 'e', and
   * 'f'.
   *
   * Under HPHP, when the next element is unset inside a foreach by loop, the
   * loop's iterator is appropriately updated. HPHP successfully upholds the
   * invariant that a foreach by refererence loop that exhausts the array will
   * visit every element that has not been deleted exactly once.
   */
  MVCRO("<?php\n"
       "function f3() {\n"
       "  $i = 0;\n"
       "  $foo = array('a'=>1, 'b'=>2, 'c'=>3, 'd'=>4, 'e'=>5, 'f'=>6);\n"
       "  $bar = array();\n"
       "  $a = 0;\n"
       "  foreach ($foo as $key => &$val) {\n"
       "    echo \"key=$key val=$val\\n\";\n"
       "    if ($key == 'b' && $a == 0) {\n"
       "      $a = 1;\n"
       "      unset($foo['c']);\n"
       "      $foo['c'] = 333;\n"
       "    }\n"
       "    ++$i;\n"
       "    if ($i >= 20)\n"
       "      break;\n"
       "  }\n"
       "  var_dump($foo);\n"
       "}\n"
       "f3();\n"
       ,
       "key=a val=1\n"
       "key=b val=2\n"
       "key=d val=4\n"
       "key=e val=5\n"
       "key=f val=6\n"
       "key=c val=333\n"
       "array(6) {\n"
       "  [\"a\"]=>\n"
       "  int(1)\n"
       "  [\"b\"]=>\n"
       "  int(2)\n"
       "  [\"d\"]=>\n"
       "  int(4)\n"
       "  [\"e\"]=>\n"
       "  int(5)\n"
       "  [\"f\"]=>\n"
       "  int(6)\n"
       "  [\"c\"]=>\n"
       "  &int(333)\n"
       "}\n"
       );

  MVCR("<?php\n"
       "function f4() {\n"
       "  $i = 0;\n"
       "  $foo = array('a'=>1, 'b'=>2, 'c'=>3, 'd'=>4, 'e'=>5, 'f'=>6);\n"
       "  $bar = array();\n"
       "  $a = 0;\n"
       "  foreach ($foo as $key => &$val) {\n"
       "    echo \"key=$key val=$val\\n\";\n"
       "    if ($key == 'b' && $a == 0) {\n"
       "      $a = 1;\n"
       "      unset($foo['c']);\n"
       "      $bar['b'] = 888;\n"
       "      $foo['c'] = 333;\n"
       "    }\n"
       "    ++$i;\n"
       "    if ($i >= 20)\n"
       "      break;\n"
       "  }\n"
       "  var_dump($foo);\n"
       "}\n"
       "f4();\n");

  /**
   * Zend PHP 5.2 outputs:
   *   key=f val=3
   *   key()=e current()=1
   *   key=e val=1
   *   key()=d current()=5
   *   key=d val=9
   *   key()=0s0 current()=0
   *   key=0s0 val=0
   *   key()=1s1 current()=1
   *   key=1s1 val=1
   *   key()=2s2 current()=2
   *   key=2s2 val=2
   *   key()=3s3 current()=3
   *   key=3s3 val=3
   *   key()=4s4 current()=4
   *   ...
   *
   * The difference in behavior is intentional. For more info, see testcase h3.
   */
  MVCRO("<?php\n"
       "function f5() {\n"
       "  $i = 0;\n"
       "  $foo = array('f'=>3, 'e'=>1, 'd'=>5, 'a'=>6, 'b'=>2, 'c'=>4);\n"
       "  $a = 0;\n"
       "  foreach ($foo as $key => &$val) {\n"
       "    echo \"key=$key val=$val\\n\";\n"
       "    if ($key == 'e' && $a == 0) {\n"
       "      $a = 1;\n"
       "      unset($foo['e']);\n"
       "      unset($foo['d']);\n"
       "      $foo['d'] = 9;\n"
       "      for ($j = 0; $j < 10000; ++$j)\n"
       "        $foo[$j . 's' . $j] = $j;\n"
       "    }\n"
       "    ++$i;\n"
       "    if ($i >= 20)\n"
       "      break;\n"
       "  }\n"
       "}\n"
       "f5();\n"
       ,
       "key=f val=3\n"
       "key=e val=1\n"
       "key=a val=6\n"
       "key=b val=2\n"
       "key=c val=4\n"
       "key=d val=9\n"
       "key=0s0 val=0\n"
       "key=1s1 val=1\n"
       "key=2s2 val=2\n"
       "key=3s3 val=3\n"
       "key=4s4 val=4\n"
       "key=5s5 val=5\n"
       "key=6s6 val=6\n"
       "key=7s7 val=7\n"
       "key=8s8 val=8\n"
       "key=9s9 val=9\n"
       "key=10s10 val=10\n"
       "key=11s11 val=11\n"
       "key=12s12 val=12\n"
       "key=13s13 val=13\n"
       );

  MVCR("<?php\n"
       "function f6() {\n"
       "  $i = 0;\n"
       "  $foo = array('f'=>3, 'e'=>1, 'd'=>5, 'a'=>6, 'b'=>2, 'c'=>4);\n"
       "  $a = 0;\n"
       "  foreach ($foo as $key => &$val) {\n"
       "    if ($key == 'e' && $a == 0) {\n"
       "      $a = 1;\n"
       "      unset($foo['e']);\n"
       "      unset($foo['d']);\n"
       "      $bar['e'] = 8;\n"
       "      $foo['d'] = 9;\n"
       "      for ($j = 0; $j < 10000; ++$j)\n"
       "        $foo[$j . 's' . $j] = $j;\n"
       "    }\n"
       "    ++$i;\n"
       "    if ($i >= 20)\n"
       "      break;\n"
       "  }\n"
       "}\n"
       "f6();\n");

  /**
   * Zend PHP 5.2 outputs:
   *   key=0 value=0
   *   key=1 value=1
   *   key=2 value=0
   *   key=3 value=1
   *
   * The difference in behavior is intentional. Under PHP, a foreach by
   * reference loop will not visit an element that is appended to the array
   * during the iteration for the last element in the array.
   *
   * Under HPHP, a foreach by reference loop will always visit an element that
   * is appended to the array during any iteration, provided that the element
   * is not deleted and the loop does not exit early.
   */
  MVCRO("<?php\n"
       "function f7() {\n"
       "  $i = 0;\n"
       "  $bar = array();\n"
       "  $arr = array(0,1,0,1);\n"
       "  foreach ($arr as $k => &$v) {\n"
       "    echo \"key=$k value=$v\\n\";\n"
       "    if ($k == 0)\n"
       "      $val = 1;\n"
       "    else\n"
       "      $val = $arr[$k-1];\n"
       "    $arr[$k+1] = $val;\n"
       "    ++$i;\n"
       "    if ($i >= 20)\n"
       "      break;\n"
       "  }\n"
       "}\n"
       "f7();\n"
       ,
       "key=0 value=0\n"
       "key=1 value=1\n"
       "key=2 value=0\n"
       "key=3 value=1\n"
       "key=4 value=0\n"
       "key=5 value=1\n"
       "key=6 value=0\n"
       "key=7 value=1\n"
       "key=8 value=0\n"
       "key=9 value=1\n"
       "key=10 value=0\n"
       "key=11 value=1\n"
       "key=12 value=0\n"
       "key=13 value=1\n"
       "key=14 value=0\n"
       "key=15 value=1\n"
       "key=16 value=0\n"
       "key=17 value=1\n"
       "key=18 value=0\n"
       "key=19 value=1\n"
       );

  MVCR("<?php\n"
       "function f8() {\n"
       "  $i = 0;\n"
       "  $bar = array();\n"
       "  $arr = array(0,1,0,1);\n"
       "  foreach ($arr as $k => &$v) {\n"
       "    echo \"key=$k value=$v\\n\";\n"
       "    if ($k == 0)\n"
       "      $val = 1;\n"
       "    else\n"
       "      $val = $arr[$k-1];\n"
       "    unset($arr[$k+1]);\n"
       "    $bar[] = 0;\n"
       "    $arr[$k+1] = $val;\n"
       "    ++$i;\n"
       "    if ($i >= 20)\n"
       "      break;\n"
       "  }\n"
       "}\n"
       "f8();\n");

  MVCR("<?php\n"
       "function f9() {\n"
       "  $i = 0;\n"
       "  $arr = array(1,1,1);\n"
       "  $bar = array();\n"
       "  $first = true;\n"
       "  foreach ($arr as $k => &$v) {\n"
       "    echo \"k=$k v=$v\\n\";\n"
       "    if (!$first) {\n"
       "      $prev_k = ($k+2)%3;\n"
       "      unset($arr[$prev_k]);\n"
       "      if (count($bar) > 100)\n"
       "        $bar = array();\n"
       "      $bar[] = 1;\n"
       "      $arr[$prev_k] = 1;\n"
       "    }\n"
       "    $first = false;\n"
       "    ++$i;\n"
       "    if ($i >= 20)\n"
       "      break;\n"
       "  }\n"
       "}\n"
       "f9();\n");

  /**
   * XXX Note that this test clobbers the array inside the foreach by reference
   * loop. When UseSmallArray=true, this causes a fatal error to be thrown
   * saying "SmallArray should have been escalated". We may need to change
   * MutableArrayIter to check if the array needs to be escalated at the
   * beginning of each iteration.
   */
  MVCR("<?php function g1() {\n"
       "  $arr = array(0,1,2,3);\n"
       "  $b = true;\n"
       "  foreach ($arr as &$v) {\n"
       "    echo \"val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      $arr = array(4,5,6,7);\n"
       "     }\n"
       "  }\n"
       "}\n"
       "g1();\n");

  MVCR("<?php function g2() {\n"
       "  $arr = array(0,1,2,3);\n"
       "  $b = true;\n"
       "  foreach ($arr as &$v) {\n"
       "    echo \"val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      $old = $arr;\n"
       "      $arr = array(4,5,6,7);\n"
       "    } else if ($v == 6) {\n"
       "      $arr = $old;\n"
       "      unset($old);\n"
       "    }\n"
       "  }\n"
       "}\n"
       "g2();\n");

  MVCR("<?php function g3() {\n"
       "  $arr2 = array(0,1,2,3);\n"
       "  $arr = $arr2;\n"
       "  $b = true;\n"
       "  $b2 = true;\n"
       "  foreach ($arr as &$v) {\n"
       "    echo \"val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      $arr = array(4,5,6,7);\n"
       "    } else if ($b2 && $v == 6) {\n"
       "      $b2 = false;\n"
       "      $arr = $arr2;\n"
       "    }\n"
       "  }\n"
       "}\n"
       "g3();\n");

  MVCR("<?php function g4() {\n"
       "  $arr = array(0,1,2,3);\n"
       "  $b = true;\n"
       "  foreach ($arr as &$v) {\n"
       "    echo \"val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      array_push($arr, 4);\n"
       "    }\n"
       "  }\n"
       "}\n"
       "g4();\n");

  MVCR("<?php function g5() {\n"
       "  $arr = array(0,1,2,3);\n"
       "  $arr2 = $arr;\n"
       "  $b = true;\n"
       "  foreach ($arr as &$v) {\n"
       "    echo \"val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      array_push($arr, 4);\n"
       "    }\n"
       "  }\n"
       "}\n"
       "g5();\n");

  MVCR("<?php function g6() {\n"
       "  $arr = array(0,'a'=>1,2,'b'=>3,4);\n"
       "  $b = true;\n"
       "  foreach ($arr as $k => &$v) {\n"
       "    echo \"key=$k val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      array_pop($arr);\n"
       "    }\n"
       "  }\n"
       "}\n"
       "g6();\n");

  MVCR("<?php function g7() {\n"
       "  $arr = array(0,'a'=>1,2,'b'=>3,4);\n"
       "  $b = true;\n"
       "  foreach ($arr as $k => &$v) {\n"
       "    echo \"key=$k val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      unset($arr[1]); \n"
       "    }\n"
       "  }\n"
       "}\n"
       "g7();\n");

  /**
   * Zend PHP 5.2 outputs:
   *   key=0 val=0
   *   key=a val=1
   *   key=0 val=0
   *   key=a val=1
   *   key=b val=3
   *
   * The difference in behavior is intentional. Under PHP, after the next
   * element is unset inside the foreach by reference loop and the array_pop
   * operation is performed, a heuristic is used to determine which element
   * should be visited next. If this specific example, the loop chooses to
   * resume at key '0'.
   *
   * Under HPHP, when the next element is unset inside a foreach by loop, the
   * loop's iterator is appropriately updated. Likewise, the loop's iterator
   * remains intact after the array_pop operation. Thus, after the unset and
   * the pop operation, HPHP resumes the loop at key 'b'.
   */
  MVCRO("<?php function g8() {\n"
       "  $arr = array(0,'a'=>1,2,'b'=>3,4);\n"
       "  $b = true;\n"
       "  foreach ($arr as $k => &$v) {\n"
       "    echo \"key=$k val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      unset($arr[1]); \n"
       "      array_pop($arr);\n"
       "    }\n"
       "  }\n"
       "}\n"
       "g8();\n"
       ,
       "key=0 val=0\n"
       "key=a val=1\n"
       "key=b val=3\n"
       );

  MVCR("<?php function g9() {\n"
       "  $arr = array(0,1,2,3,4);\n"
       "  $b = true;\n"
       "  foreach ($arr as &$v) {\n"
       "    echo \"val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      array_shift($arr);\n"
       "    }\n"
       "  }\n"
       "}\n"
       "g9();\n");

  MVCR("<?php function g10() {\n"
       "  $arr = array(0,1,2,3);\n"
       "  $b = true;\n"
       "  foreach ($arr as &$v) {\n"
       "    echo \"val=$v\\n\";\n"
       "    if ($b && $v == 1) {\n"
       "      $b = false;\n"
       "      array_unshift($arr, 4);\n"
       "    }\n"
       "  }\n"
       "}\n"
       "g10();\n");

  MVCR("<?php function g11() {\n"
       "  $arr = array(0,1,2,3);\n"
       "  reset($arr);\n"
       "  var_dump(current($arr));\n"
       "  foreach ($arr as &$v) {\n"
       "    var_dump(current($arr));\n"
       "  }\n"
       "  var_dump(current($arr));\n"
       "}\n"
       "g11();\n");

  MVCR("<?php\n"
       "function h1(&$arr, $i) {\n"
       "  foreach ($arr as $k => &$v) {\n"
       "    echo \"i=$i key=$k\\n\";\n"
       "    if ($k == 0) {\n"
       "      if ($i > 0) {\n"
       "        h1($arr, $i-1);\n"
       "      } else if ($i == 0) {\n"
       "        echo \"Unsetting key 1\\n\";\n"
       "        unset($arr[1]);\n"
       "      }\n"
       "    }\n"
       "  }\n"
       "  end($arr);\n"
       "}\n"
       "$arr = array('a','b','c');\n"
       "h1($arr, 40);\n"
       "var_dump($arr);\n");

  /**
   * Zend PHP 5.2 outputs:
   *   val=0
   *   val=1
   *   val=2
   *   val=3
   *   val=4
   *   bool(false)
   *
   * The difference in behavior is intentional. Under PHP, foreach by value can
   * in some cases modify the array's internal iterator without triggering an
   * array copy. This can potentially expose information to user code about
   * when array copies are triggered.
   *
   * Under HPHP, foreach by value will never modify the array's internal
   * iterator. The advantage here is that we do not expose information about
   * when array copies are triggered to user code.
   *
   * The PHP manual states the following:
   *   foreach has some side effects on the array pointer. Don't rely on the
   *   array pointer during or after the foreach without resetting it.
   */
  MVCRO("<?php function k1() {\n"
       "  $arr = array(0,1,2,3,4);\n"
       "  reset($arr);\n"
       "  foreach ($arr as $v) {\n"
       "    echo \"val=$v\\n\";\n"
       "  }\n"
       "  var_dump(current($arr));\n"
       "}\n"
       "k1();\n"
       ,
       "val=0\n"
       "val=1\n"
       "val=2\n"
       "val=3\n"
       "val=4\n"
       "int(0)\n"
       );

  MVCR("<?php function k2() {\n"
       "  $arr = array(0,1,2,3,4);\n"
       "  reset($arr);\n"
       "  $arr2 = $arr;\n"
       "  foreach ($arr as $v) {\n"
       "    echo \"val=$v\\n\";\n"
       "  }\n"
       "  var_dump(current($arr));\n"
       "  var_dump(current($arr2));\n"
       "}\n"
       "k2();\n");

  /**
   * Zend PHP 5.2 outputs:
   *   val=0
   *   val=1
   *   val=2
   *   val=3
   *   val=4
   *   int(0)
   *   bool(false)
   *
   * The difference in behavior is intentional. For more info see testcase k1.
   */
  MVCRO("<?php function k3() {\n"
       "  $arr = array(0,1,2,3,4);\n"
       "  reset($arr);\n"
       "  $b = true;\n"
       "  foreach ($arr as $v) {\n"
       "    if ($b) {\n"
       "      $b = false;\n"
       "      $arr2 = $arr;\n"
       "    }\n"
       "    echo \"val=$v\\n\";\n"
       "  }\n"
       "  var_dump(current($arr));\n"
       "  var_dump(current($arr2));\n"
       "}\n"
       "k3();\n"
       ,
       "val=0\n"
       "val=1\n"
       "val=2\n"
       "val=3\n"
       "val=4\n"
       "int(0)\n"
       "int(0)\n"
       );

  /**
   * Zend PHP 5.2 outputs:
   *   val=0
   *   val=1
   *   val=2
   *   val=3
   *   val=4
   *   int(0)
   *   bool(false)
   *
   * This behavior is intentional. For more info see testcase k1.
   */
  MVCRO("<?php function k4() {\n"
       "  $arr = array(0,1,2,3,4);\n"
       "  reset($arr);\n"
       "  $b = true;\n"
       "  foreach ($arr as $v) {\n"
       "    if ($b) {\n"
       "      $b = false;\n"
       "      $arr2 = $arr;\n"
       "    }\n"
       "    echo \"val=$v\\n\";\n"
       "  }\n"
       "  var_dump(current($arr2));\n"
       "  var_dump(current($arr));\n"
       "}\n"
       "k4();\n"
       ,
       "val=0\n"
       "val=1\n"
       "val=2\n"
       "val=3\n"
       "val=4\n"
       "int(0)\n"
       "int(0)\n"
       );

  MVCR("<?php "
       "$a = array(1,2,3,4,5);"
       "function foo($a, $b, $c) {"
       "  var_dump('foo');"
       "  return 1;"
       "}"
       "function bar($a, $b, $c) {"
       "  var_dump('bar');"
       "  return 2;"
       "}"
       "function buz($x,$y) { if ($y) return $x; return 1; }"
       "$s = buz('hello',1);"
       "foreach ($a as $s[3]) {"
       "  var_dump($s);"
       "}"
       "$i = 0;"
       "foreach ($a as "
       "         $a[bar($i++, $i++, $i++)] => &$a[foo($i++, $i++, $i++)]) {"
       "  var_dump($a[1],$a[2]);"
       "}"
       "foreach ($a as "
       "         $a[bar($i++, $i++, $i++)] => $a[foo($i++, $i++, $i++)]) {"
       "  var_dump($a[1],$a[2]);"
       "}");


  return true;
}

bool TestCodeRun::TestArrayAssignment() {
  MVCR("<?php "
      "$a = array(1, 2, 3);"
      "$b = $a;"
      "$b[4] = 4;"
      "var_dump($a);"
      "var_dump($b);"
      "$b = 3;"
      "var_dump($a);"
      "var_dump($b);");
  MVCR("<?php "
      "$a = array('1', '2', '3');"
      "$b = $a;"
      "$b[4] = '4';"
      "var_dump($a);"
      "var_dump($b);"
      "$b = '3';"
      "var_dump($a);"
      "var_dump($b);");
  MVCR("<?php "
      "$a = array(1.5, 2.5, 3.5);"
      "$b = $a;"
      "$b[4] = 4.5;"
      "var_dump($a);"
      "var_dump($b);"
      "$b = 3.5;"
      "var_dump($a);"
      "var_dump($b);");
  MVCR("<?php "
      "$a = array(1, 'hello', 3.5);"
      "$b = $a;"
      "$b[4] = 'world';"
      "var_dump($a);"
      "var_dump($b);");
  MVCR("<?php "
      "$a = array(1, 'hello', 3.5);"
      "$b = $a;"
      "$b[4] = 'world';"
      "var_dump($a);"
      "var_dump($b);"
      "$b = 3;"
      "var_dump($a);"
      "var_dump($b);");
  MVCR("<?php "
      "$a = array('a' => '1', 2 => 2, 'c' => '3');"
      "var_dump($a);"
      "$a = array('a' => '1', 2 => 2, 'c' => '3',"
      "           'd' => array('a' => '1', 2 => 2, 'c' => '3'));"
      "var_dump($a);");
  MVCR("<?php "
      "$a = array(1=>'main', 2=>'sub');"
      "$b = $a;"
      "var_dump(array_pop($b));"
      "print_r($a);"
      "var_dump(array_shift($b));"
      "print_r($a);");
  MVCR("<?php "
      "$a = array(1, 2, 3);"
      "var_dump($a);"
      "array_pop($a);"
      "var_dump($a);"
      "array_shift($a);"
      "var_dump($a);");
  MVCR("<?php "
      "function foo() {"
      "  $p = 1;"
      "  $q = 2;"
      "  $r = 3;"
      "  $s = 4;"
      "  $a = array('1'=>$p, '2'=>&$q);"
      "  $b = array('3'=>$r, '4'=>&$s);"
      "  var_dump($a);"
      "  $a += $b;"
      "  var_dump($a);"
      "  var_dump($b);"
      "}"
      "foo();");

  MVCR("<?php $a = false; $a['a'] = 10;");
  MVCR("<?php $a = false; $a['a']['b'] = 10;");

  // invalid offset type
  MVCR("<?php\n"
       "class c { function f(&$a, $b) { $a = $b; } }\n"
       "function setNullVariantHelper($f, $value) {\n"
       "  $a = array();\n"
       "  $obj = new c;\n"
       "  $obj->$f($a[$obj] = 1, $value);\n"
       "  var_dump($a[$obj] = 1);\n"
       "}\n"
       "function setNullVariant($value) {\n"
       "  setNullVariantHelper('f', $value);\n"
       "}\n"
       "setNullVariant('Surprise!');\n"
       "$b = null;\n"
       "var_dump($b[1]);\n");

  // empty string to array conversion
  MVCR("<?php\n"
       "$s = '';\n"
       "$s[0] += 10;\n"
       "var_dump($s);\n");

  return true;
}

bool TestCodeRun::TestArrayFunctions() {
  MVCR("<?php "
       "class A implements Countable { public function count() { return 1;}} "
       "$obj = new A(); var_dump(count($obj));");

  MVCR("<?php\n"
       "$a = array(\n"
       "  array('a' => 'a'),\n"
       "  array('b' => 'bb'),\n"
       "  array('c' => 'cc'),\n"
       ");\n"
       "\n"
       "$refs = array();\n"
       "foreach ($a as &$arr) {\n"
       "  $refs[] = &$arr;\n"
       "}\n"
       "array_splice($a, 1, 1);\n"
       "var_dump($a);\n"
      );

  MVCR("<?php\n"
       "$x = array('x' => 'y');\n"
       "$a = array('a1' => $x, 'a2' => $x);\n"
       "$b = array('a1' => array(1,2,3), 'a2' => array(1,2,3));\n"
       "var_dump(array_merge_recursive($a, $b));");
  MVCR("<?php\n"
       "$x = array('x' => 'y');\n"
       "$a = array('a1' => &$x, 'a2' => &$x);\n"
       "$b = array('a1' => array(), 'a2' => array(1,2));\n"
       "var_dump(array_merge_recursive($a, $b));");
  MVCR("<?php\n"
       "$x = array('x' => 'y');\n"
       "$a = array('a1' => &$x, 'a2' => &$x);\n"
       "$b = array('a1' => array(1,2), 'a2' => array(3,4));\n"
       "var_dump(array_merge_recursive($a, $b));");
  MVCR("<?php $a = array(1 => 1, 3 => 3); "
      "var_dump(array_merge($a, array(2)));");
  MVCR("<?php $a = array(1 => 1, 3 => 3); "
      "var_dump(array_merge($a, array()));");
  MVCR("<?php $a = array('a' => 1, 3 => 3); "
      "var_dump(array_merge($a, array(2)));");
  MVCR("<?php $a = array('a' => 1, 'b' => 3); "
      "var_dump(array_merge($a, array(2)));");
  MVCR("<?php $a = array('a' => 1, 3 => 3); "
      "var_dump(array_merge($a, array('a' => 2)));");
  MVCR("<?php $a = array('a' => 1, 3 => 3); "
      "var_dump(array_merge($a, array('b' => 2)));");
  MVCR("<?php $a = array('a' => 1, 'b' => 3); "
      "var_dump(array_merge($a, array('c' => 2)));");

  MVCR("<?php "
      "var_dump(array_unique(array(array(1,2), array(1,2), array(3,4),)));");
  MVCR("<?php "
      "$input = array(\"a\" => \"green\","
      "               \"red\", \"b\" => \"green\", \"blue\", \"red\");"
      "$result = array_unique($input);"
      "print_r($result);");
  MVCR("<?php "
      "$input = array(4, \"4\", \"3\", 4, 3, \"3\");"
      "$result = array_unique($input);"
      "var_dump($result);");

  // Preservation of keys and references in original arrays.
  MVCR("<?php\n"
       "class A { }\n"
       "$o = new A;\n"
       "$f = '10';\n"
       "$o->$f = 100;\n"
       "$a = (array)$o;\n"
       "$v = 1;\n"
       "$a[10] = &$v;\n"
       "$a[11] = array(&$v);\n"
       "var_dump($a);\n"
       // array_diff
       "$b = array(10 => 10);\n"
       "var_dump(array_diff_key($a, $b));\n"
       // array_merge and array_merge_recursive
       "var_dump(array_merge($a, $b));\n"
       "var_dump(array_merge_recursive($a, $b));\n"
       // array_reverse
       "var_dump(array_reverse($a));\n"
       // array_chunk
       "var_dump(array_chunk($a, 2));\n"
       );

  // Test array_reduce
  MVCRO("<?php\n"
        "function f($x, $y) {\n"
        "  var_dump($x, $y);\n"
        "  return $x + $x + $y + 1;\n"
        "}\n"
        "var_dump(array_reduce(array(), 'f'));\n"
        "var_dump(array_reduce(array(), 'f', null));\n"
        "var_dump(array_reduce(array(), 'f', 0));\n"
        "var_dump(array_reduce(array(), 'f', 23));\n"
        "var_dump(array_reduce(array(4), 'f'));\n"
        "var_dump(array_reduce(array(4), 'f', null));\n"
        "var_dump(array_reduce(array(4), 'f', 0));\n"
        "var_dump(array_reduce(array(4), 'f', 23));\n"
        "var_dump(array_reduce(array(1,2,3,4), 'f'));\n"
        "var_dump(array_reduce(array(1,2,3,4), 'f', null));\n"
        "var_dump(array_reduce(array(1,2,3,4), 'f', 0));\n"
        "var_dump(array_reduce(array(1,2,3,4), 'f', 23));  \n"
        ,
        "NULL\n"
        "NULL\n"
        "int(0)\n"
        "int(23)\n"
        "NULL\n"
        "int(4)\n"
        "int(5)\n"
        "NULL\n"
        "int(4)\n"
        "int(5)\n"
        "int(0)\n"
        "int(4)\n"
        "int(5)\n"
        "int(23)\n"
        "int(4)\n"
        "int(51)\n"
        "NULL\n"
        "int(1)\n"
        "int(2)\n"
        "int(2)\n"
        "int(7)\n"
        "int(3)\n"
        "int(18)\n"
        "int(4)\n"
        "int(41)\n"
        "NULL\n"
        "int(1)\n"
        "int(2)\n"
        "int(2)\n"
        "int(7)\n"
        "int(3)\n"
        "int(18)\n"
        "int(4)\n"
        "int(41)\n"
        "int(0)\n"
        "int(1)\n"
        "int(2)\n"
        "int(2)\n"
        "int(7)\n"
        "int(3)\n"
        "int(18)\n"
        "int(4)\n"
        "int(41)\n"
        "int(23)\n"
        "int(1)\n"
        "int(48)\n"
        "int(2)\n"
        "int(99)\n"
        "int(3)\n"
        "int(202)\n"
        "int(4)\n"
        "int(409)\n"
        );

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestVariant() {
  MVCR("<?php $a = 1; $a = 'test'; print $a;");
  MVCR("<?php $a = 1; $a = 'test'; $a .= 'b'; print $a;");

  MVCR("<?php $a = array(); $a[] = 3; $a = 'test'; var_dump($a);");
  MVCR("<?php $a = array(); $a['test'] = 3; var_dump($a);");
  MVCR("<?php $a['test'] = 3; var_dump($a);");

  MVCR("<?php $a = 1; $a = 'test'; print $a{0};");
  MVCR("<?php $a=1;$a='t'; $a[0]  = 'AB'; var_dump($a);");
  MVCR("<?php $a=1;$a='';  $a[0]  = 'AB'; var_dump($a);");
  MVCR("<?php $a=1;$a='t'; $a[10] = 'AB'; var_dump($a);");
  MVCR("<?php $a=1;$a='t'; $b = $a; $a[0] = 'AB'; var_dump($a); var_dump($b);");
  MVCR("<?php $a=1;$a='t'; $b = $a; $a[10]= 'AB'; var_dump($a); var_dump($b);");
  MVCR("<?php $a=1;$a='t'; $b = $a; $b[0] = 'AB'; var_dump($a); var_dump($b);");
  MVCR("<?php $a=1;$a='t'; $b = $a; $b[10]= 'AB'; var_dump($a); var_dump($b);");

  MVCR("<?php $a = 't'; $a = 1; print $a + 2;");
  MVCR("<?php $a = 't'; $a = 1; print 2 + $a;");
  MVCR("<?php $a = 't'; $a = 1; $b = 'a'; $b = 2; print $a + $b;");
  MVCR("<?php $a = 't'; $a = 1; $a += 2; print $a;");
  MVCR("<?php $a = 't'; $a = 1; $a += 'n'; print $a;");
  MVCR("<?php $a = 't'; $a = 1; $a += '5'; print $a;");
  MVCR("<?php $b = 'test'; $b = 1; $a += $b; print $a;");

  MVCR("<?php $a = 't'; $a = 1; print -$a;");
  MVCR("<?php $a = 't'; $a = -$a; print $a;");
  MVCR("<?php $a = 't'; $a = 1; print $a - 2;");
  MVCR("<?php $a = 't'; $a = 1; print 2 - $a;");
  MVCR("<?php $a = 't'; $a = 1; $b = 'a'; $b = 2; print $a - $b;");
  MVCR("<?php $a = 't'; $a = 1; $a -= 2; print $a;");
  MVCR("<?php $a = 't'; $a = 1; $a -= 'n'; print $a;");
  MVCR("<?php $a = 't'; $a = 1; $a -= '5'; print $a;");

  MVCR("<?php $a = 't'; $a = 10; print $a * 2;");
  MVCR("<?php $a = 't'; $a = 10; print 2 * $a;");
  MVCR("<?php $a = 't'; $a = 10; $b = 'a'; $b = 2; print $a * $b;");
  MVCR("<?php $a = 't'; $a = 10; $a *= 2; print $a;");
  MVCR("<?php $a = 't'; $a = 10; $a *= 'n'; print $a;");
  MVCR("<?php $a = 't'; $a = 10; $a *= '5'; print $a;");

  MVCR("<?php $a = 't'; $a = 10; print $a / 2;");
  MVCR("<?php $a = 't'; $a = 10; print 2 / $a;");
  MVCR("<?php $a = 't'; $a = 10; $b = 'a'; $b = 2; print $a / $b;");
  MVCR("<?php $a = 't'; $a = 10; $a /= 2; print $a;");
  MVCR("<?php $a = 't'; $a = 10; $a /= '5'; print $a;");

  MVCR("<?php $a = 't'; $a = 10; print $a % 2;");
  MVCR("<?php $a = 't'; $a = 10; print 2 % $a;");
  MVCR("<?php $a = 't'; $a = 10; $b = 'a'; $b = 2; print $a % $b;");
  MVCR("<?php $a = 't'; $a = 10; $a %= 2; print $a;");
  MVCR("<?php $a = 't'; $a = 10; $a %= '5'; print $a;");

  MVCR("<?php $a = 't'; $a = 10; var_dump(~$a);");

  MVCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a & $b);");
  MVCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a | $b);");
  MVCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a ^ $b);");
  MVCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a << 2);");
  MVCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a >> 2);");

  MVCR("<?php $b = 't'; $a = 10; $b = 9; var_dump($a & $b);");
  MVCR("<?php $b = 't'; $a = 10; $b = 9; var_dump($a | $b);");
  MVCR("<?php $b = 't'; $a = 10; $b = 9; var_dump($a ^ $b);");
  MVCR("<?php $b = 't'; $a = 10; $b = 2; var_dump($a << $b);");
  MVCR("<?php $b = 't'; $a = 10; $b = 2; var_dump($a >> $b);");

  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a & $b);");
  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a | $b);");
  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a ^ $b);");
  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 2; var_dump($a << $b);");
  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 2; var_dump($a >> $b);");

  MVCR("<?php $a = 't'; $a = 10; $b = 9; $a &= $b; var_dump($a);");
  MVCR("<?php $a = 't'; $a = 10; $b = 9; $a |= $b; var_dump($a);");
  MVCR("<?php $a = 't'; $a = 10; $b = 9; $a ^= $b; var_dump($a);");
  MVCR("<?php $a = 't'; $a = 10; $b = 9; $a <<= 2; var_dump($a);");
  MVCR("<?php $a = 't'; $a = 10; $b = 9; $a >>= 2; var_dump($a);");

  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a and $b);");
  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a or $b);");
  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a xor $b);");
  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump(!$a);");
  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a && $b);");
  MVCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a || $b);");

  MVCR("<?php $a = 10; ++$a; var_dump($a);");
  MVCR("<?php $a = 10; $a++; var_dump($a);");
  MVCR("<?php $a = 10; --$a; var_dump($a);");
  MVCR("<?php $a = 10; $a--; var_dump($a);");

  MVCR("<?php $a = 't'; $a = 10; ++$a; var_dump($a);");
  MVCR("<?php $a = 't'; $a = 10; $a++; var_dump($a);");
  MVCR("<?php $a = 't'; $a = 10; --$a; var_dump($a);");
  MVCR("<?php $a = 't'; $a = 10; $a--; var_dump($a);");

  MVCR("<?php $a = 'test'; ++$a; var_dump($a);");
  MVCR("<?php $a = 'test'; $a++; var_dump($a);");
  MVCR("<?php $a = 'test'; --$a; var_dump($a);");
  MVCR("<?php $a = 'test'; $a--; var_dump($a);");

  MVCR("<?php $a = 1; $a = 'test'; var_dump(~$a);");
  MVCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; var_dump($a & $b);");
  MVCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; var_dump($a | $b);");
  MVCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; var_dump($a ^ $b);");
  MVCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; $a &= $b; var_dump($a);");
  MVCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; $a |= $b; var_dump($a);");
  MVCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; $a ^= $b; var_dump($a);");
  MVCR("<?php class a { public $var2 = 1; public $var1; }"
      "class b extends a { public $var2; }"
      "function f() { $obj1 = new b(); var_dump($obj1); $obj1->var1 = 1; }"
      "f();"); //#147156
  return true;
}

bool TestCodeRun::TestObject() {
  MVCR("<?php "
      "var_dump((object)NULL);"
      "var_dump((object)true);"
      "var_dump((object)10);"
      "var_dump((object)'test');"
      "var_dump((object)array(10, 20));"
      );

  MVCR("<?php class A {} $obj = new A(); "
      "var_dump($obj);"
      "var_dump((bool)$obj);"
      "var_dump((int)$obj);"
      "var_dump((array)$obj);"
      "var_dump((object)$obj);"
      );

  MVCR("<?php class A { public $test = 'ok';} $obj = new A(); "
      "var_dump($obj);"
      "var_dump((bool)$obj);"
      "var_dump((int)$obj);"
      "var_dump((array)$obj);"
      "var_dump((object)$obj);"
      );

  MVCR("<?php "
      "var_dump((object)NULL);"
      "var_dump((object)true);"
      "var_dump((object)10);"
      "var_dump((object)'test');"
      "var_dump((object)array(10, 20));"
      );

  MVCR("<?php class A { public $a = 0;} class B extends A {}"
      "$obj1 = new A(); $obj2 = new A(); $obj2->a++; "
      "$obj3 = new B(); $obj3->a = 10;"
      "var_dump($obj1->a);"
      "var_dump($obj1);"
      "var_dump($obj2->a);"
      "var_dump($obj2);"
      "var_dump($obj3);"
      "var_dump($obj1 instanceof A);"
      "var_dump($obj3 instanceof A);"
      "var_dump($obj1 instanceof B);"
      "var_dump($obj3 instanceof B);"
      );

  MVCR("<?php "
      "class A {"
      "  public $a = 3;"
      "  public function __construct($a) {"
      "    $this->a = $a + 1;"
      "  }"
      "  public function __destruct() {"
      "    $this->a += 2;"
      "    var_dump($this->a);"
      "  }"
      "}"
      "class B extends A {"
      "  public function __construct($a) {"
      "  }"
      "}"
      "class C extends A {"
      "  public function __construct($a) {"
      "    parent::__construct($a);"
      "  }"
      "}"
      "$obj = new A(1); var_dump($obj->a);"
      "$obj = new B(1); var_dump($obj->a);"
      "$obj = new C(1); var_dump($obj->a);"
      );

  MVCR("<?php "
      "class A {"
      "  public $b = 3;"
      "  public $a = 2;"
      "}"
      "$obj = new A(); var_dump($obj); var_dump($obj->c);"
      );

  MVCR("<?php "
      "class A {"
      "  public $a = 2;"
      "}"
      "class B {"
      "  public $b = 3;"
      "}"
      "$obj = new A(); var_dump($obj); /*var_dump($obj->b);*/" // undeclared
      "$obj = new B(); var_dump($obj); var_dump($obj->b);"
      );

  MVCR("<?php "
      "class A {"
      "  public $a = 2;"
      "}"
      "class B {"
      "  public $b = 3;"
      "}"
      "$obj = new A(); var_dump($obj); var_dump($obj->a);"
      "$obj = new B(); var_dump($obj); var_dump($obj->b);"
      );

  MVCR("<?php "
      "class A {"
      "  public $a = 2;"
      "}"
      "class B extends A {"
      "  public $b = 3;"
      "}"
      "$obj = new A(); var_dump($obj); var_dump($obj->b);"
      "$obj = new B(); var_dump($obj); var_dump($obj->b);"
      );

  MVCR("<?php "
      "class A {"
      "  public $a = 2;"
      "  public $b = 1;"
      "}"
      "class B extends A {"
      "  public $b = 3;"
      "}"
      "$obj = new A(); var_dump($obj); var_dump($obj->b);"
      "$obj = new B(); var_dump($obj); var_dump($obj->b);"
      );

  MVCR("<?php "
      "interface I { public function test($a);}"
      "class A implements I { public function test($a) { print $a;}}"
      "$obj = new A(); var_dump($obj instanceof I); $obj->test('cool');"
      );

  MVCR("<?php "
      "interface I { public function test($a);} "
      "class A { public function test($a) { print 'A';}} "
      "class B extends A implements I { "
      "  public function test($a) { print 'B';} "
      "}"
      "$obj = new A(); $obj->test(1);"
      "$obj = new B(); $obj->test(1);"
      );

  // circular references
  MVCR("<?php class A { public $a;} "
      "$obj1 = new A(); $obj2 = new A(); $obj1->a = $obj2; $obj2->a = $obj1;"
      "var_dump($obj1);");

  MVCR("<?php $a = 1; class A { public function t() { global $a; $b = 'a'; var_dump($$b);}} $obj = new A(); $obj->t();");

  MVCR("<?php "
      "class g {"
      "  public $v;"
      "  function set($v) {"
      "    $this->v = $v;"
      "    return $this;"
      "  }"
      "}"
      "function foo() {"
      "  $z = 1;"
      "  $qd = array('x' => $z);"
      "  $a = G()->set($qd);"
      "  var_dump($a);"
      "  $qd['e'] = true;"
      "  $b = G()->set($qd);"
      "  var_dump($a);"
      ""
      "}"
      "function G() {"
      "  return new g;"
      "}"
      "foo();");
  MVCR("<?php "
      "class EE extends Exception {"
      "}"
      "class E extends EE {"
      "  function foo() {}"
      "  function __construct() {"
      "    echo 'MAKING E';"
      "    parent::__construct();"
      "  }"
      "}"
      "new E;");
  MVCR("<?php "
      "class A {"
      "  var $a;"
      "  var $b;"
      "};"
      "$obj = new A();"
      "var_dump($obj);"
      "foreach ($obj as &$value) {"
      "  $value = 1;"
      "}"
      "var_dump($obj);"
      "$obj->c = 3;"
      "var_dump($obj);"
      "foreach ($obj as &$value) {"
      "  $value = 2;"
      "}"
      "var_dump($obj);");
  MVCR("<?php "
      "class A {"
      "  var $a;"
      "  var $b;"
      "};"
      "$obj = new A();"
      "$obj2 = $obj;"
      "foreach ($obj2 as $k => &$value) {"
      "  $value = 'ok';"
      "}"
      "var_dump($obj);"
      "var_dump($obj2);");
  MVCR("<?php "
      "class A {"
      "  var $a;"
      "  var $b;"
      "  var $c;"
      "  var $d;"
      "  public function __construct($p1, $p2, $p3, $p4) {"
      "    $this->a = $p1;"
      "    $this->b = $p2;"
      "    $this->c = $p3;"
      "    $this->d = $p4;"
      "  }"
      "};"
      "$obj = new A(1, 2, 3, 4);"
      "foreach ($obj as $key => &$val) {"
      "  if($val == 2) {"
      "    $obj->$key = 0;"
      "  } else if($val == 3) {"
      "    var_dump($key);"
      "    unset($obj->$key);"
      "  } else {"
      "    $val++;"
      "  }"
      "}"
      "var_dump($obj);");
  MVCR("<?php "
      "class A {"
      "  var $a = 1;"
      "  var $b = 2;"
      "  var $c = 3;"
      "  var $d = 4;"
      "  public function __construct() {"
      "    $this->a = 1;"
      "    $this->b = 2;"
      "    $this->c = 3;"
      "    $this->d = 4;"
      "  }"
      "};"
      "function f() {"
      "  $obj = new A();"
      "  foreach ($obj as $key => &$val) {"
      "    $val = 5;"
      "  }"
      "  var_dump($obj);"
      "}"
      "f();");

  MVCR("<?php function test() {"
       "$abc = 'abc'; var_dump($abc instanceof Nothing); }"
      "test();");

  MVCR("<?php "
       "class X {"
       "  public $x = 1;"
       "  }"
       "class Y {"
       "  public $x = array();"
       "}"
       "function test() {"
       "  $x = (object)new Y;"
       "  var_dump($x->x);"
       "  $x = new X;"
       "  var_dump($x->x);"
       "}"
       "test();");

  MVCR("<?php\n"
       "class A { }\n"
       "class B extends A {\n"
       "  static function check1($a) { return $a instanceof self; }\n"
       "  static function check2($a) { return $a instanceof parent; }\n"
       "}\n"
       "$a = new B;\n"
       "var_dump(B::check1($a), B::check2($a));\n"
       "$b = (object)array(1, 2, 3);\n"
       "var_dump(B::check1($b), B::check2($b));\n");

  MVCR("<php\n"
       "class X { static function f($o, $s) { return $o instanceof $s; } }\n"
       "$x = new X;\n"
       "var_dump(X::f($x, 'self'));\n"
       "var_dump(X::f($x, 'X'));\n");

  MVCR("<php\n"
       "class X {\n"
       "  static function f($o) {\n"
       "    $s = 'self'; return $o instanceof $s;\n"
       "  }\n"
       "}\n"
       "X::f(new X);\n");

  return true;
}

bool TestCodeRun::TestObjectProperty() {
  MVCR("<?php "
       "class A { static protected $foo = 11; "
       "  function foo() { var_dump(A::$foo);}} "
       "class B extends A { static public $foo;} "
       "var_dump(B::$foo); B::$foo = 123; A::foo();");
  MVCR("<?php "
       "class A { static protected $foo = 11; } "
       "class B extends A {} "
       "class C extends B { static public $foo;} "
       "var_dump(C::$foo); ");
  MVCR("<?php "
       "class A { static private $foo = 11; } "
       "class B extends A {} "
       "class C extends B { static public $foo;} "
       "var_dump(C::$foo); ");

  MVCR("<?php\n"
       "class A {\n"
       "  public $a = 'apple';\n"
       "  public $b = 'banana';\n"
       "}\n"
       "\n"
       "$old = new A;\n"
       "unset($old->a);\n"
       "var_dump($old);\n"
       "\n"
       "$new = new A;\n"
       "unset($new->b);\n"
       "var_dump($new);\n"
       "\n"
       "foreach ($new as $property => $value) {\n"
       "  $old->$property = $value;\n"
       "}\n"
       "var_dump($old->a);\n"
       "var_dump($old->b);\n"
       // We can't really maintain the same order as PHP does, even though we
       // can move unset and reset properties to the back of the list, but
       // then we can't really tell who's added back first if there are two :-(
       //"var_dump($old);\n"
       );
  MVCR("<?php\n"
       "class A {\n"
       "  public $a = 'apple';\n"
       "}\n"
       "$obj = new A;\n"
       "var_dump(isset($obj->a), property_exists($obj, 'a'));\n"
       "$obj->a = null;\n"
       "var_dump(isset($obj->a), property_exists($obj, 'a'));\n"
       "unset($obj->a);\n"
       "var_dump(isset($obj->a), property_exists($obj, 'a'));\n"
       "$obj->a = 123;\n"
       "var_dump(isset($obj->a), property_exists($obj, 'a'));\n"
       "$obj->a = null;\n"
       "var_dump(isset($obj->a), property_exists($obj, 'a'));\n"
       );

  MVCR("<?php\n"
       "class A {\n"
       "  private $prop = 'test';\n"
       "\n"
       "  function __get($name) {\n"
       "    return $this->$name;\n"
       "  }}\n"
       "\n"
       "$obj = new A();\n"
       "var_dump($obj->prop);\n"
      );

  MVCR("<?php "
      "class A { public $a = 10; public function foo() { $this->a = 20;} } "
      "class B extends A { public $a = 'test';} "
      "$obj = new B(); $obj->foo(); var_dump($obj->a);");

  MVCR("<?php "
      "class A { public $a = null; }"
      "class B extends A { public function foo() { var_dump($this->a);} } "
      "class C extends B { public $a = 'test';} "
      "$obj = new C(); $obj->foo();");
  MVCR("<?php "
      "$y = true;"
      "define('foo', $y ? 1 : 0);"
      "if (false) {"
      "  class redecClass {"
      "  }"
      "} else {"
      "  final class redecClass {"
      "    const redecConst = foo;"
      "    const redecConst2 = 456;"
      "    public static $fx = foo;"
      "  }"
      "}"
      "class T {"
      "  const c = foo;"
      "  const c2 = redecClass::redecConst;"
      "  const c3 = redecClass::redecConst2;"
      "  public static $q = foo;"
      "  public static $n = 123;"
      "}"
      "class T2 {"
      "  const c = foo;"
      "  public static $q = foo;"
      "}"
      "class T3 {"
      "  const c = foo;"
      "}"
      "class normal {"
      "  const C = 1;"
      "  public static $xx = 123;"
      "}"
      "function test() {"
      "  var_dump(T::c);"
      "  var_dump(T::c2);"
      "  var_dump(T::c3);"
      "  var_dump(T::$q);"
      "  var_dump(T::$n);"
      "  var_dump(T2::c);"
      "  var_dump(T2::$q);"
      "  var_dump(T3::c);"
      "  var_dump(normal::C);"
      "  var_dump(normal::$xx);"
      "}"
      "test();");
  MVCR("<?php "
      "class c {"
      "  public $d = 'd';"
      "  private $e = 'e';"
      "  protected $f = 'f';"
      "  function t1($y) {"
      "    foreach ($y as $k => $v) {"
      "      var_dump($v);"
      "    }"
      "  }"
      "}"
      "class d extends c {"
      "  public $a = 'a';"
      "  private $b = 'b';"
      "  protected $c = 'c';"
      "  function t2($y) {"
      "    foreach ($this as $k => $v) {"
      "      var_dump($v);"
      "    }"
      "    foreach ($y as $k => $v) {"
      "      var_dump($v);"
      "    }"
      "    foreach ($y as $k => $v) {"
      "      var_dump($v);"
      "    }"
      "  }"
      "}"
      "$x = new d;"
      "$x->surprise = 1;"
      "$y = new d;"
      "$y->shock = 2;"
      "echo \"t2\n\";"
      "$x->t2($y);"
      "echo \"t1\n\";"
      "$x->t1($y);"
      "$z = new c;"
      "echo \"t12\n\";"
      "$z->t1($x);"
      "foreach ($x as $k => $v) {"
      "  var_dump($v);"
      "}");

  MVCR("<?php "
      "$one = array('cluster'=> 1, 'version'=>2);"
      "var_dump(isset($one->cluster));"
      "var_dump(empty($one->cluster));"
      "$two = 'hello';"
      "var_dump(isset($two->scalar));");

  MVCR("<?php "
      "function f() { return false; }"
      "if (f()) { class A { } }"
      "else { class A { static $a = 100; var $b = 1000; } }"
      "class B { var $a = 1; static $b = array(1, 2, 3); }"
      "var_dump(get_class_vars('A'));"
      "A::$a = 1;"
      "var_dump(get_class_vars('A'));"
      "var_dump(get_class_vars('B'));");
  MVCR("<?php "
      "if (true) {"
      "  class A {"
      "    var $a0;"
      "    static $a1 = 1;"
      "    static $a2 = 2;"
      "  }"
      "  class X {"
      "    var $x0;"
      "    static $x1 = 1;"
      "    static $x2 = 2;"
      "  }"
      "} else {"
      "  class A {"
      "    var $a3;"
      "    static $a4 = 4;"
      "    static $a5 = 5;"
      "  }"
      "  class X {"
      "    var $y3;"
      "    static $y4 = 4;"
      "    static $y5 = 5;"
      "  }"
      "}"
      "class B extends A {"
      "  var $b0;"
      "  static $b1 = 1;"
      "  static $b2 = 2;"
      "}"
      "class Y extends X {"
      "  var $y0;"
      "  static $y1 = 1;"
      "  static $y2 = 2;"
      "}"
      "class C {"
      "  var $c0;"
      "  static $c1 = 1;"
      "  static $c2 = 2;"
      "}"
      "class Z {"
      "  var $z0;"
      "  static $z1 = 1;"
      "  static $z2 = 2;"
      "}"
      "var_dump(get_class_vars('A'));"
      "var_dump(get_class_vars('B'));"
      "var_dump(get_class_vars('C'));"
      "var_dump(get_class_vars('X'));"
      "var_dump(get_class_vars('Y'));"
      "var_dump(get_class_vars('Z'));");
  MVCR("<?php "
      "class A {"
      "  private $pri = 'a-pri';"
      "  protected $pro = 'a-pro';"
      "  function bar() {"
      "    var_dump($this->pri);"
      "    $pri = $q ? 'zz' : 'p'.'ri';"
      "    var_dump($this->$pri);"
      "  }"
      "  function bar2() {"
      "    var_dump($this->pro);"
      "  }"
      "}"
      "class B extends A {"
      "  private $pri = 'b-pri';"
      "  function bar() {"
      "    parent::bar();"
      "    var_dump($this->pri);"
      "  }"
      "}"
      "$obj = new B(); $obj->bar();"
      "class C extends B {"
      "  public $pri = 'c-pri';"
      "  public $pro = 'c-pro';"
      "  function bar2() {"
      "    parent::bar2();"
      "    var_dump($this->pro);"
      "  }"
      "}"
      "$obj = new C; $obj->bar(); $obj->bar2();"
      "var_dump(serialize($obj));"
      "class Base {"
      "  protected $pro = 1;"
      "  private $pri = 'base-pri';"
      "  function q0() {"
      "    var_dump($this->pri);"
      "  }"
      "}"
      ""
      "class R extends Base {"
      "  public $rpub = 1;"
      "  protected $pro = 2;"
      "  private $pri = 'r-pri';"
      "  function q() {"
      "    var_dump($this->pri);"
      "  }"
      "}"
      ""
      "class D extends R {"
      "  public $dpub = 'd';"
      "  protected $pro = 'pro';"
      "  private $pri = 'd-pri';"
      "  function qq() {"
      "    var_dump($this->pri);"
      "  }"
      "}"
      ""
      "class DD extends D {"
      "  private $pri = 'dd-pri';"
      "  function qqq() {"
      "    var_dump($this->pri);"
      "  }"
      "}"
      ""
      "if (false) {"
      "  class R{}"
      "}"
      "$d = new D;"
      "$d->qq();"
      "$d->q();"
      "$d->q0();"
      "$d = new DD;"
      "$d->qqq();"
      "$d->qq();"
      "$d->q();"
      "$d->q0();"
      );

  MVCR("<?php\n"
      "class A { }\n"
      "$a = new A();\n"
      "$f = 20;\n"
      "$a->$f = 100;\n"
      "var_dump($a);\n"
      "unset($a->$f);\n"
      "var_dump($a);\n");

  MVCR("<?php "
       "class X {"
       "  public $x = 0;"
       "  function y($u,&$a) {"
       "    $a++;"
       "  }"
       "};"
       "if (0) { class X{} }"
       "function f() {}"
       "function test() {"
       "  $x = new X;"
       "  $x->y(f(),$x->x);"
       "  var_dump($x);"
       "  $x->y(0,$x->x);"
       "  var_dump($x);"
       "}"
       "test();");
  MVCR("<?php "
      "class A {"
      "  private $a = array('apple');"
      "  private $b = 'banana';"
      "  function foo() {"
      "    $b = new A();"
      "    unset($b->b);"
      "    var_dump($b);"
      "    foreach ($b as $prop => $value) {"
      "      var_dump($prop);"
      "    }"
      "  }"
      "}"
      "A::foo();");

  MVCR("<?php "
       "class X {"
       "  public $pub_var = null;"
       "  public $pub_set = array();"
       "  private $priv_var = 2;"
       "  function __get($name) {"
       "    echo 'get: '; var_dump($name); return $name == 'buz' ? 1 : array();"
       "  }"
       "  function __isset($name) {"
       "    echo 'isset: '; var_dump($name);"
       "    return $name == 'baz' || $name == 'buz';"
       "  }"
       "}"
       "$x = new X;"
       "var_dump(isset($x->foo));"
       "var_dump(isset($x->baz));"
       "var_dump(isset($x->buz));"
       "var_dump(isset($x->pub_var));"
       "var_dump(isset($x->pub_set));"
       "var_dump(isset($x->priv_var));"
       "var_dump(empty($x->foo));"
       "var_dump(empty($x->baz));"
       "var_dump(empty($x->buz));"
       "var_dump(empty($x->pub_var));"
       "var_dump(empty($x->pub_set));"
       "var_dump(empty($x->priv_var));"
       "unset($x->pub_var);"
       "var_dump(isset($x->pub_var));"
       "var_dump(empty($x->pub_var));");

  MVCR("<?php "
       "define('FOO', 'foo');"
       "class X {"
       "  const UNKNOWN = 1;"
       "  public $foo = -1;"
       "  static public $bar = FOO;"
       "  public $baz = self::UNKNOWN;"
       "}"
       "var_dump(get_class_vars('X'));");

  // Mutable object foreach needs to respect the context.
  MVCR("<?php\n"
       "class A { public $a = 1; protected $b = 2; private $c = 3; }\n"
       "class B extends A {\n"
       "  function f() {\n"
       "    foreach ($this as $k => &$v) { var_dump($k); $v = 100; }\n"
       "    var_dump($this);\n"
       "  }\n"
       "}\n"
       "$b = new B();\n"
       "$b->f();\n"
       "function f() {\n"
       "  $o = new B();\n"
       "  foreach ($o as $k => &$v) { var_dump($k); $v = 100; }\n"
       "  var_dump($o);\n"
       "}\n"
       "f();\n");

  // empty property name shouldn't crash
  MVCR("<?php\n"
       "$a = array(); $a[""] = 1;\n"
       "$o = (object)$a;\n"
       "var_dump($o);\n"
       "$s = serialize($o);\n"
       "$o2 = unserialize($s);\n");

  MVCR("<?php "
       "class Test"
       "{"
       "  protected static $color = array('gray' => 30);"
       "  public static function foo($type, $key) {"
       "    return isset( self::${$type}[$key] );"
       "  }"
       "}"
       "var_dump(Test::foo('color', 'gray'));");

 return true;
}

bool TestCodeRun::TestObjectMethod() {
  MVCR("<?php class A { function test() {}} "
      "$obj = new A(); $obj->test(); $obj = 1;");

  // calling a function that's implemented in a derived class
  MVCR("<?php abstract class T { function foo() { $this->test();} }"
      "class R extends T { function test() { var_dump('test'); }} "
      "$obj = new R(); $obj->test(); $obj->foo();");

  MVCR("<?php class A { function test() { print 'A';} "
      "function foo() { $this->test();}} "
      "class B extends A { function test() { print 'B';}} "
      "$obj = new A(); $obj = new B(); $obj->foo();");

  MVCR("<?php "
      "class A {} "
      "class AA extends A { function test() { print 'AA ok';} }"
      "class B { function foo(A $obj) { $obj->test();}}"
      "$obj = new AA(); $b = new B(); $b->foo($obj);"
      );

  // calling a virtual function
  MVCR("<?php abstract class T { abstract function test(); "
      "function foo() { $this->test();} }"
      "class R extends T { function test() { var_dump('test'); }} "
      "$obj = new R(); $obj->test(); $obj->foo();");

  // calling a virtual function
  MVCR("<?php abstract class T { abstract function test(); } "
      "class R extends T { function test() { var_dump('test'); }} "
      "$obj = 1; $obj = new R(); $obj->test();");

  MVCR("<?php "
      "class foo {"
      "  public function test1() {"
      "    echo 'in test1';"
      "  }"
      "  public function test2() {"
      "    self::test1();"
      "    foo::test1();"
      "    echo 'in test2';"
      "  }"
      "  public function test3() {"
      "    echo 'in test3';"
      "  }"
      "  public static function test4() {"
      "    echo 'in test4';"
      "  }"
      "}"
      "$obj = new foo();"
      "$obj->test2();"
      "foo::test1();"
      "foo::test2();"
      "foo::test3();"
      "$obj->test3();"
      "$obj->test4();");

  // calling instance method statically
  MVCR("<?php "
      "class A1 {"
      "  function a1f($a) {"
      "    var_dump('a1f:0');"
      "  }"
      "  static function a1b($a) {"
      "    var_dump('a1b:0');"
      "  }"
      "}"
      "class B1 extends A1 {"
      "  function b1f($a) {"
      "    var_dump('b1f:0');"
      "  }"
      "  static function b1b($a) {"
      "    var_dump('b1b:0');"
      "  }"
      "}"
      "$f = 'a1f';"
      "$b = 'a1b';"
      "A1::$f(1);"
      "A1::$b(1);"
      "B1::$f(1);"
      "B1::$b(1);"
      "$f = 'b1f';"
      "$b = 'b1b';"
      "B1::$f(1);"
      "B1::$b(1);"
      "$f = 'b2f';"
      "$b = 'b2b';"
      "call_user_func_array(array('B1', 'b1f'), 1);");

  // calling instance method through self
  MVCR("<?php "
      "class A {"
      "  var $a;"
      "  function f() {"
      "    var_dump($this->a);"
      "  }"
      "  function g() {"
      "    $this->a = 100;"
      "    call_user_func(array('self', 'f'));"
      "  }"
      "}");

  MVCR("<?php "
      "class c {"
      "function foo() { echo \"called\n\"; }"
      "}"
      "function meh() {}"
      "function z() {"
      "  $p = new c;"
      "  $p->foo(meh());"
      "  $p = null;"
      "}"
      "z();");
  MVCR("<?php "
       "interface intf {"
       "  function meth();"
       "}"
       "class m {"
       "  function meth() {"
       "    return 1;"
       "  }"
       "}"
       "class m2 extends m implements intf {"
       "}"
       "class m3 extends m2 {"
       "  function f() {"
       "    var_dump(parent::meth());"
       "  }"
       "}"
       "$y = new m3;"
       "$y->f();"
       "function g() {"
       "  $y = new m3;"
       "  var_dump($y->meth());"
       "}"
       "g();");
  MVCR("<?php "
       "class c {"
       "  function x() {"
       "    var_dump($this);"
       "    $t = 'this';"
       "    var_dump($$t);"
       "  }"
       "}"
       "$x = new c;"
       "$x->x();");
  MVCR("<?php "
      "class RootBase {}"
      "class Base extends RootBase {"
      "  private $privateData;"
      "}"
      "class Test extends Base {"
      "  protected function f1() {"
      "    $this->privateData = 1;"
      "    var_dump('ok');"
      "  }"
      "  public function f2() {"
      "    $this->f1();"
      "  }"
      "}"
      "function foo() {"
      "  $obj = new Test();"
      "  $obj->f2();"
      "  $obj->privateData = 2;"
      "  $obj = new Base();"
      "}"
      "foo();");

#define TEST_BODY_FOR_IS_CALLABLE \
      "var_dump(is_callable('A::a_spub'));" \
      "var_dump(is_callable('A::a_spro'));" \
      "var_dump(is_callable('A::a_spri'));" \
      "var_dump(is_callable('A::a_pro'));" \
      "var_dump(is_callable('A::a_pri'));" \
      "var_dump(is_callable('a_spub'));" \
      "var_dump(is_callable('a_spro'));" \
      "var_dump(is_callable('a_spri'));" \
      "var_dump(is_callable('a_pub'));" \
      "var_dump(is_callable('a_pro'));" \
      "var_dump(is_callable('a_pri'));" \
      "var_dump(is_callable('B::b_spub'));" \
      "var_dump(is_callable('B::b_spro'));" \
      "var_dump(is_callable('B::b_spri'));" \
      "var_dump(is_callable('B::b_pro'));" \
      "var_dump(is_callable('B::b_pri'));" \
      "var_dump(is_callable('b_spub'));" \
      "var_dump(is_callable('b_spro'));" \
      "var_dump(is_callable('b_spri'));" \
      "var_dump(is_callable('b_pub'));" \
      "var_dump(is_callable('b_pro'));" \
      "var_dump(is_callable('b_pri'));" \
      "var_dump(is_callable(array('A', 'a_spub')));" \
      "var_dump(is_callable(array('A', 'a_spro')));" \
      "var_dump(is_callable(array('A', 'a_spri')));" \
      "var_dump(is_callable(array('A', 'a_pub')));" \
      "var_dump(is_callable(array('A', 'a_pro')));" \
      "var_dump(is_callable(array('A', 'a_pri')));" \
      "var_dump(is_callable(array('A', 'A::a_spub')));" \
      "var_dump(is_callable(array('A', 'A::a_spro')));" \
      "var_dump(is_callable(array('A', 'A::a_spri')));" \
      "var_dump(is_callable(array('A', 'A::a_pro')));" \
      "var_dump(is_callable(array('A', 'A::a_pri')));" \
      "var_dump(is_callable(array('A', 'A::A::a_spub')));" \
      "var_dump(is_callable(array('A', 'A::A::a_spro')));" \
      "var_dump(is_callable(array('A', 'A::A::a_spri')));" \
      "var_dump(is_callable(array('A', 'A::A::a_pro')));" \
      "var_dump(is_callable(array('A', 'A::A::a_pri')));" \
      "var_dump(is_callable(array('A', 'C::a_spub')));" \
      "var_dump(is_callable(array('A', 'C::a_spro')));" \
      "var_dump(is_callable(array('A', 'C::a_spri')));" \
      "var_dump(is_callable(array('A', 'C::a_pub')));" \
      "var_dump(is_callable(array('A', 'C::a_pro')));" \
      "var_dump(is_callable(array('A', 'C::a_pri')));" \
      "var_dump(is_callable(array('A', 'A::C::a_spub')));" \
      "var_dump(is_callable(array('A', 'A::C::a_spro')));" \
      "var_dump(is_callable(array('A', 'A::C::a_spri')));" \
      "var_dump(is_callable(array('A', 'A::C::a_pub')));" \
      "var_dump(is_callable(array('A', 'A::C::a_pro')));" \
      "var_dump(is_callable(array('A', 'A::C::a_pri')));" \
      "var_dump(is_callable(array('A', 'b_spub')));" \
      "var_dump(is_callable(array('A', 'b_spro')));" \
      "var_dump(is_callable(array('A', 'b_spri')));" \
      "var_dump(is_callable(array('A', 'b_pub')));" \
      "var_dump(is_callable(array('A', 'b_pro')));" \
      "var_dump(is_callable(array('A', 'b_pri')));" \
      "var_dump(is_callable(array('A', 'B::a_spub')));" \
      "var_dump(is_callable(array('A', 'B::a_spro')));" \
      "var_dump(is_callable(array('A', 'B::a_spri')));" \
      "var_dump(is_callable(array('A', 'B::a_pub')));" \
      "var_dump(is_callable(array('A', 'B::a_pro')));" \
      "var_dump(is_callable(array('A', 'B::a_pri')));" \
      "var_dump(is_callable(array('A', 'B::B::a_spub')));" \
      "var_dump(is_callable(array('A', 'B::B::a_spro')));" \
      "var_dump(is_callable(array('A', 'B::B::a_spri')));" \
      "var_dump(is_callable(array('A', 'B::B::a_pub')));" \
      "var_dump(is_callable(array('A', 'B::B::a_pro')));" \
      "var_dump(is_callable(array('A', 'B::B::a_pri')));" \
      "var_dump(is_callable(array('B', 'a_spub')));" \
      "var_dump(is_callable(array('B', 'a_spro')));" \
      "var_dump(is_callable(array('B', 'a_spri')));" \
      "var_dump(is_callable(array('B', 'a_pub')));" \
      "var_dump(is_callable(array('B', 'a_pro')));" \
      "var_dump(is_callable(array('B', 'a_pri')));" \
      "var_dump(is_callable(array('B', 'A::a_spub')));" \
      "var_dump(is_callable(array('B', 'A::a_spro')));" \
      "var_dump(is_callable(array('B', 'A::a_spri')));" \
      "var_dump(is_callable(array('B', 'A::a_pro')));" \
      "var_dump(is_callable(array('B', 'A::a_pri')));" \
      "var_dump(is_callable(array('B', 'A::A::a_spub')));" \
      "var_dump(is_callable(array('B', 'A::A::a_spro')));" \
      "var_dump(is_callable(array('B', 'A::A::a_spri')));" \
      "var_dump(is_callable(array('B', 'A::A::a_pro')));" \
      "var_dump(is_callable(array('B', 'A::A::a_pri')));" \
      "var_dump(is_callable(array('B', 'C::a_spub')));" \
      "var_dump(is_callable(array('B', 'C::a_spro')));" \
      "var_dump(is_callable(array('B', 'C::a_spri')));" \
      "var_dump(is_callable(array('B', 'C::a_pub')));" \
      "var_dump(is_callable(array('B', 'C::a_pro')));" \
      "var_dump(is_callable(array('B', 'C::a_pri')));" \
      "var_dump(is_callable(array('B', 'B::C::a_spub')));" \
      "var_dump(is_callable(array('B', 'B::C::a_spro')));" \
      "var_dump(is_callable(array('B', 'B::C::a_spri')));" \
      "var_dump(is_callable(array('B', 'B::C::a_pub')));" \
      "var_dump(is_callable(array('B', 'B::C::a_pro')));" \
      "var_dump(is_callable(array('B', 'B::C::a_pri')));" \
      "var_dump(is_callable(array('B', 'b_spub')));" \
      "var_dump(is_callable(array('B', 'b_spro')));" \
      "var_dump(is_callable(array('B', 'b_spri')));" \
      "var_dump(is_callable(array('B', 'b_pub')));" \
      "var_dump(is_callable(array('B', 'b_pro')));" \
      "var_dump(is_callable(array('B', 'b_pri')));" \
      "var_dump(is_callable(array('B', 'B::a_spub')));" \
      "var_dump(is_callable(array('B', 'B::a_spro')));" \
      "var_dump(is_callable(array('B', 'B::a_spri')));" \
      "var_dump(is_callable(array('B', 'B::a_pub')));" \
      "var_dump(is_callable(array('B', 'B::a_pro')));" \
      "var_dump(is_callable(array('B', 'B::a_pri')));" \
      "var_dump(is_callable(array('B', 'B::A::a_spub')));" \
      "var_dump(is_callable(array('B', 'B::A::a_spro')));" \
      "var_dump(is_callable(array('B', 'B::A::a_spri')));" \
      "var_dump(is_callable(array('B', 'B::A::a_pro')));" \
      "var_dump(is_callable(array('B', 'B::A::a_pri')));" \
      "var_dump(is_callable(array('B', 'B::B::a_spub')));" \
      "var_dump(is_callable(array('B', 'B::B::a_spro')));" \
      "var_dump(is_callable(array('B', 'B::B::a_spri')));" \
      "var_dump(is_callable(array('B', 'B::B::a_pub')));" \
      "var_dump(is_callable(array('B', 'B::B::a_pro')));" \
      "var_dump(is_callable(array('B', 'B::B::a_pri')));" \
      "var_dump(is_callable(array(new A(), 'a_spub')));" \
      "var_dump(is_callable(array(new A(), 'a_spro')));" \
      "var_dump(is_callable(array(new A(), 'a_spri')));" \
      "var_dump(is_callable(array(new A(), 'a_pub')));" \
      "var_dump(is_callable(array(new A(), 'a_pro')));" \
      "var_dump(is_callable(array(new A(), 'a_pri')));" \
      "var_dump(is_callable(array(new A(), 'A::a_spub')));" \
      "var_dump(is_callable(array(new A(), 'A::a_spro')));" \
      "var_dump(is_callable(array(new A(), 'A::a_spri')));" \
      "var_dump(is_callable(array(new A(), 'A::a_pro')));" \
      "var_dump(is_callable(array(new A(), 'A::a_pri')));" \
      "var_dump(is_callable(array(new A(), 'A::A::a_spub')));" \
      "var_dump(is_callable(array(new A(), 'A::A::a_spro')));" \
      "var_dump(is_callable(array(new A(), 'A::A::a_spri')));" \
      "var_dump(is_callable(array(new A(), 'A::A::a_pro')));" \
      "var_dump(is_callable(array(new A(), 'A::A::a_pri')));" \
      "var_dump(is_callable(array(new A(), 'C::a_spub')));" \
      "var_dump(is_callable(array(new A(), 'C::a_spro')));" \
      "var_dump(is_callable(array(new A(), 'C::a_spri')));" \
      "var_dump(is_callable(array(new A(), 'C::a_pub')));" \
      "var_dump(is_callable(array(new A(), 'C::a_pro')));" \
      "var_dump(is_callable(array(new A(), 'C::a_pri')));" \
      "var_dump(is_callable(array(new A(), 'A::C::a_spub')));" \
      "var_dump(is_callable(array(new A(), 'A::C::a_spro')));" \
      "var_dump(is_callable(array(new A(), 'A::C::a_spri')));" \
      "var_dump(is_callable(array(new A(), 'A::C::a_pub')));" \
      "var_dump(is_callable(array(new A(), 'A::C::a_pro')));" \
      "var_dump(is_callable(array(new A(), 'A::C::a_pri')));" \
      "var_dump(is_callable(array(new A(), 'b_spub')));" \
      "var_dump(is_callable(array(new A(), 'b_spro')));" \
      "var_dump(is_callable(array(new A(), 'b_spri')));" \
      "var_dump(is_callable(array(new A(), 'b_pub')));" \
      "var_dump(is_callable(array(new A(), 'b_pro')));" \
      "var_dump(is_callable(array(new A(), 'b_pri')));" \
      "var_dump(is_callable(array(new A(), 'B::a_spub')));" \
      "var_dump(is_callable(array(new A(), 'B::a_spro')));" \
      "var_dump(is_callable(array(new A(), 'B::a_spri')));" \
      "var_dump(is_callable(array(new A(), 'B::a_pub')));" \
      "var_dump(is_callable(array(new A(), 'B::a_pro')));" \
      "var_dump(is_callable(array(new A(), 'B::a_pri')));" \
      "var_dump(is_callable(array(new A(), 'B::A::a_spub')));" \
      "var_dump(is_callable(array(new A(), 'B::A::a_spro')));" \
      "var_dump(is_callable(array(new A(), 'B::A::a_spri')));" \
      "var_dump(is_callable(array(new A(), 'B::A::a_pro')));" \
      "var_dump(is_callable(array(new A(), 'B::A::a_pri')));" \
      "var_dump(is_callable(array(new B(), 'b_spub')));" \
      "var_dump(is_callable(array(new B(), 'b_spro')));" \
      "var_dump(is_callable(array(new B(), 'b_spri')));" \
      "var_dump(is_callable(array(new B(), 'b_pub')));" \
      "var_dump(is_callable(array(new B(), 'b_pro')));" \
      "var_dump(is_callable(array(new B(), 'b_pri')));" \
      "var_dump(is_callable(array(new B(), 'B::b_spub')));" \
      "var_dump(is_callable(array(new B(), 'B::b_spro')));" \
      "var_dump(is_callable(array(new B(), 'B::b_spri')));" \
      "var_dump(is_callable(array(new B(), 'B::b_pro')));" \
      "var_dump(is_callable(array(new B(), 'B::b_pri')));" \
      "var_dump(is_callable(array(new B(), 'B::B::b_spub')));" \
      "var_dump(is_callable(array(new B(), 'B::B::b_spro')));" \
      "var_dump(is_callable(array(new B(), 'B::B::b_spri')));" \
      "var_dump(is_callable(array(new B(), 'B::B::b_pro')));" \
      "var_dump(is_callable(array(new B(), 'B::B::b_pri')));" \
      "var_dump(is_callable(array(new B(), 'C::b_spub')));" \
      "var_dump(is_callable(array(new B(), 'C::b_spro')));" \
      "var_dump(is_callable(array(new B(), 'C::b_spri')));" \
      "var_dump(is_callable(array(new B(), 'C::b_pub')));" \
      "var_dump(is_callable(array(new B(), 'C::b_pro')));" \
      "var_dump(is_callable(array(new B(), 'C::b_pri')));" \
      "var_dump(is_callable(array(new B(), 'B::C::b_spub')));" \
      "var_dump(is_callable(array(new B(), 'B::C::b_spro')));" \
      "var_dump(is_callable(array(new B(), 'B::C::b_spri')));" \
      "var_dump(is_callable(array(new B(), 'B::C::b_pub')));" \
      "var_dump(is_callable(array(new B(), 'B::C::b_pro')));" \
      "var_dump(is_callable(array(new B(), 'B::C::b_pri')));" \
      "var_dump(is_callable(array(new B(), 'a_spub')));" \
      "var_dump(is_callable(array(new B(), 'a_spro')));" \
      "var_dump(is_callable(array(new B(), 'a_spri')));" \
      "var_dump(is_callable(array(new B(), 'a_pub')));" \
      "var_dump(is_callable(array(new B(), 'a_pro')));" \
      "var_dump(is_callable(array(new B(), 'a_pri')));" \
      "var_dump(is_callable(array(new B(), 'A::B::b_spub')));" \
      "var_dump(is_callable(array(new B(), 'A::B::b_spro')));" \
      "var_dump(is_callable(array(new B(), 'A::B::b_spri')));" \
      "var_dump(is_callable(array(new B(), 'A::B::b_pro')));" \
      "var_dump(is_callable(array(new B(), 'A::B::b_pri')));" \


  MVCR("<?php "
      "function test() {"
        TEST_BODY_FOR_IS_CALLABLE
      "}"
      "class EXT {"
      "  static public function ext_spub() {"
      "    test();"
           TEST_BODY_FOR_IS_CALLABLE
      "  }"
      "}"
      "class A {"
      "  static public function a_spub() {"
      "    test();"
      "  }"
      "  static protected function a_spro() {"
      "    test();"
      "  }"
      "  static private function a_spri() {"
      "    test();"
      "  }"
      "  public function a_pub() {"
      "    test();"
      "  }"
      "  protected function a_pro() {"
      "    test();"
      "  }"
      "  private function a_pri() {"
      "    test();"
      "  }"
      "  public static function a_sf() {"
      "    test();"
      "    self::a_spub();"
      "    self::a_spro();"
      "    self::a_spri();"
      "    self::a_pub();"
      "    self::a_pro();"
      "    self::a_pri();"
      "  }"
      "  public function a_f() {"
      "    test();"
      "    self::a_spub();"
      "    self::a_spro();"
      "    self::a_spri();"
      "    self::a_pub();"
      "    self::a_pro();"
      "    self::a_pri();"
      "  }"
      "};"
      "class B extends A {"
      "  static public function b_spub() {"
      "    test();"
      "  }"
      "  static protected function b_spro() {"
      "    test();"
      "  }"
      "  static private function b_spri() {"
      "    test();"
      "  }"
      "  public function b_pub() {"
      "    test();"
      "  }"
      "  protected function b_pro() {"
      "    test();"
      "  }"
      "  private function b_pri() {"
      "    test();"
      "  }"
      "  public static function b_sf() {"
      "    test();"
      "    self::b_spub();"
      "    self::b_spro();"
      "    self::b_spri();"
      "    self::b_pub();"
      "    self::b_pro();"
      "    self::b_pri();"
      "  }"
      "  public function b_f() {"
      "    test();"
      "    self::b_spub();"
      "    self::b_spro();"
      "    self::b_spri();"
      "    self::b_pub();"
      "    self::b_pro();"
      "    self::b_pri();"
      "  }"
      "}"
      "EXT::ext_spub();"
      "test();"
           TEST_BODY_FOR_IS_CALLABLE
      "var_dump(is_callable('A::b_spub'));"
      "var_dump(is_callable('A::b_spro'));"
      "var_dump(is_callable('A::b_spri'));"
      "var_dump(is_callable('A::b_pub'));"
      "var_dump(is_callable('A::b_pro'));"
      "var_dump(is_callable('A::b_pri'));"
      "B::a_sf();"
      "B::b_sf();"
      "$obj = new B;"
      "$obj->a_sf();"
      "$obj->b_sf();"
      "$obj->a_f();"
      "$obj->b_f();");

  MVCR("<?php "
       "function foo() { echo 'Caught'; exit; }"
       "set_error_handler('foo', E_ALL);"
       "class X { function foo() { var_dump($this); } }"
       "class Y {"
       "  function bar(X $a) { $a->foo(); }"
       "}"
       "function test($y,$z) {"
       "  $y->$z($y);"
       "}"
       "test(new Y, 'bar');");
  MVCR("<?php\n"
      "/*\n"
      "   some random tests used for debugging fast method call and various "
      " invoke paths\n"
      "   // php53 means this feature cannot be tested under php 5.2\n"
      "*/\n"
      "\n"
      "$fix249639=0; // when this task is fixed or o_id on static calls\n"
      "\n"
      "global $trace;\n"
      "function f2 ($a) { return $a+200; }\n"
      "function f4 ($a) { return $a+400; }\n"
      "class B {\n"
      "  public $id;\n"
      "  public $x;\n"
      "  function __call($name, $arguments) {\n"
      "    // keep f4bogus from fatalling\n"
      "    echo \"Calling B object method '$name' \" . implode(', ', "
      "$arguments). \"\\n\";\n"
      "  }\n"
      "\n"
      "  function f1($a) { return $x=$a+11; }\n"
      "  function f2($a) { return $x=$a+12; }\n"
      "  function f4($a) { return $x=$a+12; }\n"
      "  function trace($s) { global $trace; $trace = \"<$s(\"."
      "$this->id.\")>\"; }\n"
      "  private function f4helper($a) { return $x=$a+12; }\n"
      "}\n"
      "class G  extends B {\n"
      "  public $pointless;\n"
      "  function __call($name, $arguments) {\n"
      "    // keep f4bogus from fatalling\n"
      "    echo \"Calling G object method '$name' \" . "
      "implode(', ', $arguments). \"\\n\";\n"
      "  }\n"
      "  function __construct($i) { $this->id=$i; }\n"
      "  function f($a) { $this->trace(\"G::f\"); return $a; }\n"
      "  function f1($a) { return $a; } // override\n"
      "  function flongerthan8($a,$b,$c) { return $a+$b+$c+1; }\n"
      "  function f4($a) { return B::f4($a); } \n"
      "  // check SimpleFunctionCall::outputCPPParamOrderControlled\n"
      "  /// !m_valid, !m_className.empty() case\n"
      "  // called method must not exist anywhere even though it\n"
      "  // looks like it might\n"
      "  function f4missing($a) { \n"
      "    // check SimpleFunctionCall::outputCPPParamOrderControlled\n"
      "    // !m_valid, !m_className.empty() cases\n"
      "\n"
      "    // m_validClass \n"
      "    echo \"Calling G object 'f4missing' 3 == \"; \n"
      "    //php53 echo parent::f4missing(3),\"\\n\";  // fatals in PHP\n"
      "\n"
      "    // !m_validClass, m_class\n"
      "    $b=\"B\"; // $b::f4(4);   // task 217171\n"
      "    echo \"static parent method B::f4, 16 == \", B::f4(4),\"\\n\"; "
      "  // should work\n"
      "  \n"
      "    // call an non-existant method on the one object via dynamic "
      "class name\n"
      "    //php53 if($fix249639) echo \"Calling G object method 'f4bogus' "
      "5 == \", $b::f4bogus(5); __call, \n"
      "    // call existing method on the one object via dynamic class name\n"
      "    //php53 if($fix249639) echo \"Calling G object method 'f4missing' "
      "5 == \", $b::f4missing(5); \n"
      "    // $b=\"Bbogus\"; $b::f4bogus(6); // report error\n"
      "    // !m_validClass, !m_class // DDD need this test yet \n"
      "    //echo \"missing 3 \", Bbogus::f4bogus(6),\"\\n\"; "
      "// fatals in PHP\n"
      "\n"
      "  } \n"
      "  function f5($a) { return H::f4($a); } // static call\n"
      "}\n"
      "class H {\n"
      "  function f($a) { global $trace; $trace=\"H::f,\"; return \"\"; }\n"
      "  function f3($a) { return \"\"; }\n"
      "  function f4($a) { return $a+12; }\n"
      "  function f7($a) { return \"\"; }\n"
      "}\n"
      "\n"
      "class J {\n"
      "  function __call($name, $arguments) {\n"
      "    echo \"Calling object method '$name' \" . implode(', ', "
      "$arguments). \"\\n\";\n"
      "  }\n"
      "  function f6($a) { return \"\"; }\n"
      "}\n"
      "\n"
      "function error_handler ($errnor, $errstr, $errfile, $errline) {\n"
      "  // Should catch these undefined methods here, but task 333319\n"
      "  // is blocking their being caught.  For now, suppress the PHP error\n"
      "  // so as to match the missing HPHP one.\n"
      "  //echo \"error handler<<<\\n\";\n"
      "  //var_dump($errnor, $errstr, $errfile, $errline) ;\n"
      "  //echo \">>>\\n\";\n"
      "  return true;\n"
      "}\n"
      "// test invoke_builtin_static_method\n"
      "//echo \"bar == \", \n"
      " //    call_user_func_array(array('Normalizer','normalize'),"
      "array(\"bar\")), \"\\n\";\n"
      "\n"
      "$g = new G(5);\n"
      "// test simple function case\n"
      "echo \"600 == \",\n"
      "  call_user_func_array('f2',array(call_user_func_array('f4',array(0))))"
      ", "
      "\"\\n\";\n"
      "\n"
      "// test C::o_invoke, C::o_invoke_few_args, lookup in call_user_func\n"
      "// static method call (in G::f4).\n"
      "echo \"1 1 13 34 12 == \",$g->f(1),\" \", $g->f1(1),\"  \", \n"
      "    $g->f2(1),\" \",$g->flongerthan8(10,11,12,13,14,15,16),\n"
      "    \" \",$g->f4(0),\"\\n\";\n"
      "// check case insensitive\n"
      "echo \"1 1 13 34 12 == \",$g->F(1),\" \", $g->F1(1),\"  \", \n"
      "    $g->F2(1),\" \",$g->Flongerthan8(10,11,12,13,14,15,16),\n"
      "    \" \",$g->F4(0),\"\\n\";\n"
      "\n"
      "// check SimpleFunctionCall::outputCPPParamOrderControlled\n"
      "$prev_handler=set_error_handler(\"error_handler\");\n"
      "$g->f4missing(3);\n"
      "// $b=\"G\"; $b::f4(4); \n"
      "\n"
      "// For those dynamic cases, check:\n"
      "// 1) A call to an existing method\n"
      "// 2) A call to a method which exists, but not in this class (exists "
      "in methodMap)\n"
      "// 3) A call to a method which does not exist anywhere\n"
      "\n"
      "// $func=\"f3\"; echo \"{G::f3}(3) = \",$g->$func(3),\"\\n\";\n"
      "// $func=\"missing\"; echo \"{G::missing}(3) = \",$g->$func(3),\"\\n\";"
      "\n"
      "// tests direct dynamic call \n"
      "$f='f'; $f1='f1';\n"
      "echo \"1 1 == \",$g->{$f}(1),\" \", $g->{$f1}(1),\"\\n\"; \n"
      "echo \"1 1 == \",$g->{'F'}(1),\" \", $g->{$f1}(1),\"\\n\"; \n"
      "\n"
      "$res = call_user_func_array(\"H::f\",array(2)); // ok\n"
      "\n"
      "// tests methodIndexLookup and this variety of dynamic calls\n"
      "// trying to exhause f_call_user_func_array cases\n"
      "$res = call_user_func_array(array($g,'f'),array(20)); // ok\n"
      "echo \"dynamic call \\$g->'f' $trace, 20 == $res\\n\"; \n"
      "$res= call_user_func_array(array($g,'G::f'),array(21)); // G::G::f a "
      "bit wierd\n"
      "echo \"dynamic call \\$g->'G::f' $trace, 21 == $res\\n\";\n"
      "//echo \"dynamic call \\$g->'H::f' $trace, FAIL = \",\n"
      "//      call_user_func_array(array($g,'H::f'),array(22)),\"\\n\"; "
      "// G::H::f better break\n"
      "\n"
      "// Test on static class, dynamic method name, static call\n"
      "$f = 'f1';\n"
      "echo \"31 == \",G::$f(31),\"\\n\"; // G::f exists\n"
      "$f = 'f3';\n"
      "if ($fix249639) echo \"<method not found>(32) == \",G::$f(32),\"\\n\"; "
      "// H::f3 exists, but not G::f3\n"
      "$f = 'missing';\n"
      "if ($fix249639) echo \"<method not found>(33) == \",G::$f(33),\"\\n\"; "
      "// missing does not exist \n"
      "\n"
      "// Test dynamic class, dynamic method name, static call\n"
      "$cls='G';\n"
      "$f = 'f1';\n"
      "\n"
      "//php53 echo \"31 == \",$cls::$f(31),\"\\n\"; // G::f1 exists\n"
      "$f = 'f3';\n"
      "//php53 if ($fix249639) echo \"<method not found>(32) == \","
      "$cls::$f(32),\"\\n\"; // H::f3 exists, but not G::f3\n"
      "$f = 'missing';\n"
      "//php53 if ($fix249639) echo \"<method not found>(33) == \","
      "$cls::$f(33),\"\\n\"; // missing does not exist \n"
      "\n"
      "\n"
      "// test methodIndexLookupReverse\n"
      "echo \"dynamic call \\$g->'missing' $trace, Calling G object method "
      "'missing' 2 = \", call_user_func_array(array($g,'missing'),"
      "array(2)),\"\\n\";\n"
      "echo \"dynamic call 'missing(2)' $trace, FAIL =\", "
      "call_user_func_array('missing',array(2)),\"\\n\";\n"
      "\n"
      "// more __call testing\n"
      "$j = new J();\n"
      "echo \"Calling object method 'missing' 3 = \";\n"
      "call_user_func_array(array($j,'missing'),array(3)); \n"
      "\n"
      "// test mapping for system function names\n"
      "$ourFileName = \"testFile.txt\";\n"
      "$ourFileHandle = fopen($ourFileName, 'w') or die(\"can't open file\");\n"
      "fclose($ourFileHandle);\n"
      "unlink($ourFileName);\n"
      "\n"
      "echo \"done\\n\";");
 return true;
}

bool TestCodeRun::TestClassMethod() {
  MVCR(
    "<?php\n"
    "class Foo {\n"
    "  static function Bar() {\n"
    "    if (isset($this) && isset($this->bar)) {\n"
    "      echo \"isset\\n\";\n"
    "    }\n"
    "    var_dump($this);\n"
    "  }\n"
    "} Foo::Bar(); $obj = new Foo(); $obj->Bar();\n"
  );

  MVCR(
    "<?php\n"
    "class Example {\n"
    "   function whatever() {\n"
    "      if (isset($this)) {\n"
    "          var_dump('static method call');\n"
    "      } else {\n"
    "          var_dump('non-static method call');\n"
    "      }\n"
    "   }\n"
    "}\n"
    "Example::whatever();\n"
    "$inst = new Example();\n"
    "$inst->whatever();\n"
  );

  MVCR("<?php\n"
       "if (true) {\n"
       "  class c extends AppendIterator {}\n"
       "} else {\n"
       "  class c {}\n"
       "}\n"
       "class d extends c {\n"
       "  public function rewind() {\n"
       "    var_dump('rewinding');\n"
       "  }\n"
       "}\n"
       "$obj = new d;\n"
       "foreach ($obj as $k => $v) {}\n");

  MVCR("<?php "
      "$error = 'fatal error';"
      "echo AdsConsoleRenderer::getInstance()->writeMsg('error', $error);"
      "class AdsConsoleRenderer {"
      "  public static function getInstance() {"
      "    return new AdsConsoleRenderer();"
      "  }"
      ""
      "  function writeMsg($classname = '', $s = '') {"
      "    echo $classname . \"::\" . $s;"
      "  }"
      "}");
  MVCR("<?php class A { "
      "  function f() { return \"hello\" ;}"
      "}; "
      "$g = new A(); echo $g->{'f'}();");

  return true;
}

bool TestCodeRun::TestObjectMagicMethod() {
  MVCR("<?php\n"
       "class Test {\n"
       "  public function __call($name, $args) {\n"
       "    var_dump($args);\n"
       "  }\n"
       "}\n"
       "$test = new Test();\n"
       "$test->test();\n"
      );

  MVCR("<?php class A {"
      "  private $foo, $bar; "
      "  function __construct() { $this->foo = 1; $this->bar = 2;} "
      "  public function __sleep() { $this->foo = 3; return array('foo');} "
      "}"
      " $a = new A(); var_dump(serialize($a));");

  MVCR("<?php class A { "
      "  public $a = array(); "
      "  function __set($name, $value) { $this->a[$name] = $value.'set';} "
      "  function __get($name) { return $this->a[$name].'get';} "
      "} "
      "$obj = new A(); $obj->test = 'test'; var_dump($obj->test);");

  MVCR("<?php "
      "class A {"
      "  public $a = 'aaa';"
      "  public function __get($name) { return 'getA';}"
      "}"
      "class B extends A {"
      "  public function __get($name) { return 'getB';}"
      "}"
      "$obj = new A();"
      "var_dump($obj->a);"
      "var_dump($obj->b);"
      "$obj = new B();"
      "var_dump($obj->a);"
      "var_dump($obj->b);");

  MVCR("<?php class A { "
      "  public $a = array(); "
      "  function __set($name, $value) { $this->a[$name] = $value;} "
      "  function __get($name) { return $this->a[$name];} "
      "} "
      "$obj = new A(); $obj->test = 'test'; var_dump($obj->test);");

  MVCR("<?php class A {} "
      "$obj = new A(); $obj->test = 'test'; var_dump($obj->test);");

  MVCR("<?php class A { "
      "function __call($a, $b) { var_dump($a, $b[0], $b[1]);}} "
      "$obj = new A(); $a = 1; $obj->test($a, 'ss');");
  /*
  MVCR("<?php class A { "
      "  function __set($name, $value) { $this->a[$name] = $value;} "
      "  function __get($name) { return $this->a[$name];} "
      "} "
      "$obj = new A(); $obj->test = 'test'; var_dump($obj->test);");
  */

  MVCR("<?php "
      "class foo"
      "{"
      "  public $public = 'public';"
      "  public function __sleep()"
      "  { return array('public'); }"
      "}"
      "$foo = new foo();"
      "$data = serialize($foo);"
      "var_dump($data);");
  MVCR("<?php "
      "class MemberTest {"
      "  private $data = array();"
      "  public function __set($name, $value) {"
      "    echo \"Setting '$name' to '$value'\n\";"
      "    $this->data[$name] = $value;"
      "  }"
      "  public function __get($name) {"
      "    echo \"Getting '$name'\n\";"
      "    if (array_key_exists($name, $this->data)) {"
      "      return $this->data[$name];"
      "    }"
      "    return null;"
      "  }"
      "  public function __unset($name) {"
      "    echo \"Unsetting '$name'\n\";"
      "    unset($this->data[$name]);"
      "    return 1;"
      "  }"
      "}"
      "$obj = new MemberTest;"
      "$obj->a = 1;"
      "echo $obj->a;");
  MVCR("<?php "
      "class foo"
      "{"
      "  public $public = 'public';"
      "  public function __wakeup()"
      "  { echo \"__wakeup called\\n\"; return 1; }"
      "}"
      "$foo = unserialize(\"O:3:\\\"foo\\\":1:{s:6:\\\"public\\\";s:6:\\\"public\\\";}\");"
      "var_dump($foo);");

  MVCR("<?php "
      "class b {"
      "  private $f2 = 10;"
      "  function t2() {"
      "    var_dump($this->f2);"
      "  }"
      "}"
      "class c extends b{"
      "  private $f = 10;"
      "  private static $sf = 10;"
      "  function __get($n) {"
      "    echo 'got';"
      "    return 1;"
      "  }"
      "  function t() {"
      "    var_dump($this->f2);"
      "  }"
      "}"
      "$x = new c;"
      "var_dump($x->f);"
      "$x->t();"
      "$x->t2();");

  MVCR("<?php "
      "class X implements arrayaccess {"
      "function offsetGet($n) { return $n; }"
      "function offsetSet($n, $v) { var_dump($n); }"
      "function offsetExists($n) { var_dump($n); return true; }"
      "function offsetUnset($n) { var_dump($n); }"
      "function __toString() { return 'baz'; }"
      "}"
      ""
      "$a = new X;"
      "echo \"sets\\n\";"
      "$a[true] = 5;"
      "$a[NULL] = 57;"
      "$a[3.2] = 25;"
      "$a[array(3)] = 30;"
      "$a[$a] = 32;"
      "$a['57'] = 5;"
      "$a['6.5'] = 7;"
      ""
      "echo \"gets\\n\";"
      "var_dump($a[true]);"
      "var_dump($a[NULL]);"
      "var_dump($a[3.2]);"
      "var_dump($a[array(3)]);"
      "var_dump($a[$a]);"
      "var_dump($a['57']);"
      "var_dump($a['6.5']);"
      ""
      "echo \"unsets\\n\";"
      "unset($a[true]);"
      "unset($a[NULL]);"
      "unset($a[3.2]);"
      "unset($a[array(3)]);"
      "unset($a[$a]);"
      "unset($a['57']);"
      "unset($a['6.5']);"
      ""
      "echo \"issets\\n\";"
      "isset($a[true]);"
      "isset($a[NULL]);"
      "isset($a[3.2]);"
      "isset($a[array(3)]);"
      "isset($a[$a]);"
      "isset($a['57']);"
      "isset($a['6.5']);");

  MVCR("<?php "
       "class foo"
       "{"
       "  public function __get($n)  { return 'foo'; }"
       "  public function __set($n,$v)  { }"
       "}"
       "$foo = new foo; $a = $foo->x = 'baz'; $b = $foo->x .= 'bar';"
       "var_dump($a,$b);");

  MVCR("<?php\n"
       "class A {\n"
       "  var $a;\n"
       "  function __set($n, $v) { $this->a[$n] = $v; }\n"
       "  function __get($n) { return $this->a[$n]; }\n"
       "  function f() { $this->f = 100; $this->f += 100; }\n"
       "}\n"
       "function test() {\n"
       "  $a = new A();\n"
       "  $a->f();\n"
       "  var_dump($a);\n"
       "}\n"
       "test();\n");

  MVCR("<?php "
       "class X {"
       "  public $real = 1;"
       "  function __get($name) { echo 'get:'; var_dump($name); return 'abc'; }"
       "  function __set($name, $val) { echo 'set:'; var_dump($name,$val); }"
       "}"
       "function test($x) {"
       "  ++$x->foo;"
       "  var_dump($x->bar++);"
       "  $x->real++;"
       "  var_dump($x);"
       "}"
       "test(new X);");

  return true;
}

bool TestCodeRun::TestObjectAssignment() {
  MVCR("<?php "
      "class foo {"
      "  public function __construct() {"
      "    $this->val = 1;"
      "  }"
      "  function bar() {"
      "    echo $this->val;"
      "    $ref = &$this;"
      "    $ref->val = 2;"
      "    echo $this->val;"
      "    $ref2 = $this;"
      "    $ref2->val = 3;"
      "    echo $this->val;"
      "    $x = new foo();"
      "    echo $x->val;"
      "    $ref3 = &$x;"
      "    $ref3->val = 4;"
      "    echo $x->val;"
      "    $ref4 = $x;"
      "    $ref4->val = 5;"
      "    echo $x->val;"
      "  }"
      "  var $val;"
      "}"
      "$x = new foo();"
      "$x->bar();"
      "$ref5 = $x;"
      "$ref5->val = 6;"
      "echo $x->val;"
      "$ref6 = &$x;"
      "$ref6->val = 7;"
      "echo $x->val;");

  return true;
}

bool TestCodeRun::TestNewObjectExpression() {
  MVCR("<?php "
      "class A { var $num; }"
      "function foo() { return new A(); }"
      "foreach(($a=(object)new A()) as $v);"
      "foreach(($a=(object)foo()) as $v);"
      "foreach(($a=new A()) as $v);"
      "$a->num = 1;"
      "print($a->num);");

  MVCR("<?php class A {} class B extends A { "
       "static function foo() { return new self();} } "
       "var_dump(B::foo());");

  MVCR("<?php class A {} class B extends A { "
       "static function foo() { return new parent();} } "
       "var_dump(B::foo());");

  MVCR("<?php "
       "function g() {}"
       "function h() {}"
       "class C {}"
       "new C(g(), h());");

  MVCR("<?php "
       "class X {"
       "  function __construct($a, &$b) {"
       "  }"
       "}"
       "function test($a) {"
       "  $b = 1;"
       "  return new X($a, $b);"
       "}"
       "var_dump(test(3));");

  MVCR("<?php "
      "class A1 {"
      "  public function A1($id) {"
      "    $this->id = $id;"
      "  }"
      "}"
      "class B1 extends A1 {"
      "}"
      "class C1 extends B1 {"
      "  public function __construct($id) {"
      "    parent::__construct($id);"
      "  }"
      "  function zz($id) {"
      "    parent::__construct($id);"
      "  }"
      "}"
      ""
      "$x = new C1(100);"
      "echo $x->id.\"\\n\";"
      "$x->zz(1);"
      "echo $x->id.\"\\n\";");
  MVCR("<?php\n"
      "class A {\n"
      "  function __construct($a) { echo \"A\\n\"; }\n"
      "  function __destruct() { var_dump($this); }\n"
      "}\n"
      "function f() { echo \"f\\n\"; throw new Exception(); }\n"
      "function test() { $a = new A(f()); }\n"
      "try { test(); } catch (Exception $e) { }\n");
  MVCR("<?php "
      "class foo {"
      "  static function ioo($y, &$x) {"
      "    return new self(1);"
      "  }"
      "  function __construct($a, $b) {}"
      "}"
      "function t() {"
      "  $x = 1;"
      "  foo::ioo($x, $y);"
      "}"
      "t();");
  MVCR("<?php "
       "class X {"
       "  function __toString() { return 'hello'; }"
       "}"
       "function f() {"
       "  return 'bar';"
       "}"
       "function test($e) {"
       "  $a = 'foo';"
       "  for ($i = 0; $i < 10; $i++) {"
       "    $a .= new X($e['x']) . f();"
       "  }"
       "  return $a;"
       "}"
       "var_dump(test());");

  return true;
}

bool TestCodeRun::TestObjectPropertyExpression() {
  MVCR("<?php "
      "class test {"
      "  function foo() {"
      "    $var = $this->blah->prop->foo->bar = \"string\";"
      "    var_dump($this->blah);"
      "  }"
      "}"
      "$t = new test;"
      "$t->foo();");
  MVCR("<?php "
      "class C1 {"
      "  public function __get( $what ) {"
      "    echo \"get\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  public function __set( $what, $value ) {"
      "    echo \"set\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "$c = new C1();"
      "$c->a = 1;"
      "$c->a .= 1;"
      "print $c->a;");
  MVCR("<?php "
      "class C1 {"
      "  public function __get( $what ) {"
      "    echo \"get\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  public function __set( $what, $value ) {"
      "    echo \"set\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C2 {"
      "  public $p2;"
      "}"
      "$c2 = new C2();"
      "$c2->p = new C1();"
      "$c2->p->a = 1;"
      "$c2->p->a .= 1;"
      "print $c2->p->a;");
  MVCR("<?php "
      "class C1 {"
      "  public function __get( $what ) {"
      "    echo \"get\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  public function __set( $what, $value ) {"
      "    echo \"set\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C2 {"
      "  public $p2;"
      "}"
      "class C3 {"
      "  public $p3;"
      "}"
      "$c3 = new C3();"
      "$c3->p3 = new C2();"
      "$c3->p3->p2 = new C1();"
      "$c3->p3->p2->a = 1;"
      "$c3->p3->p2->a .= 1;"
      "print $c3->p3->p2->a;");
  MVCR("<?php "
      "class C1 {"
      "  public function __get( $what ) {"
      "    echo \"get C1\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  public function __set( $what, $value ) {"
      "    echo \"set C1\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C2 {"
      "  public function __get( $what ) {"
      "    echo \"get C2\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  public function __set( $what, $value ) {"
      "    echo \"set C2\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C3 {"
      "  public function __get( $what) {"
      "    echo \"get C3\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  public function __set( $what, $value ) {"
      "    echo \"set C3\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "$c3 = new C3();"
      "$c3->p3 = new C2();"
      "$c3->p3->p2 = new C1();"
      "$c3->p3->p2->a = 1;"
      "$c3->p3->p2->a .= 1;"
      "print $c3->p3->p2->a;");
  MVCR("<?php "
      "class C1 {"
      "  protected function __get( $what ) {"
      "    echo \"get C1\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  protected function __set( $what, $value ) {"
      "    echo \"set C1\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C2 {"
      "  protected function __get( $what ) {"
      "    echo \"get C2\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  protected function __set( $what, $value ) {"
      "    echo \"set C2\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C3 {"
      "  protected function __get( $what) {"
      "    echo \"get C3\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  protected function __set( $what, $value ) {"
      "    echo \"set C3\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "$c3 = new C3();"
      "$c3->p3 = new C2();"
      "$c3->p3->p2 = new C1();"
      "$c3->p3->p2->a = 1;"
      "$c3->p3->p2->a .= 1;"
      "print $c3->p3->p2->a;");
  MVCR("<?php "
      "class C1 {"
      "  private function __get( $what ) {"
      "    echo \"get C1\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  private function __set( $what, $value ) {"
      "    echo \"set C1\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C2 {"
      "  private function __get( $what ) {"
      "    echo \"get C2\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  private function __set( $what, $value ) {"
      "    echo \"set C2\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C3 {"
      "  private function __get( $what) {"
      "    echo \"get C3\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  private function __set( $what, $value ) {"
      "    echo \"set C3\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "$c3 = new C3();"
      "$c3->p3 = new C2();"
      "$c3->p3->p2 = new C1();"
      "$c3->p3->p2->a = 1;"
      "$c3->p3->p2->a .= 1;"
      "print $c3->p3->p2->a;");
  MVCR("<?php "
      "class C1 {"
      "  public function __get( $what ) {"
      "    echo \"get\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  public function __set( $what, $value ) {"
      "    echo \"set\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "$c = new C1();"
      "$c->a += 1;"
      "print $c->a;"
      "$c->a += 10;"
      "print $c->a;"
      "$c->a -= 2;"
      "print $c->a;"
      "$c->a *= 3;"
      "print $c->a;"
      "$c->a /= 2;"
      "print $c->a;"
      "$c->a %= 8;"
      "print $c->a;"
      "$c->a <<= 3;"
      "print $c->a;"
      "$c->a >>= 2;"
      "print $c->a;"
      "$c->a ^= 18;"
      "print $c->a;"
      "$c->a &= 333;"
      "print $c->a;"
      "$c->a |= 7;"
      "print $c->a;");
  MVCR("<?php "
      "class C1 {"
      "}"
      "class C2 {"
      "  private function __get( $what ) {"
      "    echo \"get C2\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  private function __set( $what, $value ) {"
      "    echo \"set C2\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C3 {"
      "  private function __get( $what ) {"
      "    echo \"get C3\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  private function __set( $what, $value ) {"
      "    echo \"set C3\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "function assign_ref(&$v) {"
      "  $v = 22;"
      "}"
      "$c3 = new C3();"
      "$c3->p3 = new C2();"
      "$c3->p3->p2 = new C1();"
      "$c3->p3->p2->a = 1;"
      "$c3->p3->p2->a .= 1;"
      "print $c3->p3->p2->a;"
      "assign_ref($c3->p3->p2->a);"
      "print $c3->p3->p2->a;");
  MVCR("<?php "
      "class C1 {"
      "  private function __get( $what ) {"
      "    echo \"get C1\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  private function __set( $what, $value ) {"
      "    echo \"set C1\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C2 {"
      "  private function __get( $what ) {"
      "    echo \"get C2\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  private function __set( $what, $value ) {"
      "    echo \"set C2\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "class C3 {"
      "  private function __get( $what ) {"
      "    echo \"get C3\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  private function __set( $what, $value ) {"
      "    echo \"set C3\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "function assign_ref(&$v) {"
      "  $v = 22;"
      "}"
      "$c3 = new C3();"
      "$c3->p3 = new C2();"
      "$c3->p3->p2 = new C1();"
      "$c3->p3->p2->a = 1;"
      "$c3->p3->p2->a .= 1;"
      "print $c3->p3->p2->a;"
      "assign_ref($c3->p3->p2->a);"
      "print $c3->p3->p2->a;");
  MVCR("<?php "
      "$b = 10;"
      "class C1 {"
      "  public function __get( $what ) {"
      "    global $b;"
      "    return $b;"
      "  }"
      "}"
      "$c1 = new C1();"
      "function assign_ref(&$lv) {"
      "  $lv = 8;"
      "}"
      "assign_ref($c1->a);"
      "var_dump($b);");
  MVCR("<?php "
      "class C1 {"
      "  public function __get( $what ) {"
      "    echo \"get C1\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  public function __set( $what, $value ) {"
      "    echo \"set C1\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "$c1 = new C1();"
      "$c1->a = new C1();"
      "$c1->a->b = new C1();"
      "$c1->a->b->c = 10;"
      "var_dump($c1->a->b->c);"
      "$c1->a->b->c .= 10;"
      "var_dump($c1->a->b->c);");
  MVCR("<?php "
      "class C1 {"
      "  public function __get( $what ) {"
      "    echo \"get C1\n\";"
      "    return $this->_p[ $what ];"
      "  }"
      "  public function __set( $what, $value ) {"
      "    echo \"set C1\n\";"
      "    $this->_p[ $what ] = $value;"
      "  }"
      "  private $_p = array();"
      "}"
      "$c1 = new C1();"
      "$c1->a = new C1();"
      "$c1->a->b = new C1();"
      "for ($i = 0; $i < 2048; $i++) {"
      "  $c1->a->b->c = 10;"
      "}"
      "var_dump($c1->a->b->c);");
  MVCR("<?php "
       "class Y {}"
       "class X {"
       "  public $a;"
       "  function __construct() {"
       "    $this->a = array('x' => new Y);"
       "  }"
       "  function bar() {"
       "    var_dump('bar');"
       "    $this->qq = new Y;"
       "    $this->qq->x = $this->qq->y = 1;"
       "    return $this->qq;"
       "  }"
       "}"
       "function foo() { var_dump('foo'); return 'foo'; }"
       "function test($x, $a, $s) {"
       "  $t = &$s->t;"
       "  unset($x->bar()->x);"
       "  unset($x->q->r->s->${foo()});"
       "  unset($x->y->a->b->c);"
       "  unset($x->a['x']->y->a->b->c);"
       "  unset($a['a']['y'][foo()]);"
       "  unset($a['b']->y->z);"
       "  unset($a->c->d);"
       "  var_dump($x, $a, $s);"
       "}"
       "test(new X, array(), false);");

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// comparisons

#define COMPARE(a, op, b)                                       \
  "print ++$i; print \"\\t\"; "                                 \
  "print (" #a #op #b ") ? 'Y' : 'N'; "                         \
  "$a = 1; $a = 't'; $a = " #a "; "                             \
  "print ($a " #op #b ") ? 'Y' : 'N'; "                         \
  "$b = 1; $b = 't'; $b = " #b "; "                             \
  "print (" #a #op "$b) ? 'Y' : 'N'; "                          \
  "print ($a " #op "$b) ? 'Y' : 'N'; "                          \
  "print \"\\t\"; "                                             \
  "print \"" #a " " #op " " #b "\t\"; "                         \
  "print \"\\n\"; "                                             \

#define COMPARE_ALL(a, op)                      \
  COMPARE(a, op, true)                          \
    COMPARE(a, op, false)                       \
    COMPARE(a, op, 1)                           \
    COMPARE(a, op, 0)                           \
    COMPARE(a, op, -1)                          \
    COMPARE(a, op, '1')                         \
    COMPARE(a, op, '0')                         \
    COMPARE(a, op, '-1')                        \
    COMPARE(a, op, NULL)                        \
    COMPARE(a, op, array())                     \
    COMPARE(a, op, array(1))                    \
    COMPARE(a, op, array(2))                    \
    COMPARE(a, op, array('1'))                  \
    COMPARE(a, op, array('0' => '1'))           \
    COMPARE(a, op, array('a'))                  \
    COMPARE(a, op, array('a' => 1))             \
    COMPARE(a, op, array('b' => 1))             \
    COMPARE(a, op, array('a' => 1, 'b' => 2))   \
    COMPARE(a, op, array(array('a' => 1)))      \
    COMPARE(a, op, array(array('b' => 1)))      \
    COMPARE(a, op, 'php')                       \
    COMPARE(a, op, '')                          \

#define COMPARE_OP(op)                                                  \
  MVCR("<?php $i = 0; " COMPARE_ALL('1.2', op));                        \
  MVCR("<?php $i = 0; " COMPARE_ALL(true, op));                         \
  MVCR("<?php $i = 0; " COMPARE_ALL(false, op));                        \
  MVCR("<?php $i = 0; " COMPARE_ALL(1, op));                            \
  MVCR("<?php $i = 0; " COMPARE_ALL(0, op));                            \
  MVCR("<?php $i = 0; " COMPARE_ALL(-1, op));                           \
  MVCR("<?php $i = 0; " COMPARE_ALL('1', op));                          \
  MVCR("<?php $i = 0; " COMPARE_ALL('0', op));                          \
  MVCR("<?php $i = 0; " COMPARE_ALL('-1', op));                         \
  MVCR("<?php $i = 0; " COMPARE_ALL(NULL, op));                         \
  MVCR("<?php $i = 0; " COMPARE_ALL(array(), op));                      \
  MVCR("<?php $i = 0; " COMPARE_ALL(array(1), op));                     \
  MVCR("<?php $i = 0; " COMPARE_ALL(array(2), op));                     \
  MVCR("<?php $i = 0; " COMPARE_ALL(array('1'), op));                   \
  MVCR("<?php $i = 0; " COMPARE_ALL(array('0' => '1'), op));            \
  MVCR("<?php $i = 0; " COMPARE_ALL(array('a'), op));                   \
  MVCR("<?php $i = 0; " COMPARE_ALL(array('a' => 1), op));              \
  MVCR("<?php $i = 0; " COMPARE_ALL(array('b' => 1), op));              \
  MVCR("<?php $i = 0; " COMPARE_ALL(array('a' => 1, 'b' => 2), op));    \
  MVCR("<?php $i = 0; " COMPARE_ALL(array(array('a' => 1)), op));       \
  MVCR("<?php $i = 0; " COMPARE_ALL(array(array('b' => 1)), op));       \
  MVCR("<?php $i = 0; " COMPARE_ALL('php', op));                        \
  MVCR("<?php $i = 0; " COMPARE_ALL('', op));                           \

bool TestCodeRun::TestComparisons() {
  MVCR("<?php var_dump(array(1 => 1, 2 => 1) ==  array(2 => 1, 1 => 1));");
  MVCR("<?php var_dump(array(1 => 1, 2 => 1) === array(2 => 1, 1 => 1));");
  MVCR("<?php var_dump(array('a'=>1,'b'=> 1) ==  array('b'=>1,'a'=> 1));");
  MVCR("<?php var_dump(array('a'=>1,'b'=> 1) === array('b'=>1,'a'=> 1));");

  MVCR("<?php $a = '05/17'; $b = '05/18'; var_dump($a == $b);");
  MVCR("<?php var_dump('05/17' == '05/18');");
  MVCR("<?php var_dump('1.0' == '1');"
       "var_dump('1.0E2' == '10E1');"
       "var_dump('1' === '1');"
       "var_dump('1.0' === '1.0');"
       "var_dump('1' === '1.0');"
       "var_dump('1.0' === '1.00');"
       "var_dump(1.0 === 1.00);"
       "var_dump('1' == '1');"
       "var_dump('1.0' == '1.0');"
       "var_dump('1' == '1.0');"
       "var_dump('1.0' == '1.00');"
       "var_dump(1.0 == 1.00);"
       "function foo($a, $b) {"
       "  $s = (string)$a;"
       "  $t = (string)$b;"
       "  var_dump($s === $t);"
       "}"
       "foo('1.00', '1.0');"
       "foo('1.0', '1.0');"
       "foo('1.', '1.0');"
       "foo('1', '1.0');");

  COMPARE_OP(==);
  COMPARE_OP(===);
  COMPARE_OP(!=);
  COMPARE_OP(<>);
  COMPARE_OP(!==);
  COMPARE_OP(<);
  COMPARE_OP(>);
  COMPARE_OP(<=);
  COMPARE_OP(>=);

  MVCR("<?php "
      "class c {"
      "  public $x = 0;"
      "}"
      "$x = new c;"
      "$x->x = 1;"
      "$y = new c;"
      "var_dump($x > $y);"
      "var_dump(array($x) == array($y));");
  MVCR("<?php "
       "function foo($p) {"
       "  if ($p) {"
       "    $a = 'foo';"
       "  }"
       "  if ('' < $a) {"
       "    echo 'yes';"
       "  } else {"
       "    echo 'no';"
       "  }"
       "  if ($a > '') {"
       "    echo 'yes';"
       "  } else {"
       "    echo 'no';"
       "  }"
       "}"
       "foo(false);");
  MVCR("<?php "
       "$part = ' 1';"
       "var_dump(trim($part) !== $part);");
  MVCR("<?php "
       "class C { }"
       "function foo($p) {"
       "  if ($p) {"
       "    $obj = new C;"
       "  } else {"
       "    $a = array(1);"
       "  }"
       "  var_dump($obj == $a);"
       "}"
       "foo(false);");
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// semantics

bool TestCodeRun::TestUnset() {
  MVCR("<?php class A { static $foo = array(123);} $a = 'A'; "
       "unset($a::$foo[0]); unset(A::$foo[0]);");

  MVCR("<?php $a = 10; unset($a); var_dump($a);");
  MVCR("<?php $a = array(10); "
      "function test() { global $a; unset($a[0]); var_dump($a);}"
      "var_dump($a); test(); var_dump($a);");
  MVCR("<?php $a = 10; unset($GLOBALS); var_dump($a);");
  MVCR("<?php "
      "function f1() {"
      "  $x = array(1,2,3);"
      "  unset($x[0]);"
      "  var_dump($x);"
      "}"
      "function f2() {"
      "  $x = array(1,2,3);"
      "  unset($x[0][0]);"
      "  var_dump($x);"
      "}"
      "function f3() {"
      "  $x = array(array(4,5,6),2,3);"
      "  unset($x[0][0]);"
      "  var_dump($x);"
      "}"
      "function f4() {"
      "  $x = array(array(4,5,6),2,3);"
      "  unset($x[0][0][0]);"
      "  var_dump($x);"
      "}"
      "f1();"
      "f2();"
      "f3();"
      "f4();");
  MVCR("<?php class A { public $arr;} $obj = new A; $obj->arr[] = 'test';"
      "var_dump($obj->arr); unset($obj->arr); var_dump($obj->arr);");
  MVCR("<?php function test() {$a=array(1,2,3); unset($a[0]);}");
  MVCR("<?php "
      "function foo() {"
      "  $a = 1;"
      "  $b = 2;"
      "  $c = 3;"
      "  unset($a, $b, $c);"
      "  var_dump($b);"
      "}"
      "foo();");
  MVCR("<?php "
      "$a = 1;"
      "function foo() {"
      "  $GLOBALS['foo'] = 1;"
      "  unset($GLOBALS['foo']);"
      "  var_dump(array_key_exists('foo', $GLOBALS));"
      "  $g['foo'] = 1;"
      "  unset($g['foo']);"
      "  var_dump(array_key_exists('foo', $g));"
      "  var_dump(array_key_exists('a', $GLOBALS));"
      "  unset($GLOBALS['a']);"
      "  var_dump(array_key_exists('a', $GLOBALS));"
      "}"
      "foo();");
  MVCR("<?php "
      "class A {"
      "  var $a;"
      "  public function __construct($p) {"
      "    $this->a = $p;"
      "  }"
      "};"
      "$obj = new A(1);"
      "var_dump($obj);"
      "unset($obj->a);"
      "var_dump($obj);"
      "$obj->a = 2;"
      "var_dump($obj);"
      "$obj->b = 3;"
      "var_dump($obj);"
      "unset($obj->b);"
      "var_dump($obj);"
      "$obj->a = 2;"
      "var_dump($obj);"
      "$obj->b = 3;"
      "var_dump($obj);"
      "unset($obj->a, $obj->b);"
      "var_dump($obj);");
  MVCR("<?php "
       "class X {"
       "  function __construct() { echo 'construct\n'; }"
       "  function __destruct() { echo 'destruct\n'; }"
       "}"
       "function test() {"
       "  $a = new X;"
       "  echo 'before unset\n';"
       "  unset($a);"
       "  echo 'after unset\n';"
       "}"
       "test();");

  MVCR("<?php "
       "function foo() { return 42; }"
       "$a = foo();"
       "var_dump((unset)foo());"
       "var_dump((unset)$a);"
       "var_dump($a);");

  return true;
}

bool TestCodeRun::TestReference() {
  MVCR("<?php $idxa = array('a' => 1240430476);"
      "$idxa = &$idxa['a'];");

  MVCR("<?php $a = array(1, 'a'); $b = $a; "
      "foreach ($b as $k => &$v) { $v = 'ok';} var_dump($a, $b);");

  MVCR("<?php $a = array(1, 'test'); $b = $a; $c = &$b[0]; "
      "$c = 10; var_dump($a, $b);");

  // reference expressions
  MVCR("<?php $a = &$b; $a = 10; var_dump($b);");
  MVCR("<?php $a = 1; $b = $a;  $a = 2; var_dump($b);");
  MVCR("<?php $a = 1; $b = &$a; $c = $b; $a = 2; var_dump($b); var_dump($c);");
  MVCR("<?php $a = 1; $b = &$a; $b = 2; var_dump($a);");
  MVCR("<?php $a = 1; $b = &$a; $c = $b; $b = 2; var_dump($a); var_dump($c);");
  MVCR("<?php $a = 1; $c = $b = &$a; $b = 2; var_dump($a); var_dump($c);");
  MVCR("<?php $a = 1; $b = &$a; $c = 2; $b = $c; $c = 5; "
      "var_dump($a); var_dump($b); var_dump($c);");
  MVCR("<?php $a = 1; $b = &$a; $c = 2; $d = &$c; $b = $d; "
      "var_dump($a); var_dump($b); var_dump($c); var_dump($d);");
  MVCR("<?php $a = 1; $b = &$a; $c = 2; $d = &$c; $b = &$d; "
      "var_dump($a); var_dump($b); var_dump($c); var_dump($d);");
  MVCR("<?php $a = 10; $b = array(&$a); var_dump($b); "
      "$a = 20; var_dump($b);");
  MVCR("<?php $a = array(); $b = 10; $a[] = &$b; $b = 20; var_dump($a);");
  MVCR("<?php $a = 10; $b = array('test' => &$a); var_dump($b); "
      "$a = 20; var_dump($b);");
  MVCR("<?php $a = array(); $b = 1; $a['t'] = &$b; $b = 2; var_dump($a);");
  MVCR("<?php $a = array(1, 2); foreach ($a as $b) { $b++;} var_dump($a);");
  MVCR("<?php $a = array(1, 2); foreach ($a as &$b) { $b++;} var_dump($a);");
  MVCR("<?php $a = array(1, array(2,3)); "
      "foreach ($a[1] as &$b) { $b++;} var_dump($a);");

  // reference parameters
  MVCR("<?php function f(&$a) { $a = 'ok';} "
      "$a = 10; f($a); var_dump($a);");
  MVCR("<?php function f(&$a) { $a = 'ok';} "
      "$a = array(); $c = &$a['b']; $c = 'ok'; var_dump($a);");
  MVCR("<?php function f(&$a) { $a = 'ok';} "
      "$a = array(); $c = &$a['b']; f($c); var_dump($a);");
  MVCR("<?php function f(&$a) { $a = 'ok';} "
      "$a = array(); f($a['b']); var_dump($a);");
  MVCR("<?php function f(&$a) { $a = 'ok';} class T { public $b = 10;} "
      "$a = new T(); $a->b = 10; f($a->b); var_dump($a);");
  MVCR("<?php function f(&$a) { $a = 'ok';} class T {} "
      "$a = new T(); $a->b = 10; f($a->b); var_dump($a);");
  MVCR("<?php function f(&$a) {} "
      "$a = array(); f($a['b']); var_dump($a);");
  MVCR("<?php function f(&$a) {} class T {} "
      "$a = new T(); $a->b = 10; f($a->b); var_dump($a);");

  // reference returns
  MVCR("<?php $a = 10; function &f() { global $a; return $a;} "
      "$b = &f(); $b = 20; var_dump($a);");
  MVCR("<?php function &f() { $a = 10; return $a;} "
      "$b = &f(); $b = 20; var_dump($b);");
  MVCR("<?php $a = array(); function &f() { global $a; return $a['b'];} "
      "$b = &f(); $b = 20; var_dump($a);");
  MVCR("<?php function &f() { $a = array(); return $a['b'];} "
      "$b = &f(); $b = 20; var_dump($b);");

  // circular references
  //VCR("<?php $a = array('a' => &$a); var_dump($a);");
  //VCR("<?php $a = array('a' => &$a); $b = array($a); var_dump($b);");
  MVCR("<?php\n"
       "$a1 = array(&$a1, 1); $a2 = $a1; unset($a1);\n"
       "$a2[0][] = 2;\n"
       "var_dump($a2[0][0][0][2]);\n");

  // shallow copy of members (either of arrays or objects)
  MVCR("<?php function test($a) { $a[1] = 10; $a['r'] = 20;} "
      "$b = 5; $a = array('r' => &$b); $a['r'] = 6; test($a); var_dump($a);");

  MVCR("<?php "
      "$a = array('a'=>0);"
      "$ref = &$a['a'];"
      "var_dump($a);"
      "$b = $a;"
      "var_dump($a,$b);"
      "$b['a'] = 1;"
      "var_dump($a,$b);"
      "$a = array(0);"
      "$ref = &$a[0];"
      "var_dump($a);"
      "$b = $a;"
      "var_dump($a,$b);"
      "$b[0] = 1;"
      "var_dump($a,$b);"
      );

  // reference argument
  MVCRNW("<?php "
        "function foo($u, $v, $w) {"
        "  $u = 10;"
        "  $v = 20;"
        "  $w = 20;"
        "}"
        "$u = 1;"
        "$v = 2;"
        "$w = 3;"
        "foo(&$u, &$v, $w);"
        "var_dump($u, $v, $w);");

  // reference self assignment
  MVCR("<?php "
       "class A {"
       "  function t($a, $arr) {"
       "  }"
       "}"
       "class B extends A {"
       "  function t($a, $arr) {"
       "    var_dump($a);"
       "    $arr['hello'] = $a;"
       "    var_dump($a);"
       "  }"
       "}"
       "function f() {"
       "  return new B();"
       "}"
       "function test() {"
       "  $v = 100;"
       "  $arr['hello'] = $v;"
       "  $a = new B();"
       "  $a->t($arr['hello'], $arr);"
       "}"
       "test();"
       "$arr = array('hello' => 1);"
       "$x = &$arr['hello'];"
       "$arr['hello'] = $x;"
       "var_dump($arr);"
       "function test2(&$a, $b) {"
       "  $a = $b;"
       "}"
       "$v = 10;"
       "test2($v, $v);"
       "var_dump($v);");

  // reference, parameter & assignment
  MVCR("<?php "
      "function test(&$some_ref) {"
      "  $some_ref = 42;"
      "}"
      "test($some_ref = 1);"
      "var_dump($some_ref);"
      "$var = null;"
      "test($var);"
      "var_dump($var);"
      "$var = null;"
      "test($some_ref = $var);"
      "var_dump($some_ref, $var);"
      "$var = null;"
      "test($some_ref = &$var);"
      "var_dump($some_ref, $var);"
      "function test2($some_ref) {"
      "  $some_ref = 42;"
      "}"
      "test2($some_ref = 1);"
      "var_dump($some_ref);"
      "$var = null;"
      "test2($var);"
      "var_dump($var);"
      "$var = null;"
      "test2($some_ref = $var);"
      "var_dump($some_ref, $var);"
      "$var = null;"
      "test2($some_ref = &$var);"
      "var_dump($some_ref, $var);");

  MVCR("<?php "
       "$foo = 123;"
       "function &baz() {"
       "  global $foo;"
       "  return $foo;"
       "}"
       "function bar() {"
       "  $baz = 'baz';"
       "  return $baz();"
       "}"
       "function buz() {"
       "  global $foo;"
       "  return ($foo);"
       "}"
       "$a = &bar();"
       "$a = 456;"
       "var_dump($a, $foo);"
       "$a = &buz();"
       "$a = 789;"
       "var_dump($a, $foo);");

  MVCR("<?php "
       "function &test(&$x) {"
       "  $x = 1;"
       "  return $x;"
       "}"
       "$x = 0;"
       "$y = &test($x);"
       "$y++;"
       "var_dump($x, $y);");

  MVCR("<?php "
       "function foo(&$a) { var_dump($a++); }"
       "function test() {"
       "  foo($a = 6);"
       "  $a = null;"
       "  foo($b += 5);"
       "  $b = null;"
       "  foo($c -= 5);"
       "  $c = null;"
       "  $e = 0;"
       "  foo(++$e);"
       "  $e = 5;"
       "  $g = 0;"
       "  foo(--$g);"
       "  $g = 7;"
       "  $h = null;"
       "  foo($h += 5);"
       "  $h = null;"
       "  foo($h -= 5);"
       "  $h = null;"
       "}"
       "test();");

  MVCR("<?php "
       "function foo() {"
       "  $perms = array('x' => 1);"
       "  $t = &$perms;"
       "  $t = $t['x'];"
       "  unset($t);"
       "  return $perms;"
       "}"
       "var_dump(foo());");

  MVCRO("<?php "
        "$a = array();"
        "$a[0][0] = $a;"
        "var_dump($a);"
        "$b[0][0] = $b;"
        "var_dump($b);"
        "$c[0] = $c;"
        "var_dump($c);"
        ,
        "array(1) {\n"
        "  [0]=>\n"
        "  array(1) {\n"
        "    [0]=>\n"
        "    array(0) {\n"
        "    }\n"
        "  }\n"
        "}\n"
        "array(1) {\n"
        "  [0]=>\n"
        "  array(1) {\n"
        "    [0]=>\n"
        "    NULL\n"
        "  }\n"
        "}\n"
        "array(1) {\n"
        "  [0]=>\n"
        "  NULL\n"
        "}\n");

  MVCR("<?php "
       "function foo($x) {"
       "  var_dump($x[0]->foo['a']['b']);"
       "  $x[0]->foo['a']['b'] = 5;"
       "  var_dump($x);"
       "}"
       "foo(false);"
       "function baz(&$x) {}"
       "foreach ($x->foo[1]->prop as &$y) {}"
       "var_dump($x);"
       "baz($q->foo[1]->prop);"
       "var_dump($q);"
       "$y = &$z->foo[1]->prop;"
       "var_dump($z);"
       "function &fiz(&$x) {"
       "  return $x->foo[1]->prop;"
       "}"
       "fiz($w);"
       "var_dump($w);");

  // https://www.intern.facebook.com/intern/tasks/?t=169731
  MVCR("<?php "
       "$x = 0;\n"
       "$foo0 = isset($g) ? \"ref\" : \"val\";\n"
       "$foo1 = isset($g) ? \"val\" : \"ref\";\n"
       "function ref(&$a, $b) { echo \"$a $b\"; }\n"
       "function val($a, $b)  { echo \"$a $b\"; }\n"
       "$foo0($x, $x = 5);\n"
       "$foo1($x, $x = 5);\n");

 return true;
}

bool TestCodeRun::TestDynamicConstants() {
  MVCR("<?php function foo($a) { return $a + 10;} define('TEST', foo(10)); "
      "var_dump(TEST);");
  MVCR("<?php function foo() { return 15;} "
      "var_dump(TEST); define('TEST', foo()); var_dump(TEST);");
  MVCR("<?php if (true) define('TEST', 1); else define('TEST', 2); "
      "var_dump(TEST);");
  MVCR("<?php var_dump(TEST); define('TEST', 1); var_dump(TEST); "
      "define('TEST', 2); var_dump(TEST);");
  MVCR("<?php if (false) define('TEST', 1); else define('TEST', 2); "
      "var_dump(TEST);");
  MVCR("<?php var_dump(defined('TEST')); var_dump(TEST);"
      "define('TEST', 13);"
      "var_dump(defined('TEST')); var_dump(TEST);");
  MVCR("<?php define('FOO', BAR); define('BAR', FOO); echo FOO; echo BAR;");
  MVCR("<?php define('A', 10); class T { static $a = array(A); } "
      "define('A', 20); var_dump(T::$a);");

  return true;
}

bool TestCodeRun::TestDynamicVariables() {
  // r-value
  MVCR("<?php $a = 1; function t() { global $a;$b = 'a'; var_dump($$b);} t();");
  MVCR("<?php $a = 1; function t() { $b = 'a'; var_dump($$b);} t();");
  MVCR("<?php function t() { $a = 'test'; $b = 'a'; var_dump($$b);} t();");
  MVCR("<?php $a = 'test'; $b = 'a'; var_dump($$b);");
  MVCR("<?php $a = 1; class A { public function t() { global $a; $b = 'a'; var_dump($$b);}} $obj = new A(); $obj->t();");

  // l-value
  MVCR("<?php $a = 'test'; $b = 'a'; $$b = 'ok'; var_dump($a);");
  MVCR("<?php $a = 'test'; $b = 'a'; $$b = 10; var_dump($a);");
  MVCR("<?php $a = 'd'; var_dump($$a); $$a = 10; var_dump($$a); var_dump($d);");

  // ref-value
  MVCR("<?php $a = 'test'; $b = 'a'; $c = &$$b; $c = 10; var_dump($a);");

  // extract
  MVCR("<?php extract(array('a' => 'aval')); var_dump($a);");
  MVCR("<?php extract(array('a' => 'ok')); $a = 1; var_dump($a);");
  MVCR("<?php $a = 1; extract(array('a' => 'ok'), EXTR_SKIP); var_dump($a);");
  MVCR("<?php $a = 1; extract(array('a' => 'ok'), EXTR_PREFIX_SAME, 'p');"
      " var_dump($p_a);");
  MVCR("<?php extract(array('a' => 'ok'), EXTR_PREFIX_ALL, 'p');"
      " var_dump($p_a);");
  MVCR("<?php extract(array('ok'), EXTR_PREFIX_INVALID, 'p'); var_dump($p_0);");
  MVCR("<?php $a = null; extract(array('a' => 'ok'), EXTR_IF_EXISTS); var_dump($a);");
  MVCR("<?php $a = null; extract(array('a' => 'ok', 'b' => 'no'), EXTR_PREFIX_IF_EXISTS, 'p'); var_dump($p_a); var_dump($b); var_dump($p_b);");
  MVCR("<?php $a = 'ok'; extract(array('b' => &$a), EXTR_REFS); $b = 'no'; var_dump($a);");
  MVCR("<?php $a = 'ok'; $arr = array('b' => &$a); extract($arr, EXTR_REFS); $b = 'no'; var_dump($a);");
  MVCR("<?php\n"
      "function f() {\n"
      "  $arr = array(1 => 2, '1d' => 3);\n"
      "  extract($arr);\n"
      "  var_dump(get_defined_vars());\n"
      "}\n"
      "f();\n");

  MVCR("<?php\n"
       "$a = 123;\n"
       "$b = 456;\n"
       "function foo() {\n"
       "  global $a;\n"
       "  $b = &$GLOBALS['b'];\n"
       "  $d = 789; $e = 111;\n"
       "  $c = &$d;\n"
       "  $arr = get_defined_vars(); var_dump($arr); return $arr;\n"
       "}\n"
       "function bar($arr) {\n"
       "  extract($arr, EXTR_REFS);\n"
       "  var_dump($a, $b, $c, $d, $e);\n"
       "  $a = 'aaa'; $b = 'bbb'; $c = 'ccc';\n"
       "  var_dump($d);\n"
       "}"
       "bar(foo());\n"
       "var_dump($a, $b);\n"
      );

  // compact
  MVCR("<?php function test() { $a = 10; $b = 'test'; "
      "  var_dump(compact('ab')); "
      "  var_dump(compact('a', 'ab', 'b')); "
      "  var_dump(compact('a', array('ab', 'b')));"
      "} test(); ");

  MVCR("<?php\n"
       "function f() { return true; }\n"
       "function test() {\n"
       "  $a = 100;\n"
       "  if (compact('a', 'b')) { }\n"
       "  var_dump(compact('a', 'b'));\n"
       "  if (f()) $b = 1; else $b = new Exception();\n"
       "  return $b;\n"
       "}\n"
       "test();\n");

  MVCR("<?php "
       "function f() { return 3; }"
       "function test($f) {"
       "  $x = $f();"
       "  return compact('x');"
       "}"
       "var_dump(test('f'));");

  // get_defined_vars
  MVCR("<?php\n"
       "function simple_getdefined_test() {\n"
       "  $a = 'a';\n"
       "  var_dump(get_defined_vars());\n"
       "}\n"
       "simple_getdefined_test();\n"
       );
  MVCR("<?php "
      "function foo() {"
      "  static $b = 20;"
      "  global $d;"
      "  $a = 10;"
      "  $b = 'c';"
      "  $$b = 20;"
      "  $gdv = get_defined_vars();"
      "  var_dump(isset($gdv['a']) && $gdv['a'] === 10);"
      "  var_dump(isset($gdv['b']) && $gdv['b'] === 'c');"
      "  var_dump(isset($gdv['c']) && $gdv['c'] === 20);"
      "  var_dump(isset($gdv['d']) && $gdv['d'] === 2.1);"
      "}"
      "$d = 2.1;"
      "foo();"
      "var_dump(isset($ggdv['argc']));"
      "var_dump(isset($ggdv['argv']));"
      "var_dump(isset($ggdv['_SERVER']));"
      "var_dump(isset($ggdv['_GET']));"
      "var_dump(isset($ggdv['_POST']));"
      "var_dump(isset($ggdv['_COOKIE']));"
      "var_dump(isset($ggdv['_FILES']));"
      "var_dump(isset($ggdv['d']));"
      );

  MVCR("<?php "
       "function foo(array $test) {"
       "  foreach ($test AS $var) {"
       "    global $$var;"
       "    $$var = $var . 'foo';"
       "  }"
       "}"
       "foo(array('a', 'b'));"
       "var_dump($a, $b);");

  MVCR("<?php "
       "function test($a) {"
       "  $b = 5;"
       "  global $$a;"
       "  var_dump($b);"
       "}"
       "test('b');");

  return true;
}

bool TestCodeRun::TestDynamicProperties() {
  MVCR("<?php class A { public $a = 1;} "
      "class B extends A { "
      "  public $m = 10;"
      "  public function test() { "
      "    $b = 'a';"
      "    $this->$b = 'test'; var_dump($this->$b); var_dump($this->a);"
      "    $c = &$this->$b; $c = array(1); var_dump($this->a);"
      "  }"
      "} $obj = new B(); $obj->test();");

  MVCR("<?php class A { public $a = 1;} class B { public $a = 2;} "
      "$obj = 1; $obj = new A(); var_dump($obj->a);");

  MVCR("<?php "
      "class A { } "
      "function f(&$a) { $a = 1000; } "
      "$a = new A(); $f = 10; $a->$f = 100; var_dump($a); "
      "var_dump((array)$a); "
      "$f = 100; "
      "f($a->$f); "
      "foreach ($a as $k => &$v) { var_dump($k); $v = 1; } "
      "var_dump($a); ");
  return true;
}

bool TestCodeRun::TestDynamicFunctions() {
  MVCR("<?php function test() { print 'ok';} $a = 'Test'; $a();");
  MVCR("<?php function test($a) { print $a;} $a = 'Test'; $a('ok');");
  MVCR("<?php function test($a, $b) { print $a.$b;} $a = 'Test'; $a('o','k');");

  MVCR("<?php function t($a = 'k') { print $a;} "
      "$a = 'T'; $a(); $a('o');");
  MVCR("<?php function t($a, $b = 'k') { print $a.$b;} "
      "$a = 'T'; $a('o'); $a('o', 'p');");
  MVCR("<?php function t($a, $b = 'k', $c = 'm') { print $a.$b.$c;} "
      "$a = 'T'; $a('o'); $a('o', 'p'); $a('o', 'p', 'q');");

  MVCR("<?php function test() { var_dump(func_get_args());} "
      "$a = 'Test'; $a();");
  MVCR("<?php function test($a) { var_dump(func_get_args());} "
      "$a = 'Test'; $a(1); $a(1, 2);");
  MVCR("<?php function test($a, $b) { var_dump(func_get_args());} "
      "$a = 'Test'; $a(1, 2); $a(1, 2, 3); $a(1, 2, 3, 4);");

  MVCR("<?php function t($a = 'k') { var_dump(func_get_args());} "
      "$a = 'T'; $a(); $a('o'); $a('o', 'p'); $a('o', 'p', 'q');");
  MVCR("<?php function t($a, $b = 'k') { var_dump(func_get_args());} "
      "$a = 'T'; $a('o'); $a('o', 'p'); $a('o', 'p', 'q');");
  MVCR("<?php function t($a, $b = 'k', $c = 'q') { var_dump(func_get_args());} "
      "$a = 'T'; $a('o'); $a('o', 'p'); $a('o', 'p', 'q');");

  MVCR("<?php function test(&$a, $b) { $a = 'ok';} $a = 'Test'; "
      "$a($a, 10); print $a;");

  MVCR("<?php $a = 'test'; function &test() { global $a; return $a;} "
      " $b = $a(); $b = 'ok'; var_dump($a); "
      " $b = &$a(); $b = 'ok'; var_dump($a);");

  MVCR("<?php function test($a, $b) { print $a.$b;} $a = 'Test'; $y = 'kqq'; "
      "$a('o',$y[0]);");
  MVCR("<?php function test($a, $b) { print $a.$b;} $a = 'Test'; "
      "$y = array('k','q','q'); $a('o',$y[0]);");

  MVCR("<?php "
      "$a = 'test';"
      "if ($a) {"
      "  function bar() {}"
      "} else {"
      "  function bar() {}"
      "}"
      "function foo() {}"
      "function goo(&$p) {}"
      "$goo = 'goo';"
      "goo(foo());"
      "$goo(foo());"
      "bar(foo());");

  // Test dynamically calling a builtin that is RefVariableArguments
  MVCR("<?php "
      "if ($argc > 100) { $f = 'var_dump'; } else { $f = 'sscanf'; }"
      "$auth = \"24\\tLewis Carroll\";"
      "$n = $f($auth, \"%d\\t%s %s\", $id, $first, $last);"
      "echo \"$id,$first,$last\\n\";");

  // Test dynamically calling a builtin that is MixedVariableArguments
  MVCR("<?php "
      "function bar($flag) {"
      "  $arr = array(array('b' => 3, 'a' => 2, 'c' => 1),"
      "               array('x' => 6, 'y' => 4, 'z' => 5),"
      "               array('p' => 8, 'q' => 9, 'r' => 7));"
      "  if ($flag) { $f = 'var_dump'; } else { $f = 'array_multisort'; }"
      "  $f($arr[0], $arr[1], $arr[2]);"
      "  var_dump($arr);"
      "}"
      "bar($argc > 100);");

  return true;
}

bool TestCodeRun::TestRenameFunction() {
  Option::DynamicInvokeFunctions.insert("test1");
  Option::DynamicInvokeFunctions.insert("test2");
  MVCR("<?php "
      "function test1() { print __FUNCTION__;} "
      "function test2() { print __FUNCTION__;} "
      "fb_rename_function('test2', 'test3');"
      "fb_rename_function('test1', 'test2'); teSt2();"
      "fb_rename_function('test2', 'test3'); teSt2();"
      );
  MVCR("<?php\n"
       "function one() { echo 'one';}\n"
       "fb_rename_function('one', 'two');\n"
       "fb_rename_function('two', 'three');\n"
       "three();");
  MVCR("<?php "
       "function test1() { echo \"test1\n\"; }"
       "function test3() { echo \"test3\n\"; }"
       "function baz($test1, $test2) {"
       "  var_dump(function_exists(\"Test1\"));"
       "  var_dump(function_exists(\"tEst2\"));"
       "  var_dump(function_exists($test1));"
       "  var_dump(function_exists($test2));"
       "}"
       "baz(\"teSt1\", \"test2\");"
       "fb_rename_function(\"test1\", \"test2\");"
       "baz(\"TEst1\", \"test2\");"
       "fb_rename_function(\"test3\", \"test1\");"
       "baz(\"test1\", \"test2\");"
       "test1();"
       "test2();")
  MVCRO("<?php\n"
        "function one() { echo 'one';}\n"
        "fb_renamed_functions(array('one', 'three'));\n"
        "var_dump(fb_rename_function('one', 'two'));\n",

        "bool(true)\n");

  MVCR("<?php "
       "function err($code,$msg) { var_dump($code,$msg); }"
       "set_error_handler('err');"
       "function test1() {}"
       "function test2() {}"
       "fb_rename_function('test1', 'test3');"
       "fb_rename_function('test2', 'test1');"
       "fb_rename_function('test1', 'test2');"
       "fb_rename_function('test3', 'test1');");


  Option::DynamicInvokeFunctions.clear();
  return true;
}

bool TestCodeRun::TestIntercept() {
  MVCRO("<?php "
        "$a = 10; $b = 20;"
        "function &foo(&$n, $p) { global $a; $n = 123; $p += 1; "
        "  var_dump('foo');"
        "return $a;}"
        "function &bar(&$n, $p) { global $b; $n = 456; $p += 2; "
        "  var_dump('bar');"
        "return $b;}"
        "fb_intercept('foo', 'fb_stubout_intercept_handler', 'bar');"
        "$n = 0; $d = 3; $c = &foo($n, $d); var_dump($c, $d); $c = 30;"
        //"var_dump($a, $b, $n);"
        ,
        "string(3) \"bar\"\nint(20)\nint(3)\n"
        //"int(10)\nint(30)\nint(456)\n"
       );

  MVCRO("<?php "
        "$a = 10; $b = 20;"
        "class A {"
        "function &foo(&$n, $p) { global $a; $n = 123; $p += 1; "
        "  var_dump('foo');"
        "return $a;}"
        "}"
        "class B {"
        "function &bar(&$n, $p) { global $b; $n = 456; $p += 2; "
        "  var_dump('bar');"
        "return $b;}"
        "}"
        "fb_intercept('A::foo', 'fb_stubout_intercept_handler', 'B::bar');"
        "$n = 0; $d = 3; $c = &A::foo($n, $d); var_dump($c, $d); $c = 30;"
        //"var_dump($a, $b, $n);"
        ,
        "string(3) \"bar\"\nint(20)\nint(3)\n"
        //"int(10)\nint(30)\nint(456)\n"
       );

  return true;
}

bool TestCodeRun::TestMaxInt() {
  MVCR("<?php "
       "$val1 = ~PHP_INT_MAX; "
       "$val2 = PHP_INT_MAX; "
       "var_dump($val1); "
       "var_dump($val2); "
       );
  return true;
}

bool TestCodeRun::TestDynamicMethods() {
  bool saveDynamic = Option::AllDynamic;
  Option::AllDynamic = true;
  MVCR("<?php "
      "class A { public function test() { print 'in A';} } "
      "class B extends A { public function test() { print 'in B';} } "
      "$obj = new B(); "
      "call_user_func_array(array($obj, 'A::test'), array());");

  MVCR("<?php $i = 'gi'; $s = 'gs'; class A { "
      "public function &dyn_test(&$a) { global $i; $a = $i; return $i;}} "
      "$obj = new A(); $f = 'dyn_test'; "
      "$c = &$obj->$f($b); var_dump($b); var_dump($c);");
  MVCR("<?php $i = 'gi'; $s = 'gs'; class A { "
      "public static function &dyn_test(&$a) "
      "{ global $s; $a = $s; return $s;}} "
      "$f = 'dyn_test'; $e = A::$f($d); var_dump($d); var_dump($e);");
  MVCR("<?php class dyn_A{} class B{} $cls = 'dyn_a'; $a = new $cls();");

  MVCR("<?php class A { function _test() { print 'ok';} "
      "function __call($name, $args) { $name = '_'.$name; $this->$name();} } "
      "$obj = new A(); $obj->test();");

  MVCR("<?php class A { function test($a, $b) { var_dump($a, $b);} } "
      "$m = 'test'; $o = new A();"
      "$ar = array(0,1); $st = 'abc';"
      "$o->$m($ar[0], $st[0]); A::$m($ar[1], $st[1]);");

  vector<string> backup = Option::DynamicMethodPrefixes;
  Option::DynamicMethodPrefixes.push_back("_");
  MVCR("<?php class A { function _test() { print 'ok';} "
      "function __call($name, $args) { $name = '_'.$name; $this->$name();} } "
      "$obj = new A(); $obj->test();");
  Option::DynamicMethodPrefixes = backup;

  MVCR("<?php "
      "class z {"
      "  function minArgTest($a1, $a2, $a3, $a4, $a5, $a6, $a7, $a8, $a9, $a10,"
      "                      $a11=true, $a12 = true) {"
      "    var_dump($a1);"
      "    var_dump($a2);"
      "    var_dump($a3);"
      "    var_dump($a4);"
      "    var_dump($a5);"
      "    var_dump($a6);"
      "    var_dump($a7);"
      "    var_dump($a8);"
      "    var_dump($a9);"
      "    var_dump($a10);"
      "    var_dump($a11);"
      "    var_dump($a12);"
      "  }"
      "  function varArgsTest() {"
      "    $args = func_get_args();"
      "    var_dump($args);"
      "  }"
      "  function varArgsTest2($a1, $a2) {"
      "    $args = func_get_args();"
      "    var_dump($args);"
      "  }"
      "  function refTestHelper(&$x) {"
      "    $x *= 2;"
      "  }"
      "}"
      "function refTest($q) {"
      "  if (false) { $q = 1; }"
      "  $x = 1;"
      "  $q->refTestHelper($x);"
      "  var_dump($x);"
      "}"
      "$q = new z;"
      "$f = 'minArgTest';"
      "$q->minArgTest('one',2,3.333,4,5,6,7,8,9,10);"
      "$q->minArgTest('one',2,3.333,4,5,6,7,8,9,10,11,12);"
      "$q->$f('one',2,3.333,4,5,6,7,8,9,10);"
      "$q->$f('one',2,3.333,4,5,6,7,8,9,10,11,12);"
      "refTest($q);"
      "$f = 'varArgsTest';"
      "$q->varArgsTest('one',2,3.333,4,5,6,7,8,9,10);"
      "$q->varArgsTest('one',2,3.333,4,5,6,7,8,9,10,11,12);"
      "$q->$f('one',2,3.333,4,5,6,7,8,9,10);"
      "$q->$f('one',2,3.333,4,5,6,7,8,9,10,11,12);"
      "$f = 'varArgsTest2';"
      "$q->varArgsTest2('one',2,3.333,4,5,6,7,8,9,10);"
      "$q->varArgsTest2('one',2,3.333,4,5,6,7,8,9,10,11,12);"
      "$q->$f('one',2,3.333,4,5,6,7,8,9,10);"
      "$q->$f('one',2,3.333,4,5,6,7,8,9,10,11,12);");

  MVCR("<?php "
      "class z {"
      "  function __construct() { echo 'construct'; }"
      "  function z() { echo 'method'; }"
      "}"
      "$z = new z;"
      "$z->z();");
  MVCR("<?php "
      "function bar() {"
      "  echo 'bar called';"
      "}"
      "class foo {"
      "  public $functions = array();"
      "  function foo() {"
      "    $function = 'bar';"
      "    print($function);"
      "    print($function());"
      "    $this->functions['test'] = $function;"
      "    print($this->functions['test']());"
      "  }"
      "}"
      "$a = new foo ();");
  MVCR("<?php "
      "function t($x) {"
      "  var_dump($x);"
      "}"
      "$x = array(1,2,3);"
      "array_map('t', $x);"
      "class z {"
      "  function q() {"
      "    $x = array(1,2,3);"
      "    array_map(array('self', 'p'), $x);"
      "  }"
      "  function p($x) {"
      "    var_dump($x);"
      "  }"
      "}"
      "$m = new z();"
      "$m->q();");

  MVCR("<?php "
       "class A {"
       "  function foo(&$test) {"
       "    $test = 10;"
       "  }"
       "}"
       "$obj = new A();"
       "$method = 'foo';"
       "$obj->$method($aa[3]);"
       "var_dump($aa);");

  Option::AllDynamic = saveDynamic;
  return true;
}

bool TestCodeRun::TestVolatile() {
  MVCR("<?php "
      "for ($i = 0; $i < 4; $i++) {"
      "  if ($i > 1 && !defined('CON')) {"
      "    define(/*|Dynamic|*/'CON', 1);"
      "  }"
      "  if (defined('CON')) {"
      "    var_dump(CON);"
      "  } else {"
      "    echo \"CON does not exists\\n\";"
      "  }"
      "}"
      "for ($i = 0; $i < 4; $i++) {"
      "  if ($i > 1 && !function_exists('foo')) {"
      "    function foo() {"
      "      echo \"foo called\\n\";"
      "    }"
      "  }"
      "  if (function_exists('foo')) {"
      "    foo();"
      "  } else {"
      "    echo \"foo does not exists\\n\";"
      "  }"
      "}"
      "for ($i = 0; $i < 4; $i++) {"
      "  if ($i > 1 && !class_exists('bar')) {"
      "    class bar {"
      "      function bar() { echo \"bar called\\n\"; }"
      "    }"
      "  }"
      "  if (class_exists('bar')) {"
      "    $a = new bar;"
      "  } else {"
      "    echo \"bar does not exists\\n\";"
      "  }"
      "}");
  MVCR("<?php "
      "function bar() {"
      "  for ($i = 0; $i < 4; $i++) {"
      "    if ($i > 1 && !function_exists('foo')) {"
      "      function foo() {"
      "        echo \"foo called\\n\";"
      "      }"
      "    }"
      "    $foo = 'foo';"
      "    if (function_exists($foo)) {"
      "      foo();"
      "    } else {"
      "      echo \"foo does not exists\\n\";"
      "    }"
      "  }"
      "  for ($i = 0; $i < 4; $i++) {"
      "    if ($i > 1 && !class_exists('goo')) {"
      "      class goo {"
      "        function goo() {"
      "          echo \"goo called\\n\";"
      "        }"
      "      }"
      "    }"
      "    $goo = 'goo';"
      "    if (class_exists($goo)) {"
      "      $a = new goo();"
      "    } else {"
      "      echo \"goo does not exists\\n\";"
      "    }"
      "  }"
      "}"
      "if (function_exists('bar')) bar();"
      "$a = 'bar';"
      "if (function_exists($a)) bar();"
      "$a = 'later';"
      "if (function_exists($a)) {"
      "  echo \"later exists\\n\";"
      "} else {"
      "  echo \"later does not exists\\n\";"
      "}"
      "$a = 'later2';"
      "if (class_exists($a)) {"
      "  echo \"later2 exists\\n\";"
      "} else {"
      "  echo \"later2 does not exists\\n\";"
      "}"
      "$a = 'later3';"
      "if (function_exists($a)) {"
      "  echo \"later3 exists\\n\";"
      "} else {"
      "  echo \"later3 does not exists\\n\";"
      "}"
      "$a = 'later4';"
      "if (class_exists($a)) {"
      "  echo \"later4 exists\\n\";"
      "} else {"
      "  echo \"later4 does not exists\\n\";"
      "}"
      "function later3() {"
      "  echo \"later3 called\\n\";"
      "}"
      "class later4 {"
      "}"
      "if (function_exists('function_exists')) {"
      "  echo \"yes\\n\";"
      "}"
      "if (class_exists('exception')) {"
      "  echo \"yes\\n\";"
      "}");
  MVCR("<?php "
      "function foo() {"
      "  if (!defined('Auth_OpenID_NO_MATH_SUPPORT')) {"
      "    define('Auth_OpenID_NO_MATH_SUPPORT', true);"
      "  }"
      "}"
      "function bar() {"
      "  return defined('Auth_OpenID_NO_MATH_SUPPORT');"
      "}"
      "if (defined('M_PI')) {"
      "  var_dump(bar());"
      "  foo();"
      "  var_dump(bar());"
      "}");
  MVCR("<?php "
      "function foo() {"
      "  if (!interface_exists('MyInterface')) {"
      "    interface MyInterface{};"
      "    echo 'no';"
      "  } else {"
      "    echo 'yes';"
      "  }"
      "}"
      "foo();"
      "foo();");
  MVCR("<?php "
      "function foo() {"
      "  if (function_exists('bar')) {"
      "    echo \"yes\\n\";"
      "  } else {"
      "    echo \"no\\n\";"
      "  }"
      "  function bar() {"
      "    echo \"bar\\n\";"
      "  }"
      "  if (function_exists('bar')) {"
      "    echo \"yes\\n\";"
      "  } else {"
      "    echo \"no\\n\";"
      "  }"
      "}"
      "foo();");
  MVCR("<?php "
      "function foo() {"
      "  if (class_exists('bar')) {"
      "    echo \"yes\\n\";"
      "  } else {"
      "    echo \"no\\n\";"
      "  }"
      "  class bar {"
      "  }"
      "  if (class_exists('bar')) {"
      "    echo \"yes\\n\";"
      "  } else {"
      "    echo \"no\\n\";"
      "  }"
      "}"
      "foo();");

  // Autoloading
  MVCR("<?php "
       "function z() {"
       "  var_dump('__autoload');"
       "  var_dump(class_exists('cNew'));"
       "  var_dump(class_exists('cNew_r'));"
       "  var_dump(class_exists('cNew_d'));"
       "  var_dump(class_exists('csm'));"
       "  var_dump(class_exists('csm_r'));"
       "  var_dump(class_exists('CcOn'));"
       "  var_dump(class_exists('CcOn_r'));"
       "  var_dump(class_exists('CcOn_d'));"
       "  var_dump(class_exists('csmeth'));"
       "  var_dump(class_exists('csmeth_r'));"
       "  var_dump(class_exists('csmeth_d'));"
       "  var_dump(class_exists('cpar'));"
       "  var_dump(class_exists('cpar_r'));"
       "  var_dump(class_exists('cref'));"
       "  var_dump(class_exists('cex'));"
       "}"
       "function test() {"
       "  function t1() {"
       "    new cNew();"
       "  }"
       "  t1();"
       "  function t2() {"
       "    new cNew_r();"
       "  }"
       "  t2();"
       "  function t4() {"
       "    $x = 'cNew_d';"
       "    new $x();"
       "  }"
       "  t4();"
       "  function t5() {"
       "    var_dump(csm::$mem);"
       "  }"
       "  t5();"
       "  function t6() {"
       "    var_dump(csm_r::$mem);"
       "  }"
       "  t6();"
       "  function t7() {"
       "    var_dump(CcOn::C);"
       "  }"
       "  t7();"
       "  function t8() {"
       "    var_dump(CcOn_r::C);"
       "  }"
       "  t8();"
       "  function t9() {"
       "    var_dump(constant('CcOn_d::C'));"
       "  }"
       "  t9();"
       "  function t10() {"
       "    csmeth::m();"
       "  }"
       "  t10();"
       "  function t11() {"
       "    csmeth_r::m();"
       "  }"
       "  t11();"
       "  function t12() {"
       "    call_user_func(array('csmeth_d', 'm'));"
       "  }"
       "  t12();"
       "  function t13() {"
       "    class a extends cpar {}"
       "    new a;"
       "  }"
       "  t13();"
       "  function t14() {"
       "    class b extends cpar_r {}"
       "    new b;"
       "  }"
       "  t14();"
       "  function t15() {"
       "    new ReflectionClass('cref');"
       "  }"
       "  t15();"
       "  function t16() {"
       "    var_dump(class_exists('cex'));"
       "  }"
       "  t16();"
       "  function t17() {"
       "    var_dump(class_exists('cex_r'));"
       "  }"
       "  t17();"
       "}"
       "test();"
       "z();"
       "function __autoload($name) {"
       "  var_dump('autoload ' . $name);"
       "  switch ($name) {"
       "    case 'cNew':"
       "      class cNew {}"
       "      break;"
       "    case 'cNew_r':"
       "      class cNew_r {}"
       "      if (false) {"
       "        class cNew_r {}"
       "      }"
       "      break;"
       "    case 'cNew_d':"
       "      class cNew_d {}"
       "      break;"
       "    case 'csm':"
       "      class csm {"
       "        public static $mem = 1;"
       "      }"
       "      break;"
       "    case 'csm_r':"
       "      class csm_r {"
       "        public static $mem = 1;"
       "      }"
       "      if (false) {"
       "        class csm_r {}"
       "      }"
       "      break;"
       "    case 'CcOn':"
       "      class CcOn {"
       "        const C = 2;"
       "      }"
       "      break;"
       "    case 'CcOn_r':"
       "      class CcOn_r {"
       "        const C = 2;"
       "      }"
       "      if (false) {"
       "        class CcOn_r {}"
       "      }"
       "      break;"
       "    case 'CcOn_d':"
       "      class CcOn_d {"
       "        const C = 2;"
       "      }"
       "      break;"
       "    case 'csmeth':"
       "      class csmeth {"
       "        public static function m() { echo '1\n'; }"
       "      }"
       "      break;"
       "    case 'csmeth_r':"
       "      class csmeth_r {"
       "        public static function m() { echo '1'; }"
       "      }"
       "      if (false) {"
       "        class csmeth_r {}"
       "      }"
       "      break;"
       "    case 'csmeth_d':"
       "      class csmeth_d {"
       "        public static function m() { echo '1'; }"
       "      }"
       "      break;"
       "    case 'cpar':"
       "      class cpar {}"
       "      break;"
       "    case 'cpar_r':"
       "      class cpar_r {}"
       "      if (false) {"
       "        class cpar_r {}"
       "      }"
       "      break;"
       "    case 'cref':"
       "      class cref {}"
       "      break;"
       "    case 'cex':"
       "      class cex {}"
       "      break;"
       "    case 'cex_r':"
       "      class cex_r {}"
       "      if (false) {"
       "        class cex_r {}"
       "      }"
       "      break;"
       "  }"
       "}");
  MVCR("<?php "
       "class_exists('c');"
       "class c {"
       "  const A = 'a';"
       "  const B = 'b';"
       "  const C = 'c';"
       "  const D = 'd';"
       "  public static $S = array("
       "    self::A,"
       "    self::B,"
       "    self::C,"
       "    self::D);"
       "}"
       "var_dump(c::$S);");
  MVCR("<?php "
       "class B {}"
       "class A extends B {"
       "  static function make() {"
       "    $b = new parent();"
       "    $a = new self();"
       "  }"
       "}"
       "if (false) { class A {};}"
       "A::make();");
  MVCR("<?php "
      "function foo($a) {"
      "  if ($a) {"
      "    class A {}"
      "  }"
      "}"
      "foo(true);"
      "function bar() {"
      "  if (class_exists('A')) {"
      "    class C extends A { }"
      "    $obj = new C;"
      "    var_dump($obj);"
      "  } else {"
      "    var_dump('no');"
      "  }"
      "}"
      "bar();");
  MVCR("<?php "
      "function foo($a) {"
      "  if ($a) {"
      "    interface A {}"
      "  }"
      "}"
      "foo(true);"
      "function bar() {"
      "  if (interface_exists('A')) {"
      "    class C implements A { }"
      "    $obj = new C;"
      "    var_dump($obj);"
      "  } else {"
      "    var_dump('no');"
      "  }"
      "}"
      "bar();");

  MVCR("<?php "
      "if (class_exists('B')) {"
      "  class A extends B {"
      "    public function f() {"
      "      var_dump('A');"
      "    }"
      "  }"
      "}"
      "class B extends C {"
      "  public function f() {"
      "    var_dump('B');"
      "  }"
      "}"
      "class C {}"
      "if (class_exists('A')) {"
      "  $obj = new A;"
      "  $obj->f();"
      "} else {"
      "  var_dump('correct');"
      "}");
  MVCR("<?php "
      "function wrapper($a) {"
      "  if ($a) {"
      "    class C {"
      "      private static $v;"
      "      public static function f() {"
      "        return self::$v;"
      "      }"
      "    }"
      "  }"
      "}"
      "class C2 {"
      "  private static $v;"
      "  public static function f() {"
      "    return self::$v;"
      "  }"
      "}"
      "function foo($a) {"
      "  if ($a == 0) return is_callable(array('C', 'f'), null);"
      "  return is_callable(array('C2', 'f'), null);"
      "}"
      "wrapper(false);"
      "var_dump(foo(0));"
      "var_dump(foo(1));"
      "if (class_exists('C')) var_dump('yes'); else var_dump('no');");
  MVCR("<?php "
      "function __autoload($name) {"
      "  if ($name == 'CaT') {"
      "  class CaT {"
      "    function __construct() {}"
      "  }"
      "  }"
      "  var_dump($name);"
      "}"
      ""
      "new CaT(1);"
      "class_exists('cat', false);");

  return true;
}

bool TestCodeRun::TestHereDoc() {
  MVCR("<?php\n"
       "$nullherequote= <<<fail\n"
       "fail;\n"
       "echo \"--$nullherequote--\\n\";"
       "$x=\"foo\";"
       "$threestops= <<<pass\n"
       "passable $x\n"
       "pass;x\n"
       "ss;\n"
       "pass;\n"
       "echo \"$threestops\\n\";");
  return true;
}

bool TestCodeRun::TestProgramFunctions() {
  //VCR("<?php var_dump($_SERVER);");
  MVCR("<?php var_dump($argc, count($argv));");
  //VCR("<?php var_dump($_ENV);");

  MVCR("<?php function p($a) { print $a;} "
      "register_shutdown_function('p', 'shutdown');");
  return true;
}

bool TestCodeRun::TestCompilation() {
  MVCR("<?php class A { public static $foo = 123;} $a = foo(); "
       "function foo() { return 'foo';} var_dump(A::$$a);");

  // testing re-declared classes with missing parents
  MVCR("<?php $a = bar(); if ($a) { class fOO extends Unknown {} } else "
       "{ class Foo extends unknOwn {} } function bar() { return 123;}");

  // testing re-declared classes with different cases
  MVCR("<?php $a = bar(); if ($a) { class fOO {} } else "
       "{ class Foo {} } function bar() { return 123;} $obj = new foo();");

  // testing re-declared functions with different cases
  MVCR("<?php $a = bar(); if ($a) { function fOO() {} } else "
       "{ function Foo() {} } function bar() { return 123;} foo();");

  // overlapped interface
  MVCR("<?php interface A {} class B implements A {} "
      "class C extends B implements A {} $obj = new C();");

  // trigraph
  MVCR("<?php print '\?\?/';");

  // testing type inference on a re-declared constant
  MVCR("<?php if (false) define('a', 'test'); define('a', 5); print $b % a;");

  // testing type inference on a special type casting: PlusOperand -> Double
  MVCR("<?php function d() { return '2009';} $y = (d()) + 6;");

  MVCR("<?php class A { public static $a = array('a', 'b'); public static function test() { self::$a[] = 'c'; var_dump(self::$a);} } A::test();");

  // \x65D is invalid in C++
  MVCR("<?php var_dump(\"[\\x][\\xA][\\x65][\\x65D]\");");
  MVCR("\\x65D");

  // float vs. double
  MVCR("<?php $a = 1; $a = 'test'; var_dump($a + 2.5);");

  // +/- String
  MVCR("<?php $a = -date('w');");

  // ObjectOffset.at()
  MVCR("<?php class A { public $a = array('t' => 't');} class B { public $a;} "
      "$a = 1; $a = new A(); $a->a['t'] = true; var_dump($a->a['t']);");

  // Variant % operator
  MVCR("<?php $a = date('d') % 10;");
  MVCR("<?php $a = 'test'; $a = 1; print $a % 10;");

  // defining a constant after it's used
  MVCR("<?php $a = MAX_LATITUDE + 5;"
      "if (12 > -MAX_LATITUDE) define('MAX_LATITUDE', 90); ");

  // toInt64() wrapper
  MVCR("<?php print 1 << 32;");

  // !$a is closer in C++
  MVCR("<?php if (!$a = true) {}");

  // integer as array element
  MVCR("<?php function test($a = 0) { $b = $a; $c = $b[$a];}");

  // String/Array operators
  MVCR("<?php function str() { return 'test';} "
      "function test() { var_dump(str() - $a);}");

  // unused variable warning
  MVCR("<?php function test() {} function foo() { test($a = 1);}");

  // void return functions
  MVCR("<?php function test() {} true ? test() : 1;");

  // uninitialized variables need to be Variant
  MVCR("<?php function test() { $a = 0; $a += $b;} test();");

  // VariantOffset = VariantOffset
  MVCR("<?php class A {} $a = new A(); $a->a = $a->b = 'test'; var_dump($a);");
  MVCR("<?php $a = 1; $a = array(); $a['a'] = $a['b'] = 'test'; var_dump($a);");

  // lval on Variant
  MVCR("<?php function test() { return array();} reset(test());");

  // variant.o_lval() needs lval() wrapper
  MVCR("<?php class A { public $prop = 1;} class B { public $prop = 5;} "
      "$a = 1; $a = new A(); $a->prop++; var_dump($a->prop);");

  // obj->prop doesn't need lval() wrapper
  MVCR("<?php class A { public $prop = 1;} "
      "$a = new A(); $a->prop++; var_dump($a->prop);");

  // ((p_obj)variant)->prop
  MVCR("<?php class A { public $prop = 1;} "
      "$a = 1; $a = new A(); $a->prop++; var_dump($a->prop);");

  // unsigned int should never be seen
  MVCR("<?php $a = 0xC0000000 & $b;");

  // redefine properties
  MVCR("<?php class E extends exception { public $message; public $code;}");

  // redefine static members
  MVCR("<?php class A { static $a = 1;} "
      "class B extends A { static $a = 2;} var_dump(B::$a);");

  // overriding function with assignment on parameters
  MVCR("<?php class A { "
      "function __call($a, $b) { $b = 'a'; $b = 1; "
      "var_dump($a, $b[0], $b[1]);}} "
      "$obj = new A(); $a = 1; $b = 'a'; $b = 2; $obj->test($a, $b);");

  // method->method
  MVCR("<?php class A { public function getA() { return $this;} "
      "public function test() { var_dump('test');}} "
      "class B { public function getA() {} public function test(){}}"
      "$obj = new A(); $obj->getA()->test();"
      );

  // constructor fallback
  MVCR("<?php class A { function __construct($a) { var_dump($a);} } "
      "class B extends A {} "
      "$a = new B('test');");

  // prop->method
  MVCR("<?php class A { function test() {}} class B { public $b;} "
      "class C { function test() {}} "
      "$a = 'test'; $a = new B(); $a->b = new A(); $a->b->test();");

  // testing code generation order
  MVCR("<?php "
      "$global = B::CLASS_CONSTANT; "
      "$another = test2($global); "
      "define('CONSTANT', test2('defining')); "
      //"test();"
      //"function test($a = CONSTANT) { test2($a);} "
      "function test2($a) { var_dump($a); return 12345;} "
      "class A extends B {} "
      "class B { const CLASS_CONSTANT = 1;} ");

  // $_SERVER is already defined
  MVCR("<?php $_SERVER = array('test' => 1); var_dump($_SERVER);");
  MVCR("<?php $GLOBALS['_SERVER'] = array('test' => 1); var_dump($_SERVER);");

  // class constant as default
  MVCR("<?php "
      "class A { const C = 123; static function t($a = B::C) {} } A::t();"
      "class B { const C = 456; static function t($a = A::C) {} } B::t();");

  // base virtual function prototype
  MVCR("<?php class T { function __toString() { return 123;}} "
      "$obj = new T(); var_dump($obj);");

  // void wrapper
  MVCR("<?php function test() {} var_dump(test()); $a = test();");

  // ambiguous overload
  MVCR("<?php $a = 'test'; $a = 123; switch ($a) { case -1: var_dump($a);}");

  // [][]
  MVCR("<?php $a['a']['b'] = 'test'; var_dump($a['a']['b']);");

  // testing Variant to specific class conversion
  MVCR("<?php class A { function test(A $a) { $a->foo();} "
      "function foo() { print 'foo';}}");

  MVCR("<?php "
      "class A { function f($a) {} }"
      "$obj = new A;"
      "$obj->f(date('m/d/y H:i:s', 123456789));"
      "$v = date(\"m\",123456789)+1;");

  // no side effect optimization met if() short
  MVCR("<?php if ($a) $a == 0;");

  // */ and // in default argument
  MVCR("<?php "
      "function foo($p1=\"/.*/\", $p2=\"//\") {"
      "  var_dump($p1, $p2);"
      "}"
      "foo();");

  MVCR("<?php "
       "$n = floor(1.0);"
       "var_dump(($n > 0) ? $n : $n + 1);");

  MVCR("<?php "
       "class X {"
       "  function test($a, $b, $c) {"
       "    return $a != $b;"
       "  }"
       "}"
       "function test($a) {"
       "  $x = new X;"
       "  return $a ? $x->test(1, 2) : false;"
       "}"
       "var_dump(test(1));");

  MVCR("<?php function bug1($a, $b) {"
       "foreach ($b[$a++ + $a++] as &$x) { echo $x; }}");

  MVCR("<?php "
       "function f($a, $b, $c) { return 'hello'; }"
       "function test($a) {"
       "  $x = ($a->foo = f($b++, $b++, $b++)) . f(1,2,3);"
       "  return $x;"
       "}");

  MVCR("<?php "
       "function g() {}"
       "function test1() {"
       "  return '' . g();"
       "}");

  MVCR("<?php "
       "function g() {}"
       "function test1() {"
       "  return 0 + g();"
       "}");

  MVCR("<?php "
       "function g() {}"
       "function test1() {"
       "  return 1 * g();"
       "}");

  MVCR("<?php "
       "$data = new stdclass;"
       "$type = 'OCI-Lob';"
       "if ($data instanceof $type) {"
       "  echo 'true';"
       "}"
       "class X {"
       "  static $foo;"
       "};"
       "function test() {"
       "  $x = new X;"
       "  $foo = 'foo';"
       "  if (isset($x->$foo) || empty($x->$foo) ||"
       "      isset($x->{'bar'})) {"
       "    unset($x->$foo);"
       "    unset($x->{'bar'});"
       "    echo true;"
       "  }"
       "}");

  MVCR("<?php "
       "class X {"
       "  static function g() {}"
       "};"
       "@X::g();"
       "function g($a,$b) {}"
       "function f() { return 3; }"
       "@g(f(),f());");

  MVCR("<?php for ($i = 0; $i++,$i - 10;) {}");

  MVCR("<?php function test($className) {"
       "$x = new ReflectionClass($className);"
       "return $x->newInstance()->loadAll(); }");

  MVCR("<?php var_dump(array(1,2,3)+array(4,5,6));");

  MVCR("<?php "
       "function foo($a) {"
       "  $r = '';"
       "  if ($a) {"
       "    $r ->error = '';"
       "  }"
       "  return $r;"
       "}"
       "var_dump(foo(true));"
       "var_dump(foo(false));");
  MVCR("<?php "
       "function foo($a) {"
       "  $r = '';"
       "  if ($a) {"
       "    $r ->error->line = 1;"
       "  }"
       "  return $r;"
       "}"
       "var_dump(foo(true));"
       "var_dump(foo(false));");

  MVCR("<?php "
       "call_user_func_array(array('Normalizer','normalize'),array('bar'));");

  MVCR("<?php "
       "function bar($g) { return $g; }"
       "class X {"
       "  static function foo() {"
       "    echo $this->baz(bar(1), bar(''));"
       "  }"
       "}");

  MVCR("<?php "
       "function foo() {}"
       "function test() {"
       "  foo()->bar();"
       "}");

  MVCR("<?php "
       "function checker($x) {"
       "  $msg = foo();"
       "  $notice = $msg['title'].'. '.$msg['body'];"
       "  foo();"
       "  @list($a,$b) = $x;"
       "  $x = @$x['a'];"
       "  $x = @$x['b'];"
       "  return $a - $b + $x;"
       "}");
  MVCR("<?php "
       "class C {"
       "  function foo($a) {"
       "    var_dump($this + $a);"
       "    var_dump($this - $a);"
       "    var_dump($this * $a);"
       "    var_dump($this / $a);"
       "    var_dump($a + $this);"
       "    var_dump($a - $this);"
       "    var_dump($a * $this);"
       "    var_dump($a / $this);"
       "  }"
       "}"
       "$obj = new C;"
       "$obj->foo(1);");
  MVCR("<?php "
       "class X {}"
       "class Y extends X { public $foo; }"
       "function foo() {"
       "  $x = new Y;"
       "  $x && var_dump($x->foo);"
       "  $x = new X;"
       "  var_dump($x);"
       "}"
       "foo();");

  MVCR("<?php "
       "function bar($a) {}"
       "function foo($x) {"
       "  $a = $x;"
       "  echo $x;"
       "  unset($a);"
       "  $a = bar(1);"
       "  bar($a++);"
       "}");

  MVCR("<?php function test($a) { return null * $a; }");

  MVCR("<?php "
      "function foo() {"
      "  define('AAA', 1);"
      "  if (false) {"
      "    define('BBB', 'bbb');"
      "  }"
      "}");
  return true;
}

bool TestCodeRun::TestReflection() {
  MVCR("<?php\n"
       "class A { public function foo() {}}\n"
       "$x = new ReflectionMethod('A::foo');\n"
       "var_dump($x->name, $x->class);\n");

  MVCR("<?php class A { public static function test() { print 'ok';}}"
      "var_dump(is_callable('A::test'));"
      "var_dump(function_exists('A::test'));");

  MVCR("<?php function test($a) { return 'ok'.$a;}"
      "var_dump(function_exists('TEst')); "
      "var_dump(is_callable('teSt'));"
      "var_dump(call_user_func('teST', 'blah')); "
      "var_dump(call_user_func_array('teST', array('blah'))); "
      );

  MVCR("<?php class B { public function f($a) { return 'ok'.$a;}} "
      "class A extends B { public $p = 'g';} "
      "$obj = new A(); "
      "var_dump(get_class($obj)); "
      "var_dump(get_parent_class($obj)); "
      "var_dump(is_a($obj, 'b')); "
      "var_dump(is_subclass_of($obj, 'b'));"
      "var_dump(method_exists($obj, 'f'));"
      "var_dump(method_exists('A', 'f'));"
      "var_dump(is_callable(array($obj, 'f')));"
      "var_dump(is_callable(array('A', 'f')));"
      "var_dump(get_object_vars($obj));"
      "var_dump(call_user_method('f', $obj, 'blah'));"
      "var_dump(call_user_method_array('f', $obj, array('blah')));"
      );

  MVCR("<?php class A { public static function f($a) { return 'ok'.$a;}} "
      "$obj = new A(); "
      "var_dump(method_exists($obj, 'f'));"
      "var_dump(method_exists('A', 'f'));"
      "var_dump(is_callable(array($obj, 'f')));"
      "var_dump(is_callable(array('A', 'f')));"
      "var_dump(call_user_func(array($obj,'f'), 'blah'));"
      "var_dump(call_user_func_array(array($obj,'f'), array('blah')));"
      "var_dump(call_user_func(array('A','f'), 'blah'));"
      "var_dump(call_user_func_array(array('A','f'), array('blah')));"
      );

  MVCR("<?php "
      "class A { function foo() {} }"
      "class B extends A { function bar() {}}"
      "var_dump(get_class_methods(new B()));");

  MVCR("<?php "
       "class X {"
       "  public function a() { var_dump(get_class_methods($this)); }"
       "  protected function b() {}"
       "  private function c() {}"
       "  final function d() {}"
       "  public function e() {}"
       "}"
       "class Y {"
       "  public function a() { var_dump(get_class_methods($this)); }"
       "}"
       "$x = new X;"
       "$x->a();"
       "$y = new Y;"
       "$y->a();"
       "var_dump(get_class_methods($x));"
       "var_dump(get_class_methods($y));");

  MVCR("<?php "
      "interface A { function foo(); }"
      "abstract class B implements A { function bar() {}}"
      "var_dump(get_class_methods('B'));");

  MVCR("<?php "
      "interface I1 { function ifoo2(); function ifoo1(); }"
      "interface I2 { function ifoo4(); function ifoo3(); }"
      "class A { function foo() {} function foo2() {} }"
      "abstract class B extends A implements I1, I2 { function bar() {}}"
      "abstract class C extends A implements I2, I1 { function bar() {}}"
      "class D extends C { function ifoo2() {} function ifoo1() {}"
      "  function ifoo4() {} function ifoo3() {} function bar() {} }"
      "var_dump(get_class_methods('B'));"
      "var_dump(get_class_methods('C'));"
      );

  MVCR("<?php class A { static $a = 10; public $b = 20;}"
      "$obj = new A(); var_dump(get_object_vars($obj));");

  MVCR("<?php\n"
       "function foo($a = null) {}\n"
       "$func = new ReflectionFunction('foo');\n"
       "$params = $func->getParameters();\n"
       "var_dump($params[0]->isDefaultValueAvailable());\n");

  MVCR("<?php "
       "class X {"
       "  function X() { }"
       "  function __construct() { }"
       "}"
       "class Y {"
       "  function Y() { }"
       "}"
       "class Z {"
       "  function z() { }"
       "}"
       "function test($cname, $mname) {"
       "  $x = new ReflectionClass($cname);"
       "  $m = $x->getMethod($mname);"
       "  echo \"$cname:$mname:\"; var_dump($m->isConstructor());"
       "}"
       "test('X', 'X');"
       "test('Y', 'Y');"
       "test('Y', 'y');"
       "test('Z', 'Z');"
       "test('Z', 'z');");
  MVCR("<?php "
      "function foo($a, $b) { }"
      "$funcs = get_defined_functions();"
      "var_dump($funcs['user']);");

  return true;
}

bool TestCodeRun::TestReflectionClasses() {
  MVCR("<?php "
      // declarations
      "interface i1 {} interface i2 {}"
      "class cls1 implements i1, i2 { "
      "  const CLS_CONST = 2;"
      "  protected $prop1;"
      "  public static $prop2 = 23;"
      "  function method1($param1) { print $param1;} "
      "} "
      "function &func1(cls1 $p1, &$p2, $p3='def') { "
      "  static $a=1; var_dump($p1);"
      "} "
      "function func2($a) { var_dump($a);} "
      "class cls2 extends cls1 {}"
      ""
      // helpers
      "function dump_func($func) {"
      "  var_dump($func->getName()); "
      "  var_dump($func->isInternal()); "
      "  var_dump($func->isUserDefined()); "
      "  $vars = $func->getStaticVariables(); "
      "  var_dump(count($vars));"
      "  var_dump(isset($vars['a']));"
      "  var_dump($func->returnsReference()); "
      "  var_dump($func->getNumberOfParameters()); "
      "  var_dump($func->getNumberOfRequiredParameters()); "
      "  foreach ($func->getParameters() as $name => $param) {"
      "    var_dump($name); "
      "    dump_param($param); "
      "  }"
      "} "
      ""
      "function verify_class($cls) {"
      "  if ($cls) {"
      "    var_dump($cls->getName()); "
      "  } else {"
      "    var_dump(null);"
      "  }"
      "}"
      ""
      "function verify_classes($classes) {"
      "  ksort($classes);"
      "  foreach ($classes as $cls) {"
      "    verify_class($cls); "
      "  }"
      "}"
      ""
      "function dump_param($param) {"
      "  var_dump($param->getName()); "
      "  var_dump($param->isPassedByReference()); "
      "  verify_class($param->getDeclaringClass()); "
      "  verify_class($param->getClass()); "
      "  var_dump($param->isArray()); "
      "  var_dump($param->allowsNull()); "
      "  var_dump($param->isOptional()); "
      "  var_dump($param->isDefaultValueAvailable()); "
      "  if ($param->isOptional()) { "
      //"    var_dump($param->getDefaultValue()); "
      "  } "
      "  var_dump($param->getPosition()); "
      "} "
      ""
      "function dump_prop($prop, $obj) {"
      "  var_dump($prop->getName()); "
      "  var_dump($prop->isPublic()); "
      "  var_dump($prop->isPrivate()); "
      "  var_dump($prop->isProtected()); "
      "  var_dump($prop->isStatic()); "
      //"  var_dump($prop->isDefault()); "
      "  var_dump($prop->getModifiers()); "
      "  if ($prop->isPublic()) { "
      "    var_dump($prop->getValue($obj)); "
      "    if (!$prop->isStatic()) {"
      "      var_dump($prop->setValue($obj, 78)); "
      "    }"
      "    var_dump($prop->getValue($obj)); "
      "  } "
      "  verify_class($prop->getDeclaringClass()); "
      "} "
      ""
      "function dump_class($cls, $obj) {"
      "  var_dump($cls->isInstance($obj)); "
      "  var_dump($cls->getName()); "
      "  var_dump($cls->isInternal()); "
      "  var_dump($cls->isUserDefined()); "
      "  var_dump($cls->isInstantiable()); "
      "  var_dump($cls->hasConstant('CLS_CONST')); "
      "  var_dump($cls->hasMethod('method1')); "
      "  var_dump($cls->hasProperty('prop1')); "
      "  dump_func($cls->getMethod('method1')); "
      "  dump_prop($cls->getProperty('prop1'), $obj); "
      "  verify_classes($cls->getInterfaces()); "
      "  var_dump($cls->isInterface()); "
      "  var_dump($cls->isAbstract()); "
      "  var_dump($cls->isFinal()); "
      "  var_dump($cls->getModifiers()); "
      "  verify_class($cls->getParentClass()); "
      "  var_dump($cls->isSubclassOf('i1')); "
      "  var_dump($cls->getStaticPropertyValue('prop2')); "
      //"  var_dump($cls->setStaticPropertyValue('prop2', 45)); "
      "  cls1::$prop2 = 45; "
      "  var_dump($cls->getStaticPropertyValue('prop2')); "
      "  var_dump(cls1::$prop2); "
      "  var_dump($cls->isIterateable()); "
      "  var_dump($cls->implementsInterface('i2')); "
      "  foreach ($cls->getProperties() as $name => $prop) {"
      "    var_dump($name); "
      "    dump_prop($prop, $obj); "
      "  }"
      "  foreach ($cls->getMethods() as $name => $func) {"
      "    var_dump($name); "
      "    dump_func($func); "
      "    var_dump($func->isFinal()); "
      "    var_dump($func->isAbstract()); "
      "    var_dump($func->isPublic()); "
      "    var_dump($func->isPrivate()); "
      "    var_dump($func->isProtected()); "
      "    var_dump($func->isStatic()); "
      "    var_dump($func->isConstructor()); "
      "    var_dump($func->isDestructor()); "
      "    var_dump($func->getModifiers() & 0xFFFF); "
      "    verify_class($func->getDeclaringClass()); "
      "    if ($name == 'method1') $func->invoke($obj, 'invoked'); "
      "  }"
      "}"
      ""
      // verification
      "$func = new ReflectionFunction('func1'); "
      "dump_func($func); "
      ""
      "$func = new ReflectionFunction('func2'); "
      "$func->invoke('invoked');"
      ""
      "$cls = new ReflectionClass('cls1'); "
      "$obj = $cls->newInstance(); "
      "dump_class($cls, $obj);"
      ""
      "$cls = new ReflectionClass('cls2'); "
      "$obj = $cls->newInstance(); "
      "dump_class($cls, $obj);"
      "");
  MVCR("<?php "
      "abstract class c {"
      "  public static $arr = array();"
      "  function g() {"
      "    $cl = new ReflectionClass(get_class($this));"
      "    $p = $cl->getProperty('arr');"
      "    return $p->getValue();"
      "  }"
      "}"
      "abstract class aa extends c {"
      "  public function get_arr() {"
      "    $actions = parent::get_arr();"
      "    return $actions;"
      "  }"
      "}"
      "class a extends aa {"
      "  public static $arr = array('v');"
      "}"
      "$x = new a;"
      "var_dump($x->g());");
  MVCR("<?php "
      "$z=true;"
      "if ($z) {"
      "  class AaaA {"
      "    function f() {"
      "      var_dump(get_class());"
      "    }"
      "  }"
      "} else {"
      "  class aAAa {}"
      "}"
      "class BbBb {}"
      "$r = new ReflectionClass('aaaa');"
      "var_dump($r->getName());"
      "$r = new ReflectionClass('bbbb');"
      "var_dump($r->getName());"
      "$a = new aaaa;"
      "$a->f();");
  MVCR("<?php "
      "class z {"
      "  const foo = 10;"
      "}"
      "class c {"
      "  const bar = z::foo;"
      "}"
      "var_dump(c::bar);"
      "$r = new ReflectionClass('c');"
      "var_dump($r->getConstant(\"bar\"));");
  MVCR("<?php "
      "class fOo {}"
      "interface ioO {}"
      "$c = new ReflectionClass('Foo');"
      "$i = new ReflectionClass('Ioo');"
      "var_dump($c->getFileName() !== '');"
      "var_dump($i->getFileName() !== '');");
  MVCR("<?php "
       "class C {"
       "public function mE() {"
       "echo 'fail';"
       "}"
       "}"
       "$ref = new ReflectionClass('C');"
       "var_dump($ref->hasMethod('mE'));"
       "var_dump($ref->hasMethod('me'));"
       "$m = $ref->getMethod('me');"
       "var_dump($m->getName());");

  MVCR("<?php "
      "class Base {"
      "  function foo() {"
      "    $m = new ReflectionMethod(get_class($this), 'bar');"
      "    var_dump($m->name);"
      "  }"
      "}"
      "$condition = 123;"
      "if ($condition) {"
      "  class A extends Base {}"
      "} else {"
      "  class A extends Base {}"
      "}"
      "class B extends A {"
      "  function bar() {"
      "  }"
      "}"
      "$obj = new B();"
      "$obj->foo();");

  MVCR("<?php "
       "interface A {function foo();}"
       "interface B extends A {}"
       "class C implements B {function foo() {}};"
       "$klass = new ReflectionClass('C');"
       "var_dump($klass->implementsInterface('A'));"
       "$inter = new ReflectionClass('B');"
       "var_dump($inter->hasMethod('foo'));");

  MVCR("<?php\n"
       "class A { private $a; protected $b; public $c; static $d; }\n"
       "function f($a) { foreach ($a as $v) { var_dump($v->getName()); } }\n"
       "$r = new ReflectionClass('A');\n"
       "$a = $r->getProperties(); f($a);\n"
       "$a = $r->getProperties(ReflectionProperty::IS_PUBLIC); f($a);\n"
       "$a = $r->getProperties(ReflectionProperty::IS_PRIVATE); f($a);\n"
       "$a = $r->getProperties(ReflectionProperty::IS_PROTECTED); f($a);\n"
       "$a = $r->getProperties(ReflectionProperty::IS_STATIC); f($a);\n");

  return true;
}

bool TestCodeRun::TestErrorHandler() {
  MVCR("<?php function handler($code, $msg) { "
      "  var_dump(strpos($msg, 'system error') !== false); return true;"
      "} "
      "set_error_handler('handler');"
      "function a() {} set_error_handler('a');restore_error_handler();"
      "trigger_error('system error'); "
      );

  MVCR("<?php function handler($code, $msg) { "
      "  var_dump(strpos($msg, 'system error') !== false); return true;"
      "} "
      "set_error_handler('handler');"
      "user_error('system error'); "
      );

  MVCR("<?php function handler($e) { "
      "  var_dump(strpos((string)$e, 'bomb') !== false); return true;"
      "} "
      "set_exception_handler('handler');"
      "function a() {} set_exception_handler('a');restore_exception_handler();"
      "throw new Exception('bomb'); "
      );

  MVCR("<?php "
      "ob_start();"
      "set_exception_handler('user_exception_handler');"
      "echo 'Hello World';"
      "throw new Exception;"
      "function user_exception_handler($e) {"
      "  ob_end_clean();"
      "  var_dump(error_get_last());"
      "  echo 'Goodbye';"
      "  var_dump(error_get_last());"
      "}");
  MVCR("<?php\n"
      "function handler($code, $msg, $file, $line) { var_dump($line); }\n"
      "set_error_handler('handler');\n"
      "function f($a) {\n"
      "  $b = $a[100];\n"
      "  return $b;\n"
      "}\n"
      "f(array(1, 2, 3));\n");
  MVCR("<?php\n"
       "class C {"
       "  public static function log(Exception $exception) {"
       "    $msg = get_class($exception).': '.$exception->getMessage();"
       "    var_dump($msg);"
       "  }"
       "  public static function setup() {"
       "    set_exception_handler(array(__CLASS__, 'log'));"
       "  }"
       "}"
       "$obj = new C;"
       "$obj->setup();"
       "throw new Exception('test');");

  return true;
}

bool TestCodeRun::TestAssertOptions() {
  MVCR("<?php "
      "assert_options(ASSERT_ACTIVE, 0);"
      "assert_options(ASSERT_WARNING, 0);"
      "var_dump(assert(false));"
      "var_dump(assert_options(ASSERT_ACTIVE, 0));"
      "var_dump(assert_options(ASSERT_WARNING, 1));"
      "var_dump(assert(false));"
      "var_dump(assert_options(ASSERT_ACTIVE, 0));"
      "var_dump(assert_options(ASSERT_WARNING, 2));"
      "var_dump(assert(false));"
      "var_dump(assert_options(ASSERT_ACTIVE, 1));"
      "var_dump(assert_options(ASSERT_WARNING, 0));"
      "var_dump(assert(false));"
      "var_dump(assert_options(ASSERT_ACTIVE, 2));"
      "var_dump(assert_options(ASSERT_WARNING, 0));"
      "var_dump(assert(false));");

  return true;
}

bool TestCodeRun::TestExtMisc() {
  MVCR("<?php var_dump(pack('nvc*', 0x1234, 0x5678, 65, 66));");
  MVCR("<?php var_dump(unpack('nfrist/vsecond/c2chars', "
      "pack('nvc*', 0x1234, 0x5678, 65, 66)));");
  MVCR("<?php $d=fopen('test/test_code_run.cpp', 'r');"
       "var_dump(is_object($d));var_dump(is_resource($d));");
  return true;
}

bool TestCodeRun::TestInvalidArgument() {
  MVCR("<?php "
      "var_dump(fb_rename_function('', ''));"

      "$ch = curl_init();"
      "var_dump(curl_setopt($ch, -1, 'http://www.example.com/'));"
      "curl_close($ch);"

      "var_dump(hotprofiler_enable(-1));"

      "var_dump(iconv_set_encoding('internal_encoding',"
      "  str_pad('invalid-charset', 64)));"
      "var_dump(iconv_mime_decode("
      "  'Subject: =?UTF-8?B?UHLDvGZ1bmcgUHLDvGZ1bmc=?=',"
      "  0, str_pad('invalid-charset', 64)));"
      "var_dump(iconv_mime_decode_headers("
      " 'Subject: =?UTF-8?B?UHLDvGZ1bmcgUHLDvGZ1bmc=?=', 0,"
      "  str_pad('invalid-charset', 64)));"
      "var_dump(iconv_strlen('UHLDvGZ1bmcgUHLDvGZ1bmc=',"
      "  str_pad('invalid-charset', 64)));"
      "$subject = 'Subject: =?UTF-8?B?UHLDvGZ1bmcgUHLDvGZ1bmc=?=';"
      "var_dump(iconv_strpos($subject,"
      " 'H', 0, str_pad('invalid-charset', 64)));"
      "var_dump(iconv_strrpos($subject,"
      " 'H', str_pad('invalid-charset', 64)));"
      "var_dump(iconv_substr('AB',0,1, str_pad('invalid-charset', 64)));"
      "$preferences = array("
      "  'output-charset' => 'UTF-8',"
      "  'line-length' => 76,"
      "  'line-break-chars' => '\n'"
      ");"
      "$preferences['scheme'] = 'Q';"
      "$preferences['input-charset'] = str_pad('invalid-charset', 64);"
      "var_dump(iconv_mime_encode('Subject', 'Pr\xc3\274fung Pr\xc3\274fung',"
      "  $preferences));"
      "$preferences['input-charset'] = 'ISO-8859-1';"
      "$preferences['output-charset'] = str_pad('invalid-charset', 64);"
      "var_dump(iconv_mime_encode('Subject', 'Pr\xc3\274fung Pr\xc3\274fung',"
      "  $preferences));"
      "var_dump(iconv_set_encoding('internal_encoding',"
      "                            str_pad('invalid-charset', 64)));"
      "var_dump(iconv('UTF-8', str_pad('invalid-charset', 64), ''));"
      "var_dump(iconv(str_pad('invalid-charset', 64), 'UTF-8', ''));"

      "var_dump(time_nanosleep(-1, 0));"
      "var_dump(time_nanosleep(0, -1));"
      "var_dump(time_sleep_until(0));"

      "var_dump(gzcompress('abc', -2));"
      "var_dump(gzdeflate('abc', -2));"

      "var_dump(http_build_query(1));"
      "var_dump(parse_url('http://www.example.com', 100));"

      "var_dump(mysql_fetch_array(null, 0));"
      "var_dump(mysql_fetch_object(null, 'stdClass'));"

      "var_dump(dns_check_record('127.0.0.1', 'INVALID_TYPE'));"

      "var_dump(assert_options(-1));"

      "var_dump(simplexml_load_string('', 'INVALID_CLASS'));"
      "var_dump(simplexml_load_string('', 'stdClass'));"

      "var_dump(stream_get_contents('', -1));"

      "$fp = fopen('test/test_ext_file.txt', 'r');"
      "var_dump(fgets($fp, -1));"
      "var_dump(fputcsv($fp, array(), 'abc'));"
      "var_dump(fputcsv($fp, array(), 'a', 'def'));"
      "var_dump(fgetcsv($fp, array(), 'abc'));"
      "var_dump(fgetcsv($fp, array(), 'a', 'def'));"
      "fclose($fp);"
      "$tmpfname = tempnam('', str_repeat('a', 128));"
      "var_dump(strlen(basename($tmpfname)));"
      "unlink($tmpfname);"
      "$tmpfname = tempnam('', '/var/www' . str_repeat('a', 128));"
      "var_dump(strlen(basename($tmpfname)));"
      "unlink($tmpfname);"

      "$ar1 = array(10, 100, 100, 0);"
      "$ar2 = array(1, 3, 2);"
      "var_dump(array_multisort($ar1, $ar2));"

      "$phrase  = 'eat fruits, vegetables, and fiber every day.';"
      "$healthy = array('fruits', 'vegetables');"
      "$yummy   = array('pizza', 'beer', 'ice cream');"
      "var_dump(str_replace($healthy, $yummy, $phrase));"
      "var_dump(str_replace('ll', $yummy, 'good golly miss molly!',"
      "                     $count));"
      "var_dump(setlocale(LC_ALL, array('de_DE@euro', 'de_DE', 'deu_deu'),"
      "                   array(1, 2)));"
      "var_dump(setlocale(LC_ALL, str_pad('a', 255)));"

      "var_dump(pack(\"\\xf4\", 0x1234, 0x5678, 65, 66));"
      "var_dump(pack(\"x5\", 0x1234, 0x5678, 65, 66));"
      "var_dump(pack(\"h\", -0x1234));"
      "var_dump(pack(\"h\", 12345678900));"
      "var_dump(unpack(\"\\xf4\", \"0x1234\"));"

      "var_dump(sscanf('foo', '[%s', $id, $first, $last));"
      "var_dump(sscanf('foo', '%z', $id, $first, $last));"
      "var_dump(sscanf(\"SN/abc\", \"SN/%d%d\", $out));"
      "var_dump($out);"
      "var_dump(sscanf(\"SN/abc\", \"\", $out));"
      "var_dump($out);"

      "var_dump(printf('%$', 3));"
      "var_dump(vsprintf('%$', 3));"
      "var_dump(sprintf('%$', 3));"
      "var_dump(vsprintf('%$', 3));"

      "var_dump(str_word_count('abc', 2, '...'));"
      "var_dump(str_word_count('abc', 2, 'b..a'));"
      "var_dump(str_word_count('abc', 2, 'a..b..c'));"

      "var_dump(base_convert('05678', 8, 37));"

      "var_dump(convert_cyr_string('abc', 'y', 'z'));"

      "var_dump(money_format('%abc', 1.33));"
      "var_dump(money_format('%i%i', 1.33));"

      "var_dump(str_pad('abc', 10, '', 100));"
      "var_dump(str_pad('abc', 10, ' ', 100));"

      "var_dump(wordwrap('', 75, '', true));"
      "var_dump(wordwrap(null, 75, '', true));"
      "var_dump(wordwrap('abc', 75, '', true));"
      "var_dump(wordwrap('abc', 0, '', true));");


  MVCR("<?php "
       "function bar($a) { return $a; }"
       "function baz($a) { return $a; }"
       "function foo($x) {"
       "  return fb_call_user_func_safe_return('baz',"
       "         fb_call_user_func_safe_return('bar', $x));"
       "}");

  MVCR("<?php "
       "function handler($err, $errstr) {"
       "  $errstr = preg_replace('/given,.*$/','given', $errstr);"
       "  var_dump($err, $errstr);"
       "}"
       "set_error_handler('handler');"
       "class y {}"
       "class x {"
       "  function __construct(y $y) {}"
       "}"
       "var_dump(new X(null));");
  return true;
}

bool TestCodeRun::TestSuperGlobals() {
  MVCR("<?php function foo() { "
       "file_get_contents('http://example.com');"
       "var_dump(empty($http_response_header));"
       "} foo();");
  MVCR("<?php "
       "function test() {"
       "  unset($GLOBALS['_SERVER']);"
       "  $GLOBALS['_SERVER']['foo'] = 'bar';"
       "  var_dump($_SERVER['foo']);"
       "  }"
       "test();");
  MVCR("<?php "
       "function test() {"
       "  $_POST = array('HELLO' => 1);"
       "}"
       "test();"
       "var_dump($_POST);");

  MVCR("<?php "
       "class X {"
       "  static function test() {"
       "    var_dump(__FUNCTION__);"
       "    var_dump(__CLASS__);"
       "    var_dump(__METHOD__);"
       "    return array($GLOBALS[__FUNCTION__],"
       "                 $GLOBALS[__CLASS__],"
       "                 $GLOBALS[__METHOD__]);"
       "  }"
       "}"
       "$test = 'this_is_function_test';"
       "$X = 'this_is_class_x';"
       "$GLOBALS['X::test'] = 'this_is_method_test::x';"
       "var_dump(X::test());");

  MVCR("<?php\n"
       "$a = 100;\n"
       "function f() {\n"
       "  foreach ($GLOBALS as $k => &$v) {\n"
       "    if ($k == 'a') { $v = -1; }\n"
       "  }\n"
       "  global $a;\n"
       "  var_dump($a);\n"
       "  $b = $GLOBALS;\n"
       "  $b['a'] = 0;\n"
       "  var_dump($GLOBALS['a']);\n"
       "  var_dump(end($GLOBALS));\n"
       "  reset($GLOBALS);\n"
       "  end($b);\n"
       "  var_dump(current($GLOBALS));\n"
       "}\n"
       "f();\n");

  return true;
}

bool TestCodeRun::TestGlobalStatement() {
  MVCR("<?php\n"
       "global $c;\n"
       "function &foo() {\n"
       "  $a = 5;\n"
       "  global $c;\n"
       "  $c = &$a;\n"
       "  var_dump($c);\n"
       "  return $a;\n"
       "}\n"
       "$b = foo();\n"
       "$b = 6;\n"
       "var_dump($c);\n"
       "var_dump($b);\n"
      );

  MVCR("<?php "
      "$global_var = 10;"
      "function test_unset() {"
      "  global $global_var;"
      "  var_dump( isset($global_var) );"
      "  var_dump( empty($global_var) );"
      "  unset($global_var);"
      "  var_dump( isset($global_var) );"
      "  var_dump( empty($global_var) ); }"
      "var_dump($global_var);"
      "test_unset();"
      "var_dump($global_var);"
      );

  MVCR("<?php "
      "$a = 0;"
      "function test() {"
      "  $a = 1;"
      "  if (true) global $a;"
      "  $a = 2;"
      "}"
      "test();"
      "print \"$a\\n\";"
      );

  MVCR("<?php "
      "$a = 1;"
      "function test() {"
      "  $b = 1;"
      "  global $a;"
      "  $a = 10;"
      "}"
      "var_dump($a);"
      "test();"
      "var_dump($a);"
      "  return true;"
      );

  MVCR("<?php "
      "function test() {"
      "  if (true) {"
      "    global $a;"
      "    $a = 10;"
      "  }"
      "}"
      "test();"
      "var_dump($a);"
      );

  MVCR("<?php "
       "$x = 1;"
       "$GLOBALS['x'] = 2;"
       "var_dump($x);"
       "function test() {"
       "  global $x;"
       "  $x = 3;"
       "  $GLOBALS['x'] = 4;"
       "  var_dump($x);"
       "}"
       "test();");

  return true;
}

bool TestCodeRun::TestStaticStatement() {
  MVCR("<?php "
      "function test_unset_static() {"
      "  static $static_var;"
      "  $static_var ++;"
      "  echo \"value of static_var before unset: $static_var\\n\";"
      "  var_dump( isset($static_var) );"
      "  var_dump( empty($static_var) );"
      "  unset($static_var);"
      "  echo \"value of static_var after unset: $static_var\\n\";"
      "  var_dump( isset($static_var) );"
      "  var_dump( empty($static_var) );"
      "  $static_var = 20;"
      "  echo \"value of static_var after new assignment: $static_var\\n\";"
      "}"
      "test_unset_static();"
      "test_unset_static();"
      "test_unset_static();"
      );

  MVCR("<?php "
      "function test() {"
      "  $static_var = 3;"
      "  echo $static_var;"
      "  static $static_var;"
      "  $static_var ++;"
      "  echo $static_var;"
      "}"
      "test();"
     )

  MVCR("<?php "
      "class A { static function test() {"
      "  $static_var = 3;"
      "  echo $static_var;"
      "  static $static_var;"
      "  $static_var ++;"
      "  echo $static_var;"
      "} }"
      "A::test();"
     )

  MVCR("<?php "
      "  $static_var = 1;"
      "  echo $static_var . \"\\n\";"
      "  static $static_var;"
      "  echo $static_var . \"\\n\";"
      "  $static_var ++;"
      "  echo $static_var . \"\\n\";"
     )

  MVCR("<?php "
      "  $static_var = 1;"
      "  global $static_var;"
      "  echo $static_var . \"\\n\";"
      "  $static_var --;"
      "  echo $static_var . \"\\n\";"
     )

  MVCR("<?php "
      "  $static_var = 1;"
      "  echo $static_var . \"\\n\";"
      "  static $static_var;"
      "  echo $static_var . \"\\n\";"
      "  $static_var ++;"
      "  echo $static_var . \"\\n\";"
      "  global $static_var;"
      "  echo $static_var . \"\\n\";"
      "  $static_var --;"
      "  echo $static_var . \"\\n\";"
     )

  MVCR("<?php "
      "static $static_var = 1;"
      "echo $static_var . \"\\n\";"
      "function test()"
      "{"
      "  static $static_var = -1;"
      "  echo $static_var . \"\\n\";"
      "  $static_var = 2;"
      "  echo $static_var . \"\\n\";"
      "  $static_var++;"
      "  echo $static_var . \"\\n\";"
      "}"
      "test();"
      "test();"
     )

  MVCR("<?php "
      "static $static_var;"
      "echo $static_var . \"\\n\";"
      "$static_var = 1;"
      "echo $static_var . \"\\n\";"
     )

  MVCR("<?php "
      "function test() {"
      "  if (false) {"
      "    static $static_var = +3;"
      "  }"
      "  echo $static_var . \"\\n\";"
      "  $static_var = 4;"
      "  echo $static_var . \"\\n\";"
      "}"
      "test();"
     )

  MVCR("<?php "
      "if (false) {"
      "  static $static_var = +3;"
      "}"
      "echo $static_var . \"\\n\";"
      "$static_var = 4;"
      "echo $static_var . \"\\n\";"
     )

  MVCR("<?php "
      "echo $static_var . \"\\n\";"
      "static $static_var = 4;"
      "echo $static_var . \"\\n\";"
      );

  MVCR("<?php "
      "static $a = 5;"
      "echo $a . \"\\n\";"
      "global $a;"
      "echo $a . \"\\n\";"
      );

  MVCR("<?php "
      "function test() {"
      "  static $commenced = false;"
      "  if ($commenced === false) {"
      "    return 1;"
      "  }"
      "  $commenced = true;"
      "  unset($args);"
      "}"
      "echo test();"
      );

  MVCR("<?php "
      "static $a = 1, $b = 2;"
      "static $c = 1;"
      "static $d = 1;"
      "static $e = 1;"
      "static $f = 1;"
      "static $g = 1;"
      "static $h = 1;"
      "static $i = 1;"
      "static $i = 2;"
      "if (false) {"
      "  static $a = 2;"
      "  static $b = 3;"
      "  static $c = 2;"
      "  static $g;"
      "  static $i;"
      "  $e = 2;"
      "} else {"
      "  static $d = 2;"
      "  static $h;"
      "  $f = 2;"
      "}"
      "echo $a;"
      "echo $b;"
      "echo $c;"
      "echo $d;"
      "echo $e;"
      "echo $f;"
      "echo $g;"
      "echo $h;"
      "echo $i;"
      ""
      "function f() {"
      "  static $a = 1, $b = 2;"
      "  static $c = 1;"
      "  static $d = 1;"
      "  static $e = 1;"
      "  static $f = 1;"
      "  static $g = 1;"
      "  static $h = 1;"
      "  static $i = 1;"
      "  static $i = 2;"
      "  if (false) {"
      "    static $a = 2;"
      "    static $b = 3;"
      "    static $c = 2;"
      "    static $g;"
      "    static $i;"
      "    $e = 2;"
      "  } else {"
      "    static $d = 2;"
      "    static $h;"
      "    $f = 2;"
      "  }"
      "  echo $a;"
      "  echo $b;"
      "  echo $c;"
      "  echo $d;"
      "  echo $e;"
      "  echo $f;"
      "  echo $g;"
      "  echo $h;"
      "  echo $i;"
      "}"
      "f();"
      "class foo {"
      "  static $a = 1, $b = 2;"
      "  static $c = 1;"
      "  static $d = 1;"
      "  static $e = 1;"
      "  static $f = 1;"
      "  function bar() {"
      "    static $a = 1, $b = 2;"
      "    static $c = 1;"
      "    static $d = 1;"
      "    static $e = 1;"
      "    static $f = 1;"
      "    static $g = 1;"
      "    static $h = 1;"
      "    static $i = 1;"
      "    static $i = 2;"
      "    if (false) {"
      "      static $a = 2;"
      "      static $b = 3;"
      "      static $c = 2;"
      "      static $g;"
      "      static $i;"
      "      $e = 2;"
      "    } else {"
      "      static $d = 2;"
      "      static $h;"
      "      $f = 2;"
      "    }"
      "    echo foo::$a;"
      "    echo foo::$b;"
      "    echo foo::$c;"
      "    echo foo::$d;"
      "    echo foo::$e;"
      "    echo foo::$f;"
      "    echo $a;"
      "    echo $b;"
      "    echo $c;"
      "    echo $d;"
      "    echo $e;"
      "    echo $f;"
      "    echo $g;"
      "    echo $h;"
      "    echo $i;"
      "  }"
      "}"
      "echo foo::$a;"
      "echo foo::$b;"
      "$v = new foo;"
      "$v->bar();");

  MVCR("<?php "
      "define('FOO', 1);"
      "class a {"
      "  static $b = FOO;"
      "}"
      "function foo() {"
      "  static $a;"
      "  static $a = FOO;"
      "  echo $a;"
      "}"
      "foo();"
      "echo a::$b;");

  MVCR("<?php "
      "class c {"
      "  public $q = 20;"
      "  function x() {"
      "    $foo = 20;"
      "    static $foo;"
      "    $foo = $this->q;"
      "    echo $foo;"
      "  }"
      "  function y() {"
      "    static $foo = 20;"
      "    $foo++;"
      "    echo $foo;"
      "  }"
      "  static function sf() {"
      "    static $foo = 0;"
      "    $foo++;"
      "    echo $foo;"
      "  }"
      "}"
      "class d extends c {"
      "  public $q = 30;"
      "}"
      "$x = new c();"
      "$x->x();"
      "$x->y();"
      "$x->y();"
      "$x->y();"
      "$x->y();"
      "$x = new d();"
      "$x->x();"
      "$x->y();"
      "$x->y();"
      "$x->y();"
      "c::sf();"
      "c::sf();"
      "c::sf();"
      "d::sf();"
      "d::sf();"
      "d::sf();");

  MVCR("<?php "
       "class A {"
       "  private function foo() {"
       "    static $x = null;"
       "    var_dump(get_class($this), $x);"
       "    $x = 1;"
       "  }"
       "  public function run() {"
       "    $this->foo();"
       "  }"
       "}"
       "class B extends A {}"
       "class C extends A {}"
       "$a = new A;"
       "$b = new B;"
       "$c = new C;"
       "$a->run();"
       "$b->run();"
       "$c->run();");

  return true;
}

bool TestCodeRun::TestIfStatement() {
  MVCR("<?php "
      "if (false) {"
      "  echo \"case 1\\n\";"
      "}"
      "if (false) {"
      "  echo \"case 2\\n\";"
      "  function f1() {"
      "    $v = 1;"
      "  }"
      "}"
      "if (true) {"
      "  echo \"case 3\\n\";"
      "}"
      "if (true) {"
      "  echo \"case 4\\n\";"
      "  function f2() {"
      "    $v = 1;"
      "  }"
      "}"
      "if (true) {"
      "  echo \"case 5\\n\";"
      "} else {"
      "  echo \"case 6\\n\";"
      "}"
      "if (true) {"
      "  echo \"case 7\\n\";"
      "} else {"
      "  echo \"case 8\\n\";"
      "  function f3() {"
      "    $v = 1;"
      "  }"
      "}"
      "if ($a) {"
      "  echo \"case 9\\n\";"
      "} elseif (true) {"
      "  echo \"case 10\\n\";"
      "  function f4() {"
      "    $v = 1;"
      "  }"
      "} else {"
      "  echo \"case 11\\n\";"
      "}"
      "if ($a) {"
      "  echo \"case 12\\n\";"
      "} elseif (true) {"
      "  echo \"case 13\\n\";"
      "} else {"
      "  echo \"case 14\\n\";"
      "  function f5() {"
      "    $v = 1;"
      "  }"
      "}"
      "if ($a) {"
      "  echo \"case 15\\n\";"
      "} elseif (true) {"
      "  echo \"case 16\\n\";"
      "} elseif (true) {"
      "  echo \"case 17\\n\";"
      "  function f6() {"
      "    $v = 1;"
      "  }"
      "} else {"
      "  echo \"case 18\\n\";"
      "}"
      "if ($a) {"
      "  echo \"case 19\\n\";"
      "} elseif (true) {"
      "  echo \"case 20\\n\";"
      "} elseif (true) {"
      "  echo \"case 21\\n\";"
      "  function f7() {"
      "    $v = 1;"
      "  }"
      "}"
      "if ($a) {"
      "  echo \"case 22\\n\";"
      "} elseif (true) {"
      "  echo \"case 23\\n\";"
      "} elseif (false) {"
      "  echo \"case 24\\n\";"
      "  function f8() {"
      "    $v = 1;"
      "  }"
      "} else {"
      "  echo \"case 25\\n\";"
      "}"
      "if ($a) {"
      "  echo \"case 26\\n\";"
      "} elseif (true) {"
      "  echo \"case 27\\n\";"
      "} elseif (false) {"
      "  echo \"case 28\\n\";"
      "  function f9() {"
      "    $v = 1;"
      "  }"
      "}");

  MVCR("<?php "
      "if (true) {"
      "  function foo() { echo \"foo\\n\"; }"
      "} else if (false) {"
      "  function bar() { echo \"bar\\n\"; }"
      "}");

  MVCR("<?php "
      "if (true) {"
      "  function foo() { echo \"foo\\n\"; }"
      "} elseif (false) {"
      "  function bar() { echo \"bar\\n\"; }"
      "}");

  MVCR("<?php "
       "function test($a,$b = 0) {"
       "  if ($a == 2) {"
       "    if ($b == 1) {"
       "      return;"
       "    }"
       "    $a = 5;"
       "  }"
       "  if ($a == 3) {"
       "    var_dump($a);"
       "  }"
       "}"
       "test(3);");

  return true;
}

bool TestCodeRun::TestBreakStatement() {
  MVCR("<?php "
      "$arr = array('one', 'two', 'three', 'four', 'stop', 'five');"
      "while (list(, $val) = each($arr)) {"
      "    if ($val == 'stop') {"
      "        break;"
      "    }"
      "    echo \"$val\\n\";"
      "}"
      ""
      "$i = 0;"
      "while (++$i) {"
      "    switch ($i) {"
      "    case 5:"
      "        echo \"At 5\\n\";"
      "        break 1;"
      "    case 10:"
      "        echo \"At 10; quitting\\n\";"
      "        break 2;"
      "    default:"
      "        break;"
      "    }"
      "}");

  return true;
}

bool TestCodeRun::TestContinueStatement() {
  MVCR("<?php "
      "for ($i = 0;$i<3;$i++) {"
      "  echo \"Start Of I loop\\n\";"
      "  for ($j=0;;$j++) {"
      "    if ($j >= 2) continue 2;"
      "      echo \"I : $i J : $j\".\"\\n\";"
      "  }"
      "  echo \"End\\n\";"
      "}"
      "for ($i = 0;$i<10;$i++) {"
      "  if ($i % 2 == 0) continue 1;"
      "  echo $i . \"\\n\";"
      "}"
      "for ($i = 0;$i<10;$i++) {"
      "  if ($i % 2 == 0) continue;"
      "  echo $i . \"\\n\";"
      "}");

  MVCR("<?php "
      "for ($i1 = 0; $i1 < 2; $i1++) {"
      "  for ($i2 = 0; $i2 < 2; $i2++) {"
      "    switch ($i2 % 2) {"
      "      case 0:"
      "        continue;"
      "        break;"
      "    }"
      "    print \"[\" . $i2 . \"]\\n\";"
      "  }"
      "  print $i1 . \"\\n\";"
      "}");

  return true;
}

bool TestCodeRun::TestReturnStatement() {
  MVCR("<?php "
      "function foo1($a) {"
      "  if ($a) return \"ok\";"
      "}"
      "function foo2($a) {"
      "  if ($a > 1) return;"
      "  if ($a == 1) return 1;"
      "}"
      "function foo3($a) {"
      "  if ($a > 1) return;"
      "  if ($a == 1) return;"
      "}"
      "function bar() {"
      "  $v1 = foo1(0);"
      "  var_dump($v1);"
      "  $v2 = foo2(0);"
      "  var_dump($v2);"
      "  $v3 = foo3(0);"
      "  var_dump($v3);"
      "}"
      "bar();");


  MVCR("<?php "
      "class q {}"
      "function g() {"
      "  return null;"
      "  return new q;"
      "}"
      "function f() {"
      "  return;"
      "  return new q;"
      "}"
      "var_dump(g());"
      "var_dump(f());"
      );
  return true;
}

bool TestCodeRun::TestAdd() {
  MVCR("<?php "
      "printf(\"%s\\n\", 30 + 30);"
      "printf(\"%s\\n\", \"30\" + 30);"
      "printf(\"%s\\n\", 30 + \"30\");"
      "printf(\"%s\\n\", \"30\" + \"30\");"
      );

  MVCR("<?php "
      "$a = \"123.456\" + 123;"
      "var_dump($a);"
      "$a = \"123.456\" + 456.123;"
      "var_dump($a);"
      "$a = \"123.456\" + \"123\";"
      "var_dump($a);"
      "$a = \"123.456\" + \"456.123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a += 123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a += 456.123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a += \"123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a += \"456.123\";"
      "var_dump($a);"
      );

  MVCR("<?php "
      "var_dump(1.7976931348623157e+308 + 1.7976931348623157e+308);"
      );
  return true;
}

bool TestCodeRun::TestMinus() {
  MVCR("<?php "
      "printf(\"%s\\n\", 30 - 30);"
      "printf(\"%s\\n\", \"30\" - 30);"
      "printf(\"%s\\n\", 30 - \"30\");"
      "printf(\"%s\\n\", \"30\" - \"30\");"
      );
  MVCR("<?php "
      "$a = \"123.456\" - 123;"
      "var_dump($a);"
      "$a = \"123.456\" - 456.123;"
      "var_dump($a);"
      "$a = \"123.456\" - \"123\";"
      "var_dump($a);"
      "$a = \"123.456\" - \"456.123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a -= 123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a -= 456.123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a -= \"123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a -= \"456.123\";"
      "var_dump($a);"
      );

  return true;
}

bool TestCodeRun::TestMultiply() {
  MVCR("<?php "
      "printf(\"%s\\n\", 30 * 30);"
      "printf(\"%s\\n\", \"30\" * 30);"
      "printf(\"%s\\n\", 30 * \"30\");"
      "printf(\"%s\\n\", \"30\" * \"30\");"
      );
  MVCR("<?php "
      "$a = \"123.456\" * 123;"
      "var_dump($a);"
      "$a = \"123.456\" * 456.123;"
      "var_dump($a);"
      "$a = \"123.456\" * \"123\";"
      "var_dump($a);"
      "$a = \"123.456\" * \"456.123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a *= 123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a *= 456.123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a *= \"123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a *= \"456.123\";"
      "var_dump($a);"
      );

  return true;
}

bool TestCodeRun::TestDivide() {
  MVCR("<?php "
      "printf(\"%s\\n\", 30 / 30);"
      "printf(\"%s\\n\", \"30\" / 30);"
      "printf(\"%s\\n\", 30 / \"30\");"
      "printf(\"%s\\n\", \"30\" / \"30\");"
      );
  MVCR("<?php "
      "$a = \"123.456\" / 123;"
      "var_dump($a);"
      "$a = \"123.456\" / 456.123;"
      "var_dump($a);"
      "$a = \"123.456\" / \"123\";"
      "var_dump($a);"
      "$a = \"123.456\" / \"456.123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a /= 123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a /= 456.123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a /= \"123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a /= \"456.123\";"
      "var_dump($a);"
      "$a = \"123\" / 123;"
      "var_dump($a);"
      "$a = \"123\" / \"123\";"
      "var_dump($a);"
      "$a = \"321\" / 123;"
      "var_dump($a);"
      "$a = \"321\" / 123.456;"
      "var_dump($a);"
      );

  return true;
}

bool TestCodeRun::TestModulus() {
  MVCR("<?php "
      "printf(\"%s\\n\", 30 % 30);"
      "printf(\"%s\\n\", \"30\" % 30);"
      "printf(\"%s\\n\", 30 % \"30\");"
      "printf(\"%s\\n\", \"30\" % \"30\");"
      );
  MVCR("<?php "
      "$a = \"123.456\" % 123;"
      "var_dump($a);"
      "$a = \"123.456\" % 456.123;"
      "var_dump($a);"
      "$a = \"123.456\" % \"123\";"
      "var_dump($a);"
      "$a = \"123.456\" % \"456.123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a %= 123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a %= 456.123;"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a %= \"123\";"
      "var_dump($a);"
      "$a = \"123.456\";"
      "$a %= \"456.123\";"
      "var_dump($a);"
      "$a = \"123\" % 123;"
      "var_dump($a);"
      "$a = \"123\" % \"123\";"
      "var_dump($a);"
      "$a = \"321\" % 123;"
      "var_dump($a);"
      "$a = \"321\" % 123.456;"
      "var_dump($a);"
      );
  MVCR("<?php "
      "$a = 1 % 9223372036854775807;"
      "var_dump($a);");

  return true;
}

bool TestCodeRun::TestOperationTypes() {
  MVCR("<?php "
      "$a = null; var_dump(+$a);"
      "$a = null; var_dump(-$a);"
      ""
      "$a = null; $b = null; var_dump($a + $b);"
      "$a = null; $b = true; var_dump($a + $b);"
      "$a = null; $b = 0; var_dump($a + $b);"
      "$a = null; $b = 1; var_dump($a + $b);"
      "$a = null; $b = 1.0; var_dump($a + $b);"
      "$a = null; $b = '1.0'; var_dump($a + $b);"
      "$a = null; $b = 'foo'; var_dump($a + $b);"
      "$a = null; $b = new Exception(); var_dump($a + $b);"
      "$a = null; $b = null; $b = 0; var_dump($a + $b);"
      "$a = null; $b = null; $b = 1; var_dump($a + $b);"
      "$a = null; $b = null; $b = 1.0; var_dump($a + $b);"
      "$a = null; $b = null; $b = '1.0'; var_dump($a + $b);"
      "$a = null; $b = null; $b = 'foo'; var_dump($a + $b);"
      "$a = null; $b = null; $b = new Exception(); var_dump($a + $b);"
      "$a = null; $b = null; $a += $b; var_dump($a);"
      "$a = null; $b = true; $a += $b; var_dump($a);"
      "$a = null; $b = 0; $a += $b; var_dump($a);"
      "$a = null; $b = 1; $a += $b; var_dump($a);"
      "$a = null; $b = 1.0; $a += $b; var_dump($a);"
      "$a = null; $b = '1.0'; $a += $b; var_dump($a);"
      "$a = null; $b = 'foo'; $a += $b; var_dump($a);"
      "$a = null; $b = new Exception(); $a += $b; var_dump($a);"
      "$a = null; $b = null; $b = 0; $a += $b; var_dump($a);"
      "$a = null; $b = null; $b = 1; $a += $b; var_dump($a);"
      "$a = null; $b = null; $b = 1.0; $a += $b; var_dump($a);"
      "$a = null; $b = null; $b = '1.0'; $a += $b; var_dump($a);"
      "$a = null; $b = null; $b = 'foo'; $a += $b; var_dump($a);"
      "$a = null; $b = null; $b = new Exception(); $a += $b; var_dump($a);"
      ""
      "$a = null; $b = null; var_dump($a - $b);"
      "$a = null; $b = true; var_dump($a - $b);"
      "$a = null; $b = 0; var_dump($a - $b);"
      "$a = null; $b = 1; var_dump($a - $b);"
      "$a = null; $b = 1.0; var_dump($a - $b);"
      "$a = null; $b = '1.0'; var_dump($a - $b);"
      "$a = null; $b = 'foo'; var_dump($a - $b);"
      "$a = null; $b = new Exception(); var_dump($a - $b);"
      "$a = null; $b = null; $b = 0; var_dump($a - $b);"
      "$a = null; $b = null; $b = 1; var_dump($a - $b);"
      "$a = null; $b = null; $b = 1.0; var_dump($a - $b);"
      "$a = null; $b = null; $b = '1.0'; var_dump($a - $b);"
      "$a = null; $b = null; $b = 'foo'; var_dump($a - $b);"
      "$a = null; $b = null; $b = new Exception(); var_dump($a - $b);"
      "$a = null; $b = null; $a -= $b; var_dump($a);"
      "$a = null; $b = true; $a -= $b; var_dump($a);"
      "$a = null; $b = 0; $a -= $b; var_dump($a);"
      "$a = null; $b = 1; $a -= $b; var_dump($a);"
      "$a = null; $b = 1.0; $a -= $b; var_dump($a);"
      "$a = null; $b = '1.0'; $a -= $b; var_dump($a);"
      "$a = null; $b = 'foo'; $a -= $b; var_dump($a);"
      "$a = null; $b = new Exception(); $a -= $b; var_dump($a);"
      "$a = null; $b = null; $b = 0; $a -= $b; var_dump($a);"
      "$a = null; $b = null; $b = 1; $a -= $b; var_dump($a);"
      "$a = null; $b = null; $b = 1.0; $a -= $b; var_dump($a);"
      "$a = null; $b = null; $b = '1.0'; $a -= $b; var_dump($a);"
      "$a = null; $b = null; $b = 'foo'; $a -= $b; var_dump($a);"
      "$a = null; $b = null; $b = new Exception(); $a -= $b; var_dump($a);"
      ""
      "$a = null; $b = null; var_dump($a * $b);"
      "$a = null; $b = true; var_dump($a * $b);"
      "$a = null; $b = 0; var_dump($a * $b);"
      "$a = null; $b = 1; var_dump($a * $b);"
      "$a = null; $b = 1.0; var_dump($a * $b);"
      "$a = null; $b = '1.0'; var_dump($a * $b);"
      "$a = null; $b = 'foo'; var_dump($a * $b);"
      "$a = null; $b = new Exception(); var_dump($a * $b);"
      "$a = null; $b = null; $b = 0; var_dump($a * $b);"
      "$a = null; $b = null; $b = 1; var_dump($a * $b);"
      "$a = null; $b = null; $b = 1.0; var_dump($a * $b);"
      "$a = null; $b = null; $b = '1.0'; var_dump($a * $b);"
      "$a = null; $b = null; $b = 'foo'; var_dump($a * $b);"
      "$a = null; $b = null; $b = new Exception(); var_dump($a * $b);"
      "$a = null; $b = null; $a *= $b; var_dump($a);"
      "$a = null; $b = true; $a *= $b; var_dump($a);"
      "$a = null; $b = 0; $a *= $b; var_dump($a);"
      "$a = null; $b = 1; $a *= $b; var_dump($a);"
      "$a = null; $b = 1.0; $a *= $b; var_dump($a);"
      "$a = null; $b = '1.0'; $a *= $b; var_dump($a);"
      "$a = null; $b = 'foo'; $a *= $b; var_dump($a);"
      "$a = null; $b = new Exception(); $a *= $b; var_dump($a);"
      "$a = null; $b = null; $b = 0; $a *= $b; var_dump($a);"
      "$a = null; $b = null; $b = 1; $a *= $b; var_dump($a);"
      "$a = null; $b = null; $b = 1.0; $a *= $b; var_dump($a);"
      "$a = null; $b = null; $b = '1.0'; $a *= $b; var_dump($a);"
      "$a = null; $b = null; $b = 'foo'; $a *= $b; var_dump($a);"
      "$a = null; $b = null; $b = new Exception(); $a *= $b; var_dump($a);"
      ""
      "$a = null; $b = true; var_dump($a / $b);"
      "$a = null; $b = 1; var_dump($a / $b);"
      "$a = null; $b = 1.0; var_dump($a / $b);"
      "$a = null; $b = '1.0'; var_dump($a / $b);"
      "$a = null; $b = null; $b = 1; var_dump($a / $b);"
      "$a = null; $b = null; $b = 1.0; var_dump($a / $b);"
      "$a = null; $b = null; $b = '1.0'; var_dump($a / $b);"
      "$a = null; $b = true; $a /= $b; var_dump($a);"
      "$a = null; $b = 1; $a /= $b; var_dump($a);"
      "$a = null; $b = 1.0; $a /= $b; var_dump($a);"
      "$a = null; $b = '1.0'; $a /= $b; var_dump($a);"
      "$a = null; $b = null; $b = 1; $a /= $b; var_dump($a);"
      "$a = null; $b = null; $b = 1.0; $a /= $b; var_dump($a);"
      "$a = null; $b = null; $b = '1.0'; $a /= $b; var_dump($a);"
      ""
      "$a = true; $b = null; var_dump($a + $b);"
      "$a = true; $b = true; var_dump($a + $b);"
      "$a = true; $b = 0; var_dump($a + $b);"
      "$a = true; $b = 1; var_dump($a + $b);"
      "$a = true; $b = 1.0; var_dump($a + $b);"
      "$a = true; $b = '1.0'; var_dump($a + $b);"
      "$a = true; $b = 'foo'; var_dump($a + $b);"
      "$a = true; $b = new Exception(); var_dump($a + $b);"
      "$a = true; $b = null; $b = 0; var_dump($a + $b);"
      "$a = true; $b = null; $b = 1; var_dump($a + $b);"
      "$a = true; $b = null; $b = 1.0; var_dump($a + $b);"
      "$a = true; $b = null; $b = '1.0'; var_dump($a + $b);"
      "$a = true; $b = null; $b = 'foo'; var_dump($a + $b);"
      "$a = true; $b = null; $b = new Exception(); var_dump($a + $b);"
      "$a = true; $b = null; $a += $b; var_dump($a);"
      "$a = true; $b = true; $a += $b; var_dump($a);"
      "$a = true; $b = 0; $a += $b; var_dump($a);"
      "$a = true; $b = 1; $a += $b; var_dump($a);"
      "$a = true; $b = 1.0; $a += $b; var_dump($a);"
      "$a = true; $b = '1.0'; $a += $b; var_dump($a);"
      "$a = true; $b = 'foo'; $a += $b; var_dump($a);"
      "$a = true; $b = new Exception(); $a += $b; var_dump($a);"
      "$a = true; $b = null; $b = 0; $a += $b; var_dump($a);"
      "$a = true; $b = null; $b = 1; $a += $b; var_dump($a);"
      "$a = true; $b = null; $b = 1.0; $a += $b; var_dump($a);"
      "$a = true; $b = null; $b = '1.0'; $a += $b; var_dump($a);"
      "$a = true; $b = null; $b = 'foo'; $a += $b; var_dump($a);"
      "$a = true; $b = null; $b = new Exception(); $a += $b; var_dump($a);"
      ""
      "$a = true; $b = null; var_dump($a - $b);"
      "$a = true; $b = true; var_dump($a - $b);"
      "$a = true; $b = 0; var_dump($a - $b);"
      "$a = true; $b = 1; var_dump($a - $b);"
      "$a = true; $b = 1.0; var_dump($a - $b);"
      "$a = true; $b = '1.0'; var_dump($a - $b);"
      "$a = true; $b = 'foo'; var_dump($a - $b);"
      "$a = true; $b = new Exception(); var_dump($a - $b);"
      "$a = true; $b = null; $b = 0; var_dump($a - $b);"
      "$a = true; $b = null; $b = 1; var_dump($a - $b);"
      "$a = true; $b = null; $b = 1.0; var_dump($a - $b);"
      "$a = true; $b = null; $b = '1.0'; var_dump($a - $b);"
      "$a = true; $b = null; $b = 'foo'; var_dump($a - $b);"
      "$a = true; $b = null; $b = new Exception(); var_dump($a - $b);"
      "$a = true; $b = null; $a -= $b; var_dump($a);"
      "$a = true; $b = true; $a -= $b; var_dump($a);"
      "$a = true; $b = 0; $a -= $b; var_dump($a);"
      "$a = true; $b = 1; $a -= $b; var_dump($a);"
      "$a = true; $b = 1.0; $a -= $b; var_dump($a);"
      "$a = true; $b = '1.0'; $a -= $b; var_dump($a);"
      "$a = true; $b = 'foo'; $a -= $b; var_dump($a);"
      "$a = true; $b = new Exception(); $a -= $b; var_dump($a);"
      "$a = true; $b = null; $b = 0; $a -= $b; var_dump($a);"
      "$a = true; $b = null; $b = 1; $a -= $b; var_dump($a);"
      "$a = true; $b = null; $b = 1.0; $a -= $b; var_dump($a);"
      "$a = true; $b = null; $b = '1.0'; $a -= $b; var_dump($a);"
      "$a = true; $b = null; $b = 'foo'; $a -= $b; var_dump($a);"
      "$a = true; $b = null; $b = new Exception(); $a -= $b; var_dump($a);"
      ""
      "$a = true; $b = null; var_dump($a * $b);"
      "$a = true; $b = true; var_dump($a * $b);"
      "$a = true; $b = 0; var_dump($a * $b);"
      "$a = true; $b = 1; var_dump($a * $b);"
      "$a = true; $b = 1.0; var_dump($a * $b);"
      "$a = true; $b = '1.0'; var_dump($a * $b);"
      "$a = true; $b = 'foo'; var_dump($a * $b);"
      "$a = true; $b = new Exception(); var_dump($a * $b);"
      "$a = true; $b = null; $b = 0; var_dump($a * $b);"
      "$a = true; $b = null; $b = 1; var_dump($a * $b);"
      "$a = true; $b = null; $b = 1.0; var_dump($a * $b);"
      "$a = true; $b = null; $b = '1.0'; var_dump($a * $b);"
      "$a = true; $b = null; $b = 'foo'; var_dump($a * $b);"
      "$a = true; $b = null; $b = new Exception(); var_dump($a * $b);"
      "$a = true; $b = null; $a *= $b; var_dump($a);"
      "$a = true; $b = true; $a *= $b; var_dump($a);"
      "$a = true; $b = 0; $a *= $b; var_dump($a);"
      "$a = true; $b = 1; $a *= $b; var_dump($a);"
      "$a = true; $b = 1.0; $a *= $b; var_dump($a);"
      "$a = true; $b = '1.0'; $a *= $b; var_dump($a);"
      "$a = true; $b = 'foo'; $a *= $b; var_dump($a);"
      "$a = true; $b = new Exception(); $a *= $b; var_dump($a);"
      "$a = true; $b = null; $b = 0; $a *= $b; var_dump($a);"
      "$a = true; $b = null; $b = 1; $a *= $b; var_dump($a);"
      "$a = true; $b = null; $b = 1.0; $a *= $b; var_dump($a);"
      "$a = true; $b = null; $b = '1.0'; $a *= $b; var_dump($a);"
      "$a = true; $b = null; $b = 'foo'; $a *= $b; var_dump($a);"
      "$a = true; $b = null; $b = new Exception(); $a *= $b; var_dump($a);"
      ""
      "$a = true; $b = true; var_dump($a / $b);"
      "$a = true; $b = 1; var_dump($a / $b);"
      "$a = true; $b = 1.0; var_dump($a / $b);"
      "$a = true; $b = '1.0'; var_dump($a / $b);"
      "$a = true; $b = null; $b = 1; var_dump($a / $b);"
      "$a = true; $b = null; $b = 1.0; var_dump($a / $b);"
      "$a = true; $b = null; $b = '1.0'; var_dump($a / $b);"
      "$a = true; $b = true; $a /= $b; var_dump($a);"
      "$a = true; $b = 1; $a /= $b; var_dump($a);"
      "$a = true; $b = 1.0; $a /= $b; var_dump($a);"
      "$a = true; $b = '1.0'; $a /= $b; var_dump($a);"
      "$a = true; $b = null; $b = 1; $a /= $b; var_dump($a);"
      "$a = true; $b = null; $b = 1.0; $a /= $b; var_dump($a);"
      "$a = true; $b = null; $b = '1.0'; $a /= $b; var_dump($a);");
  MVCR("<?php "
      "var_dump(1 / 1.7976931348623157e+308 || false);");
/*
  Fails under release build due to g++ optimization
  MVCR("<?php "
      "var_dump((1.7976931348623157e+308 + 1.7976931348623157e+308) << 0);");
*/
  MVCR("<?php "
      "function foo() {"
      "  return '1';"
      "}"
      "function bar() {"
      "  $a = 1;"
      "  $a += foo();"
      "  var_dump($a);"
      "  $b = 1;"
      "  $b -= foo();"
      "  var_dump($b);"
      "}"
      "bar();");

  MVCR("<?php "
       "$a = null;"
       "$a += new Exception();"
       "var_dump($a);");

  return true;
}

#define UNARY_OP_DATA(op) \
  "<?php " \
  #op"(!null);" \
  #op"(!true);" \
  #op"(!false);" \
  #op"(!0);" \
  #op"(!1);" \
  #op"(!1.2);" \
  #op"(!\"\");" \
  #op"(!\"0\");" \
  #op"(!\"1\");" \
  #op"(!\"1.2\");" \
  #op"(!'');" \
  #op"(!'0');" \
  #op"(!'1');" \
  #op"(!'1.2');" \
  #op"(!__LINE__);" \
  #op"(!__FUNCTION__);" \
  #op"(!0x10);" \
  #op"(!010);" \
  #op"(!\"0x10\");" \
  #op"(!\"010\");" \
  #op"(+null);" \
  #op"(+true);" \
  #op"(+false);" \
  #op"(+0);" \
  #op"(+1);" \
  #op"(+1.2);" \
  #op"(+\"\");" \
  #op"(+\"0\");" \
  #op"(+\"1\");" \
  #op"(+\"1.2\");" \
  #op"(+'');" \
  #op"(+'0');" \
  #op"(+'1');" \
  #op"(+'1.2');" \
  #op"(+__LINE__);" \
  #op"(+__FUNCTION__);" \
  #op"(+0x10);" \
  #op"(+010);" \
  #op"(+\"0x10\");" \
  #op"(+\"010\");" \
  #op"(-null);" \
  #op"(-true);" \
  #op"(-false);" \
  #op"(-0);" \
  #op"(-1);" \
  #op"(-1.2);" \
  #op"(-\"\");" \
  #op"(-\"0\");" \
  #op"(-\"1\");" \
  #op"(-\"1.2\");" \
  #op"(-'');" \
  #op"(-'0');" \
  #op"(-'1');" \
  #op"(-'1.2');" \
  #op"(-__LINE__);" \
  #op"(-__FUNCTION__);" \
  #op"(-0x10);" \
  #op"(-010);" \
  #op"(-\"0x10\");" \
  #op"(-\"010\");" \
  #op"(~0);" \
  #op"(~1);" \
  #op"(~1.2);" \
  #op"(~\"\");" \
  #op"(~\"0\");" \
  #op"(~\"1\");" \
  #op"(~\"1.2\");" \
  #op"(~'');" \
  #op"(~'0');" \
  #op"(~'1');" \
  #op"(~'1.2');" \
  #op"(~__LINE__);" \
  #op"(~__FUNCTION__);" \
  #op"(~0x10);" \
  #op"(~010);" \
  #op"(~\"0x10\");" \
  #op"(~\"010\");" \
  #op"((null));" \
  #op"((true));" \
  #op"((false));" \
  #op"((0));" \
  #op"((1));" \
  #op"((1.2));" \
  #op"((\"\"));" \
  #op"((\"0\"));" \
  #op"((\"1\"));" \
  #op"((\"1.2\"));" \
  #op"((''));" \
  #op"(('0'));" \
  #op"(('1'));" \
  #op"(('1.2'));" \
  #op"((__LINE__));" \
  #op"((__FUNCTION__));" \
  #op"((0x10));" \
  #op"((010));" \
  #op"((\"0x10\"));" \
  #op"((\"010\"));" \
  #op"((int)null);" \
  #op"((int)true);" \
  #op"((int)false);" \
  #op"((int)0);" \
  #op"((int)1);" \
  #op"((int)1.2);" \
  #op"((int)\"\");" \
  #op"((int)\"0\");" \
  #op"((int)\"1\");" \
  #op"((int)\"1.2\");" \
  #op"((int)'');" \
  #op"((int)'0');" \
  #op"((int)'1');" \
  #op"((int)'1.2');" \
  #op"((int)__LINE__);" \
  #op"((int)__FUNCTION__);" \
  #op"((int)0x10);" \
  #op"((int)010);" \
  #op"((int)\"0x10\");" \
  #op"((int)\"010\");" \
  #op"((integer)null);" \
  #op"((integer)true);" \
  #op"((integer)false);" \
  #op"((integer)0);" \
  #op"((integer)1);" \
  #op"((integer)1.2);" \
  #op"((integer)\"\");" \
  #op"((integer)\"0\");" \
  #op"((integer)\"1\");" \
  #op"((integer)\"1.2\");" \
  #op"((integer)'');" \
  #op"((integer)'0');" \
  #op"((integer)'1');" \
  #op"((integer)'1.2');" \
  #op"((integer)__LINE__);" \
  #op"((integer)__FUNCTION__);" \
  #op"((integer)0x10);" \
  #op"((integer)010);" \
  #op"((integer)\"0x10\");" \
  #op"((integer)\"010\");" \
  #op"((bool)null);" \
  #op"((bool)true);" \
  #op"((bool)false);" \
  #op"((bool)0);" \
  #op"((bool)1);" \
  #op"((bool)1.2);" \
  #op"((bool)\"\");" \
  #op"((bool)\"0\");" \
  #op"((bool)\"1\");" \
  #op"((bool)\"1.2\");" \
  #op"((bool)'');" \
  #op"((bool)'0');" \
  #op"((bool)'1');" \
  #op"((bool)'1.2');" \
  #op"((bool)__LINE__);" \
  #op"((bool)__FUNCTION__);" \
  #op"((bool)0x10);" \
  #op"((bool)010);" \
  #op"((bool)\"0x10\");" \
  #op"((bool)\"010\");" \
  #op"((boolean)null);" \
  #op"((boolean)true);" \
  #op"((boolean)false);" \
  #op"((boolean)0);" \
  #op"((boolean)1);" \
  #op"((boolean)1.2);" \
  #op"((boolean)\"\");" \
  #op"((boolean)\"0\");" \
  #op"((boolean)\"1\");" \
  #op"((boolean)\"1.2\");" \
  #op"((boolean)'');" \
  #op"((boolean)'0');" \
  #op"((boolean)'1');" \
  #op"((boolean)'1.2');" \
  #op"((boolean)__LINE__);" \
  #op"((boolean)__FUNCTION__);" \
  #op"((boolean)0x10);" \
  #op"((boolean)010);" \
  #op"((boolean)\"0x10\");" \
  #op"((boolean)\"010\");" \
  #op"((float)null);" \
  #op"((float)true);" \
  #op"((float)false);" \
  #op"((float)0);" \
  #op"((float)1);" \
  #op"((float)1.2);" \
  #op"((float)\"\");" \
  #op"((float)\"0\");" \
  #op"((float)\"1\");" \
  #op"((float)\"1.2\");" \
  #op"((float)'');" \
  #op"((float)'0');" \
  #op"((float)'1');" \
  #op"((float)'1.2');" \
  #op"((float)__LINE__);" \
  #op"((float)__FUNCTION__);" \
  #op"((float)0x10);" \
  #op"((float)010);" \
  #op"((float)\"0x10\");" \
  #op"((float)\"010\");" \
  #op"((double)null);" \
  #op"((double)true);" \
  #op"((double)false);" \
  #op"((double)0);" \
  #op"((double)1);" \
  #op"((double)1.2);" \
  #op"((double)\"\");" \
  #op"((double)\"0\");" \
  #op"((double)\"1\");" \
  #op"((double)\"1.2\");" \
  #op"((double)'');" \
  #op"((double)'0');" \
  #op"((double)'1');" \
  #op"((double)'1.2');" \
  #op"((double)__LINE__);" \
  #op"((double)__FUNCTION__);" \
  #op"((double)0x10);" \
  #op"((double)010);" \
  #op"((double)\"0x10\");" \
  #op"((double)\"010\");" \
  #op"((real)null);" \
  #op"((real)true);" \
  #op"((real)false);" \
  #op"((real)0);" \
  #op"((real)1);" \
  #op"((real)1.2);" \
  #op"((real)\"\");" \
  #op"((real)\"0\");" \
  #op"((real)\"1\");" \
  #op"((real)\"1.2\");" \
  #op"((real)'');" \
  #op"((real)'0');" \
  #op"((real)'1');" \
  #op"((real)'1.2');" \
  #op"((real)__LINE__);" \
  #op"((real)__FUNCTION__);" \
  #op"((real)0x10);" \
  #op"((real)010);" \
  #op"((real)\"0x10\");" \
  #op"((real)\"010\");" \
  #op"((string)null);" \
  #op"((string)true);" \
  #op"((string)false);" \
  #op"((string)0);" \
  #op"((string)1);" \
  #op"((string)1.2);" \
  #op"((string)\"\");" \
  #op"((string)\"0\");" \
  #op"((string)\"1\");" \
  #op"((string)\"1.2\");" \
  #op"((string)'');" \
  #op"((string)'0');" \
  #op"((string)'1');" \
  #op"((string)'1.2');" \
  #op"((string)__LINE__);" \
  #op"((string)__FUNCTION__);" \
  #op"((string)0x10);" \
  #op"((string)010);" \
  #op"((string)\"0x10\");" \
  #op"((string)\"010\");" \
  #op"(1.7e+319);" \
  #op"(!1.7e+319);" \
  #op"(+1.7e+319);" \
  #op"(-1.7e+319);" \
  #op"(~1.7e+319);" \
  #op"((1.7e+319));" \
  #op"((int)1.7e+319);" \
  #op"((integer)1.7e+319);" \
  #op"((bool)1.7e+319);" \
  #op"((boolean)1.7e+319);" \
  #op"((float)1.7e+319);" \
  #op"((double)1.7e+319);" \
  #op"((real)1.7e+319);" \
  #op"((string)1.7e+319);" \
  #op"(\"1.7e+319\");" \
  #op"(!\"1.7e+319\");" \
  #op"(+\"1.7e+319\");" \
  #op"(-\"1.7e+319\");" \
  #op"(~\"1.7e+319\");" \
  #op"((\"1.7e+319\"));" \
  #op"((int)\"1.7e+319\");" \
  #op"((integer)\"1.7e+319\");" \
  #op"((bool)\"1.7e+319\");" \
  #op"((boolean)\"1.7e+319\");" \
  #op"((float)\"1.7e+319\");" \
  #op"((double)\"1.7e+319\");" \
  #op"((real)\"1.7e+319\");" \
  #op"((string)\"1.7e+319\");"

#define UNARY_OP_ARRAY_DATA(op) \
  "<?php " \
  #op"(array(\"\\0\" => 1));" \
  #op"(array(\"\\0\" => \"\\0\"));" \
  #op"(array(\"\\0\" => \"\\\\\"));" \
  #op"(array(\"\\0\" => \"\\'\"));" \
  #op"(array(\"\\\\\" => 1));" \
  #op"(array(\"\\\\\" => \"\\0\"));" \
  #op"(array(\"\\\\\" => \"\\\\\"));" \
  #op"(array(\"\\\\\" => \"\\'\"));" \
  #op"(array(\"\\'\" => 1));" \
  #op"(array(\"\\'\" => \"\\0\"));" \
  #op"(array(\"\\'\" => \"\\\\\"));" \
  #op"(array(\"\\'\" => \"\\'\"));" \
  #op"(array(\"\\a\" => \"\\a\"));" \
  #op"(!array(\"\\0\" => \"\\0\"));" \
  #op"((array(\"\\0\" => \"\\0\")));" \
  #op"((int)array(\"\\0\" => \"\\0\"));" \
  #op"((integer)array(\"\\0\" => \"\\0\"));" \
  #op"((bool)array(\"\\0\" => \"\\0\"));" \
  #op"((boolean)array(\"\\0\" => \"\\0\"));" \
  #op"((float)array(\"\\0\" => \"\\0\"));" \
  #op"((double)array(\"\\0\" => \"\\0\"));" \
  #op"((real)array(\"\\0\" => \"\\0\"));" \
  #op"((string)array(\"\\0\" => \"\\0\"));" \
  "$a = \"0x10\";" \
  #op"($a);" \
  #op"(\"\\0\");" \
  "$a = array(\"\\0\" => 1);" \
  #op"($a);" \
  "$a = array(\"\\0\" => \"\\0\");" \
  #op"($a);" \
  "$a = array(\"\\0\" => \"\\\\\");" \
  #op"($a);" \
  "$a = array(\"\\0\" => \"\\'\");" \
  #op"($a);" \
  "$a = array(\"\\\\\" => 1);" \
  #op"($a);" \
  "$a = array(\"\\\\\" => \"\\0\");" \
  #op"($a);" \
  "$a = array(\"\\\\\" => \"\\\\\");" \
  #op"($a);" \
  "$a = array(\"\\\\\" => \"\\'\");" \
  #op"($a);" \
  "$a = array(\"\\'\" => 1);" \
  #op"($a);" \
  "$a = array(\"\\'\" => \"\\0\");" \
  #op"($a);" \
  "$a = array(\"\\'\" => \"\\\\\");" \
  #op"($a);" \
  "$a = array(\"\\'\" => \"\\'\");" \
  #op"($a);" \
  "$a = array(\"\\a\" => \"\\a\");" \
  #op"($a);"

#define UNARY_OP(op) \
  MVCR(UNARY_OP_DATA(op)) \
  MVCR(UNARY_OP_ARRAY_DATA(op))

bool TestCodeRun::TestUnaryOperators() {
  UNARY_OP(var_dump);
  return true;
}

bool TestCodeRun::TestSilenceOperator() {
  MVCR("<?php "
      "@define( 'MARKDOWN_EMPTY_ELEMENT_SUFFIX',  \" />\");");
  return true;
}

bool TestCodeRun::TestPrint() {
  UNARY_OP(echo);
  UNARY_OP(print);
  UNARY_OP(print_r);
  UNARY_OP(serialize);
  return true;
}

bool TestCodeRun::TestVarExport() {
  MVCR(UNARY_OP_DATA(var_export));


  MVCRO(UNARY_OP_ARRAY_DATA(var_export),
  "array (\n"
  "  '' . \"\\0\" . '' => 1,\n"
  ")array (\n"
  "  '' . \"\\0\" . '' => '' . \"\\0\" . '',\n"
  ")array (\n"
  "  '' . \"\\0\" . '' => '\\\\',\n"
  ")array (\n"
  "  '' . \"\\0\" . '' => '\\\\\\\'',\n"
  ")array (\n"
  "  '\\\\' => 1,\n"
  ")array (\n"
  "  '\\\\' => '' . \"\\0\" . '',\n"
  ")array (\n"
  "  '\\\\' => '\\\\',\n"
  ")array (\n"
  "  '\\\\' => '\\\\\\\'',\n"
  ")array (\n"
  "  '\\\\\\\'' => 1,\n"
  ")array (\n"
  "  '\\\\\\\'' => '' . \"\\0\" . '',\n"
  ")array (\n"
  "  '\\\\\\\'' => '\\\\',\n"
  ")array (\n"
  "  '\\\\\\\'' => '\\\\\\\'',\n"
  ")array (\n"
  "  '\\\\a' => '\\\\a',\n"
  ")falsearray (\n"
  "  '' . \"\\0\" . '' => '' . \"\\0\" . '',\n"
  ")11truetrue111'Array''0x10''' . \"\\0\" . ''array (\n"
  "  '' . \"\\0\" . '' => 1,\n"
  ")array (\n"
  "  '' . \"\\0\" . '' => '' . \"\\0\" . '',\n"
  ")array (\n"
  "  '' . \"\\0\" . '' => '\\\\',\n"
  ")array (\n"
  "  '' . \"\\0\" . '' => '\\\\\\\'',\n"
  ")array (\n"
  "  '\\\\' => 1,\n"
  ")array (\n"
  "  '\\\\' => '' . \"\\0\" . '',\n"
  ")array (\n"
  "  '\\\\' => '\\\\',\n"
  ")array (\n"
  "  '\\\\' => '\\\\\\\'',\n"
  ")array (\n"
  "  '\\\\\\\'' => 1,\n"
  ")array (\n"
  "  '\\\\\\\'' => '' . \"\\0\" . '',\n"
  ")array (\n"
  "  '\\\\\\\'' => '\\\\',\n"
  ")array (\n"
  "  '\\\\\\\'' => '\\\\\\\'',\n"
  ")array (\n"
  "  '\\\\a' => '\\\\a',\n"
  ")");
  return true;
}

bool TestCodeRun::TestLocale() {
  MVCRO("<?php "
        "class A { public $a; function __toString() { return $this->a;}} "
        "$a = new A; $a->a = 'a'; $b = new A; $b->a = 'b'; "
        "$arr = array($a, $b); sort($arr, SORT_REGULAR, true); "
        "print ((string)$arr[0]);",
        "a");
  MVCRO("<?php "
        "class A { public $a; }"
        "$a = new A; $a->a = 'a'; $b = new A; $b->a = 'b'; "
        "$arr = array($b, $a);"
        "print $arr[0]->a;"
        "sort($arr, SORT_REGULAR, true); "
        "print $arr[0]->a;",
        "ba");
  MVCRO("<?php "
        "$a = array(1);"
        "$b = array(2);"
        "$arr = array($b, $a);"
        "print $arr[0][0];"
        "asort($arr, SORT_REGULAR, true); "
        "print $arr[0][0];",
        "22");
  MVCR("<?php "
      "$a = array(\"a bc\", \"\\xc1 bc\", \"d ef\");"
      "asort($a);"
      "print_r($a);"
      "$a = array(\"a bc\", \"\\xc1 bc\", \"d ef\");"
      "asort($a, SORT_LOCALE_STRING);"
      "print_r($a);"
      "$a = array(\"a bc\", \"\\xc1 bc\", \"d ef\");"
      "setlocale(LC_ALL, \"pt_PT\");"
      "asort($a);"
      "print_r($a);"
      "$a = array(\"a bc\", \"\\xc1 bc\", \"d ef\");"
      "setlocale(LC_ALL, \"pt_PT\");"
      "asort($a, SORT_LOCALE_STRING);"
      "print_r($a);");
  return true;
}

bool TestCodeRun::TestLogicalOperators() {
  MVCR("<?php "
      "function foo() { echo \"foo\"; }"
      "$a = (false && foo());"
      "$b = (true  || foo());"
      "$c = (false and foo());"
      "$d = (true  or  foo());"
      "$e = ($e || true);"
      "$f = ($f or true);"
      "$g = ($g && false);"
      "$h = ($h and false);"
      "var_dump($a, $b, $c, $d, $e, $f, $g, $h);");
  MVCR("<?php "
       "function foo($a) { echo \"foo\"; return $a; }"
       "$x = true;"
       "$x = $x and foo(false);"
       "var_dump($x);"
       "$x = $x && foo(false);"
       "var_dump($x);"
       "$x = false;"
       "$x = $x or foo(true);"
       "var_dump($x);"
       "$x = $x || foo(true);"
       "var_dump($x);");

  MVCR("<?php var_dump($a || null);");

  return true;
}

bool TestCodeRun::TestGetClass() {
  MVCR("<?php "
      "class foo {"
      "  function bar () {"
      "    var_dump(get_class());"
      "    var_dump(get_class(null));"
      "  }"
      "}"
      "class foo2 extends foo {"
      "}"
      "$f1 = new foo;"
      "$f2 = new foo2;"
      "var_dump(get_class($f1));"
      "$f1->bar();"
      "$f2->bar();"
      "var_dump(get_class(\"qwerty\"));"
      "var_dump(get_class($f1));"
      "var_dump(get_class($f2));"
      );
  MVCR("<?php "
      "abstract class bar {"
      "  public function __construct()"
      "  {"
      "    var_dump(get_class($this));"
      "    var_dump(get_class());"
      "  }"
      "}"
      "class foo extends bar {"
      "}"
      "new foo;"
      );

  return true;
}

bool TestCodeRun::TestGetParentClass() {
  MVCR("<?php "
      "class dad {"
      "  function dad()"
      "  {}"
      "}"
      "class child extends dad {"
      "  function child()"
      "  {"
      "    echo \"I'm \" , get_parent_class($this) , \"'s son\\n\";"
      "  }"
      "}"
      "class child2 extends dad {"
      "  function child2()"
      "  {"
      "    echo \"I'm \" , get_parent_class('child2') , \"'s son too\n\";"
      "  }"
      "}"
      "$foo = new child();"
      "$bar = new child2();"
      );
  MVCR("<?php "
      "interface i {"
      "  function test();"
      "}"
      "class foo implements i {"
      "  function test() {"
      "    var_dump(get_parent_class());"
      "  }"
      "}"
      "class bar extends foo {"
      "  function test_bar() {"
      "    var_dump(get_parent_class());"
      "  }"
      "}"
      "class goo extends bar {"
      "  function test_goo() {"
      "    var_dump(get_parent_class());"
      "  }"
      "}"
      "$bar = new bar;"
      "$foo = new foo;"
      "$goo = new goo;"
      "$foo->test();"
      "$bar->test();"
      "$bar->test_bar();"
      "$goo->test();"
      "$goo->test_bar();"
      "$goo->test_goo();"
      "var_dump(get_parent_class($bar));"
      "var_dump(get_parent_class($foo));"
      "var_dump(get_parent_class($goo));"
      "var_dump(get_parent_class(\"bar\"));"
      "var_dump(get_parent_class(\"foo\"));"
      "var_dump(get_parent_class(\"goo\"));"
      "var_dump(get_parent_class(\"i\"));"
      "var_dump(get_parent_class(\"\"));"
      "var_dump(get_parent_class(\"[[[[\"));"
      "var_dump(get_parent_class(\" \"));"
      "var_dump(get_parent_class(new stdclass));"
      "var_dump(get_parent_class(array()));"
      "var_dump(get_parent_class(1));"
      );

  return true;
}

bool TestCodeRun::TestRedeclaredFunctions() {
  MVCR("<?php "
      "if (true) {"
      "  function test() {"
      "    echo('a');"
      "  }"
      "} else {"
      "  function test() {"
      "    echo('b');"
      "  }"
      "}"
      "test();"
      );

  MVCR("<?php "
       "function test($a, $b, $c, $d, $e, $f, $g = 0) {"
       "  return $a;"
       "}"
       "if (0) { function test($a) {} }"
       "test(1,2,3,4,5,6);");

  return true;
}

bool TestCodeRun::TestRedeclaredClasses() {
  MVCR(
    "<?php\n"
    "$b = 123;\n"
    "if ($b) {\n"
    "  class Exception1 extends Exception {}\n"
    "} else {\n"
    "  class Exception1 extends Exception {}\n"
    "}\n"
    "class Exception2 extends Exception1 {}\n"
    "\n"
    "function foo() {\n"
    "  $e = new Exception();\n"
    "  try {\n"
    "    throw new Exception2();\n"
    "  } catch (Exception $e) {\n"
    "    var_dump($e->getCode());\n"
    "  }\n"
    "}\n"
    "foo();\n"
  );

  MVCR("<?php "
      "class base1 {}"
      "class base2 {}"
      "if (true) {"
      "  class a extends base1 {"
      "    const aconst = \"firstA\";"
      "    const a1const = 0;"
      "    static $astat = 1;"
      "    static $a1stat = 1;"
      "    function __construct() { echo \"first def made\n\"; }"
      "    static function foo() { return 1;}"
      "  }"
      "} else {"
      "  class a extends base2 {"
      "    const aconst = \"secondA\";"
      "    const a2const = 0;"
      "    static $astat = 2;"
      "    static $a2stat = 2;"
      "    function __construct() { echo \"second def made\n\"; }"
      "    static function foo() { return 2;}"
      "  }"
      "}"
      "$foo = \"foo\";"
      "$y = new a;"
      "var_dump(a::foo());"
      "var_dump(a::$foo());"
      "var_dump(call_user_func(array('a','foo')));"
      "var_dump(a::$astat);"
      "var_dump(a::$a1stat);"
      "var_dump(a::aconst);"
      "var_dump(a::a1const);"
      "var_dump(method_exists('a',\"foo\"));"
      "var_dump(method_exists($y,\"foo\"));"
      "var_dump(property_exists(\"a\",\"astat\"));"
      "var_dump(property_exists(\"a\",\"a1stat\"));"
      "var_dump(property_exists(\"a\",\"a2stat\"));"
      "var_dump(get_parent_class($y));"
      "var_dump(is_subclass_of(\"a\", \"base1\"));"
      "var_dump(is_subclass_of(\"a\", \"base2\"));"
      "var_dump(get_object_vars($y));"
      );
  MVCR("<?php "
      "if (true) {"
      "  class base {"
      "    public $baseVal =  'base';"
      "    static $baseStatic = 'baseStat';"
      "    function childProp() { return $this->childProp; }"
      "    function testChildMeth() { return $this->childMeth(); }"
      "    static function baseStatMeth() {"
      "      return 'Base static method';"
      "    }"
      "    function childMeth() { return 'I am base'; }"
      "  }"
      "} else {"
      "  class base {"
      "  }"
      "}"
      "class child1 extends base {"
      "  public $child1Val = 'child1';"
      "  public $childProp = 'IamChild1';"
      "  static $child1Static = 'child1Stat';"
      "  function childMeth() {"
      "    return 'I am child1';"
      "  }"
      "  static function child1StatMeth() {"
      "    return 'Child 1 static method';"
      "  }"
      "  function parentChildMeth() {"
      "    return parent::childMeth();"
      "  }"
      "}"
      "class child2 extends child1 {"
      "  public $child2Val = 'child2';"
      "  public $childProp = 'IamChild2';"
      "  static $child2Static = 'child2Stat';"
      "  static function child2StatMeth() {"
      "    return 'Child 2 static method';"
      "  }"
      "  function childMeth() {"
      "    return 'I am child2';"
      "  }"
      "  function parentChildMeth() {"
      "    return parent::childMeth();"
      "  }"
      "  function testChildMeth2() { return $this->childMeth(); }"
      "}"
      "if (true) {"
      "  class child3 extends child2 {"
      "    public $child3Val = 'child3';"
      "    public $childProp = 'IamChild3';"
      "    static $child3Static = 'child3Stat';"
      "    function childMeth() {"
      "      return 'I am child3';"
      "    }"
      "    static function child3StatMeth() {"
      "      return 'Child 3 static method';"
      "    }"
      "    function parentChildMeth() {"
      "      return parent::childMeth();"
      "    }"
      ""
      "  }"
      "} else {"
      "  class child3 {}"
      "}"
      "function test($val, $exp, $feature) {"
      "  if ($val !== $exp) {"
      "    echo $feature . \" failed. Got:\\n\";"
      "    var_dump($val);"
      "    echo \"But expected:\\n\";"
      "    var_dump($exp);"
      "  } else {"
      "    echo $feature . \" passed\\n\";"
      "  }"
      "}"
      "function run() {"
      "  $base = new base;"
      "  test($base->baseVal, 'base', 'Base object member');"
      "  test(base::$baseStatic, 'baseStat', 'Base static member');"
      "  test(base::baseStatMeth(), 'Base static method', 'Base static method');"
      "  test($base->baseStatMeth(), 'Base static method', 'Base static method obj syntax');"
      "  $child1 = new child1;"
      "  test($child1->baseVal, 'base', 'dRedec inherited property');"
      "  test($child1->child1Val, 'child1', 'dRedec property');"
      "  test($child1->testChildMeth(), 'I am child1', 'dRedec parent->virtual method');"
      "  test($child1->childProp(), 'IamChild1', 'dRedec parent->child prop method');"
      "  test(child1::child1StatMeth(), 'Child 1 static method', 'dRedec static method');"
      "  test(child1::baseStatMeth(), 'Base static method', 'dRedec parent static method');"
      "  test($child1->child1StatMeth(), 'Child 1 static method', 'dRedec static method obj syntax');"
      "  test($child1->baseStatMeth(), 'Base static method', 'dRedec parent static method obj syntax');"
      "  test(child1::$baseStatic, 'baseStat', 'dRedec parent static prop');"
      "  test(child1::$child1Static, 'child1Stat', 'dRedec static prop');"
      "  test($child1->parentChildMeth(), 'I am base', 'dRedec parent method');"
      "  $child2 = new child2;"
      "  test($child2->baseVal, 'base', 'ddRedec grandparent property');"
      "  test($child2->child1Val, 'child1', 'ddRedec parent property');"
      "  test($child2->child2Val, 'child2', 'ddRedec property');"
      "  test($child2->testChildMeth(), 'I am child2', 'ddRedec grandparent->virtual method');"
      "  test($child2->testChildMeth2(), 'I am child2', 'ddRedec parent->virtual method');"
      "  test($child2->childProp(), 'IamChild2', 'ddRedec grandparent->child prop method');"
      "  test(child2::baseStatMeth(), 'Base static method', 'ddRedec grandparent static method');"
      "  test(child2::child1StatMeth(), 'Child 1 static method', 'ddRedec parent static method');"
      "  test(child2::child2StatMeth(), 'Child 2 static method', 'ddRedec static method');"
      "  test($child2->baseStatMeth(), 'Base static method', 'ddRedec grandparent static method obj syntax');"
      "  test($child2->child1StatMeth(), 'Child 1 static method', 'ddRedec parent static method obj syntax');"
      "  test($child2->child2StatMeth(), 'Child 2 static method', 'ddRedec static method obj syntax');"
      "  test(child2::$baseStatic, 'baseStat', 'ddRedec grandparent static prop');"
      "  test(child2::$child1Static, 'child1Stat', 'ddRedec parent static prop');"
      "  test(child2::$child2Static, 'child2Stat', 'ddRedec static prop');"
      "  test($child2->parentChildMeth(), 'I am child1', 'ddRedec parent method');"
      "  $child3 = new child3;"
      "  test($child3->baseVal, 'base', 'RddRedec greatgrandparent property');"
      "  test($child3->child1Val, 'child1', 'RddRedec grandparent property');"
      "  test($child3->child2Val, 'child2', 'RddRedec parent property');"
      "  test($child3->child3Val, 'child3', 'RddRedec property');"
      "  test($child3->testChildMeth(), 'I am child3', 'RddRedec greatgrandparent->virtual method');"
      "  test($child3->testChildMeth2(), 'I am child3', 'RddRedec grandparent->virtual method');"
      "  test($child3->childProp(), 'IamChild3', 'RddRedec greatgrandparent->child prop method');"
      "  test(child3::baseStatMeth(), 'Base static method', 'RddRedec greatgrandparent static method');"
      "  test(child3::child1StatMeth(), 'Child 1 static method', 'RddRedec grandparent static method');"
      "  test(child3::child2StatMeth(), 'Child 2 static method', 'RddRedec parent static method');"
      "  test(child3::child3StatMeth(), 'Child 3 static method', 'RddRedec static method');"
      "  test($child3->baseStatMeth(), 'Base static method', 'RddRedec greatgrandparent static method obj syntax');"
      "  test($child3->child1StatMeth(), 'Child 1 static method', 'RddRedec grandparent static method obj syntax');"
      "  test($child3->child2StatMeth(), 'Child 2 static method', 'RddRedec parent static method obj syntax');"
      "  test($child3->child3StatMeth(), 'Child 3 static method', 'RddRedec static method obj syntax');"
      "  test(child3::$baseStatic, 'baseStat', 'RddRedec greatgrandparent static prop');"
      "  test(child3::$child1Static, 'child1Stat', 'RddRedec grandparent static prop');"
      "  test(child3::$child2Static, 'child2Stat', 'RddRedec parent static prop');"
      "  test(child3::$child3Static, 'child3Stat', 'RddRedec static prop');"
      "  test($child3->parentChildMeth(), 'I am child2', 'RddRedec parent method');"
      "}"
      "run();"
      );
  MVCR("<?php "
      "$i = 1;"
      "if ($i == 1) {"
      "  class foo {"
      "    function foo() {"
      "      echo \"foo 1\";"
      "    }"
      "    function bar() {"
      "      echo \"bar 1\";"
      "    }"
      "  }"
      "} else {"
      "  class foo {"
      "    function foo() {"
      "      echo \"foo 2\";"
      "    }"
      "    function bar() {"
      "      echo \"bar 2\";"
      "    }"
      "  }"
      "}"
      "$t = new foo();"
      "$t->foo();"
      "$t->bar();");
  MVCR("<?php "
      "$i = 2;"
      "if ($i == 1) {"
      "  class foo {"
      "    function foo() {"
      "      echo \"foo 1\";"
      "    }"
      "    function bar() {"
      "      echo \"bar 1\";"
      "    }"
      "  }"
      "} else {"
      "  class foo {"
      "    function foo() {"
      "      echo \"foo 2\";"
      "    }"
      "    function bar() {"
      "      echo \"bar 2\";"
      "    }"
      "  }"
      "}"
      "$t = new foo();"
      "$t->foo();"
      "$t->bar();");
  MVCR("<?php "
      "class a {"
      "  public static function x() {"
      "    echo 'x';"
      "  }"
      "}"
      "if (0) {"
      "  class b {"
      "  }"
      "}"
      "class b extends a{"
      "  public static function z() {"
      "    self::x();"
      "  }"
      "}"
      "b::x();");

  MVCR("<?php "
      "function f($i) {"
      "  $j = 1;"
      "  var_dump($j);"
      "  if ($i == 1) {"
      "    class p {"
      "      public $data1;"
      "    }"
      "    class c extends p {"
      "    }"
      "    function r() { echo \"r1\\n\"; }"
      "  } else {"
      "    class p {"
      "      public $data2;"
      "    }"
      "    class c extends p {"
      "    }"
      "    function r() { echo \"r2\\n\"; }"
      "  }"
      "}"
      "if ($i == 1) {"
      "  class p {"
      "    public $data1;"
      "  }"
      "  class c extends p {"
      "  }"
      "  function r() { echo \"r3\\n\"; }"
      "}"
      "f(1);"
      "$obj = new p();"
      "var_dump($obj);"
      "$obj = new c();"
      "var_dump($obj);"
      "r();");
  MVCR("<?php "
      "class A {"
      "  protected static function foo() {}"
      "}"
      "if (false) {"
      "  class A{"
      "    protected static function foo() {}"
      "  }"
      "}"
      "class B extends A{}"
      ""
      "class C extends B {"
      "  function x() {"
      "    self::FOO();"
      "  }"
      "}");

  MVCR("<?php "
      "class A {"
      "  static function foo() {"
      "    static $z = 0;"
      "    $z++;"
      "    var_dump($z);"
      "  }"
      "}"
      "if (false) {"
      "  class A{}"
      "}"
      "class B extends A{}"
      "class C extends B {}"
      "A::foo();"
      "B::foo();"
      "C::foo();");

  MVCR("<?php "
       "$a = 1;"
       "if ($a) {"
       "interface A {}"
       "} else {"
       "interface A {}"
       "}"
       "if ($a) {"
       "interface B extends A {}"
       "} else {"
       "interface B extends A {}"
       "}"
       "class Z implements B {}");
  MVCR("<?php "
       "$ok = 1;"
       "if ($ok) {"
       "class A {"
       "const FOO = 'test';"
       "public $a = A::FOO;"
       "}"
       "} else {"
       "class A {"
       "const FOO = 'test';"
       "public $a = A::FOO;"
       "}"
       "}");
  MVCR("<?php "
       "class B {};"
       "if (0) {"
       "  class B {}"
       "}"
       "class A extends B {"
       "  function __call($name,$args) { echo 'A::$name\n'; }"
       "};"
       "$a = new A;"
       "call_user_func_array(array($a, 'foo'), array());");
  MVCR("<?php "
       "class A extends Exception { public $a = 1; }"
       "if (0) {"
       "  class A { public $a = 2; }"
       "}"
       "function test() {"
       "try {"
       "  throw new A;"
       "} catch (A $e) {"
       "  echo $e->a, '\n';"
       "}} test();");
  MVCR("<?php "
       "function nop($en,$es){};set_error_handler('nop');"
       "class X { function bar() { var_dump($this); } }"
       "if (1) {"
       "  class U {"
       "  }"
       "} else {"
       "  class U extends X {"
       "  }"
       "}"
       "class V extends U {}"
       "function test() {"
       "  $x = new X;"
       "  $x->bar();"
       "  $x = new V;"
       "  $x->bar();"
       "}"
       "test();");

  MVCR("<?php "
       "if (true) { class base extends Exception {} } else { class base {} }"
       "class child1 extends base {}"
       "$obj = new child1;"
       "echo ($obj instanceof Exception) ? \"Passed\n\" : \"Failed\n\";");

  MVCR("<?php "
       "class PEAR {"
       "  static function f() { PEAR::g(); }"
       "  function g() { echo 'This is g()'; }"
       "}"
       "if ($x) {"
       "  class PEAR {}"
       "}"
       "class D1 extends PEAR {"
       "  public $foo;"
       "  private $bar;"
       "  function bar() { return $this->foo + $this->bar; }"
       "}"
       "class D2 extends D1 {"
       "  public $foo;"
       "  private $bar;"
       "  function bar() { return $this->foo + $this->bar; }"
       "}"
       "PEAR::f();");
  MVCR("<?php\n"
       "function f() { return false; }\n"
       "if (f()) {\n"
       "  interface A { }\n"
       "  class B { }\n"
       "  interface C { }\n"
       "} else {\n"
       "  class A { }\n"
       "  interface B { }\n"
       "  interface C { }\n"
       "}\n"
       "function test($c) {\n"
       "  var_dump(class_exists('A'));\n"
       "  var_dump(interface_exists('A'));\n"
       "  var_dump(class_exists('B'));\n"
       "  var_dump(interface_exists('B'));\n"
       "  var_dump(class_exists($c));\n"
       "  var_dump(interface_exists('C'));\n"
       "}\n"
       "test('C');\n");
  MVCR("<?php "
      "class A {"
      "  static function bar(&$a) {"
      "    $a = 'ok';"
      "  }"
      "}"
      "$a = 'failed';"
      "A::bar($a);"
      "var_dump($a);"
      "if (false) {"
      "  class A{}"
      "  class A2{}"
      "}"
      "class C {"
      "  static function bar() {"
      "  }"
      "}"
      "class A2 extends C {"
      "  static function bar(&$a) {"
      "    $a = 'ok';"
      "  }"
      "}"
      "$a = 'failed';"
      "A2::bar($a);"
      "var_dump($a);");

  MVCR("<?php "
       "if (!isset($g2)) {"
       "  class test {}"
       "} else {"
       "  class test {"
       "    static $foo = 27;"
       "  }"
       "  var_dump(test::$foo);"
       "}"
       "$x = new test();"
       "$x->bar = 1;"
       "$x->foo = 2;"
       "var_dump($x);");

  MVCR("<?php "
       "class A { function fun() { return 'A'; } }"
       "if (true) { class B {}} else { class B {} }"
       "class C extends B {"
       "  public function foo() { $this->out(A::fun()); }"
       "  public function out($arg) { var_dump($arg); }"
       "}"
       "$c = new C();"
       "$c->foo();");

  MVCR("<?php "
      "class b {"
      "  function z() {"
      "    $this->x();"
      "  }"
      "  function y() {"
      "    echo 'y';"
      "  }"
      "}"
      "class c extends b {"
      "  function x() {"
      "    $this->y();"
      "  }"
      "}"
      "if (false) {"
      "  class b{}"
      "  class c{}"
      "}"
      "$x = new c();"
      "$x->z();");
  MVCR("<?php "
      "class A {}"
      "class B extends A {"
      "  function meh() {"
      "    return $this;"
      "  }"
      "}"
      "class C extends B {"
      "  function work() {"
      "    echo \"WORK\n\";"
      "  }"
      "}"
      "if (false) {"
      "  class A {}"
      "  class B {}"
      "  class C {}"
      "}"
      "function test() {"
      "  $x = new C;"
      "  $x->meh()->work();"
      "}"
      "test();");

  MVCR("<?php "
       "if (isset($g)) {"
       "  class X {}"
       "} else {"
       "  class X {"
       "    static function foo() {}"
       "    function bar() {"
       "      X::foo(1,2,3);"
       "    }"
       "  }"
       "}");

  MVCR("<?php "
       "if (isset($g)) {"
       "  class X {}"
       "} else {"
       "  class X {"
       "    public $a = 1;"
       "    function __destruct() { var_dump(__METHOD__); }"
       "  }"
       "}"
       "class X1 extends X {"
       "  public $t = 1;"
       "}"
       "function test() {"
       "  $x = new X1;"
       "  $x->t = 5;"
       "  $x->a = 3;"
       "  $y = clone $x;"
       "  var_dump($y->a,$y->t);"
       "}"
       "test();");

  return true;
}

bool TestCodeRun::TestClone() {
  MVCR("<?php "
      "class A {"
      "  public $foo = 0;"
      "  public $fooref = 1;"
      "  private $foopriv = 2;"
      "  function __clone() {"
      "    echo \"clone\n\";"
      "  }"
      "}"
      "$a1 = new A;"
      "$p = 8;"
      "$q = 9;"
      "$a1->foo = 'foo';"
      "$a1->fooref = &$p;"
      "$a1->dyn = 'dyn';"
      "$a1->dynref = &$q;"
      "var_dump($a1);"
      "$a2 = clone $a1;"
      "var_dump($a1);"
      "var_dump($a2);"
      "$a2->foo = 'a2foo';"
      "$a2->fooref = 'a2fooref';"
      "$a2->dyn = 'a2dyn';"
      "$a2->dynref = 'a2dynref';"
      "$a2->dynref2 = 'dynref2';"
      "var_dump($a1);"
      "var_dump($a2);"
      "var_dump($p);"
      "var_dump($q);");
  MVCR("<?php "
      "class c {"
      "  protected $cm = 'get';"
      "  function x() {"
      "    var_dump($this->cm);"
      "  }"
      "}"
      "class c2 extends c {}"
      "$y = new c;"
      "$y->x();"
      "$z = clone $y;"
      "$z->x();"
      "$y = new c2;"
      "$y->x();"
      "$z = clone $y;"
      "$z->x();");

  return true;
}

bool TestCodeRun::TestEvalOrder() {
  MVCRO("<?php\n"
        "set_error_handler('h');\n"
        "foo(var_dump('123'));\n"
        "var_dump('end');\n"
        "function h() { var_dump('errored');}",
        "string(7) \"errored\"\n");

  MVCRO("<?php\n"
        "set_error_handler('h');\n"
        "class A {} $obj = new A; $obj->foo(var_dump('123'));\n"
        "var_dump('end');\n"
        "function h() { var_dump('errored');}",
        "string(7) \"errored\"\n");

  MVCR("<?php "
       "$a = array(123); "
       "foreach ($a as $x => $x) { "
       "  var_dump($x); "
       "}");

  MVCR("<?php\n"
       "\n"
       "class MyIterator implements Iterator{\n"
       "  private $var = array();\n"
       "  public function __construct($array) {\n"
       "    echo \"constructing\n\";\n"
       "    if (is_array($array)) {\n"
       "      $this->var = $array;\n"
       "    }\n"
       "  }\n"
       "  public function rewind() {\n"
       "    echo \"rewinding\n\";\n"
       "    reset($this->var);\n"
       "  }\n"
       "  public function current() {\n"
       "    $var = current($this->var);\n"
       "    echo \"current: $var\n\";\n"
       "    return $var;\n"
       "  }\n"
       "  public function key() {\n"
       "    $var = key($this->var);\n"
       "    echo \"key: $var\n\";\n"
       "    return $var;\n"
       "  }\n"
       "  public function next() {\n"
       "    $var = next($this->var);\n"
       "    echo \"next: $var\n\";\n"
       "    return $var;\n"
       "  }\n"
       "  public function valid() {\n"
       "    $var = $this->current() !== false;\n"
       "    echo \"valid: \",$var?'true':'false',\"\n\";\n"
       "    return $var;\n"
       "  }\n"
       "}\n"
       "\n"
       "function f() { var_dump('f'); return 0; }\n"
       "function g() { var_dump('g'); return 0; }\n"
       "\n"
       "$a = array(1, 2);\n"
       "$values = array('a' => 1, 'b' => 2, 'c' => 3);\n"
       "$it = new MyIterator($values);\n"
       "foreach ($it as $a[f()] => $a[g()]) {\n"
       "  print \"$a[0]\n\";\n"
       "}\n");

  MVCR("<?php "
       "class X {} "
       "function foo() { var_dump('foo');} "
       "$x = new X; "
       "unset($x->a[foo()]->y); ");

  MVCR("<?php "
       "function foo($i) { "
       "  var_dump($i); "
       "  return 'a'; "
       "} "
       "${foo(1)}[foo(2)][foo(3)] = foo(4); "
      );

  MVCR("<?php "
      "class A {"
      "  public $foo;"
      "  public $bar;"
      "  function q($a) {"
      "    echo $a;"
      "    $this->foo = 9;"
      "    $this->bar = '3';"
      "    return $this;"
      "  }"
      "}"
      "$a = new A();"
      "var_dump($a->q('1')->foo + $a->q('2')->bar);"
      "var_dump($a->q('1')->foo - $a->q('2')->bar);"
      "var_dump($a->q('1')->foo / $a->q('2')->bar);"
      "var_dump($a->q('1')->foo * $a->q('2')->bar);"
      "var_dump($a->q('1')->foo % $a->q('2')->bar);"
      "var_dump($a->q('1')->foo << $a->q('2')->bar);"
      "var_dump($a->q('1')->foo >> $a->q('2')->bar);"
      "var_dump($a->q('1')->foo && $a->q('2')->bar);"
      "var_dump($a->q('1')->foo || $a->q('2')->bar);"
      "var_dump($a->q('1')->foo and $a->q('2')->bar);"
      "var_dump($a->q('1')->foo or $a->q('2')->bar);"
      "var_dump($a->q('1')->foo xor $a->q('2')->bar);"
      "var_dump($a->q('1')->foo . $a->q('2')->bar);"
      "var_dump($a->q('1')->foo & $a->q('2')->bar);"
      "var_dump($a->q('1')->foo | $a->q('2')->bar);"
      "var_dump($a->q('1')->foo ^ $a->q('2')->bar);"
      "var_dump($a->q('1')->foo == $a->q('2')->bar);"
      "var_dump($a->q('1')->foo === $a->q('2')->bar);"
      "var_dump($a->q('1')->foo != $a->q('2')->bar);"
      "var_dump($a->q('1')->foo !== $a->q('2')->bar);"
      "var_dump($a->q('1')->foo > $a->q('2')->bar);"
      "var_dump($a->q('1')->foo >= $a->q('2')->bar);"
      "var_dump($a->q('1')->foo < $a->q('2')->bar);"
      "var_dump($a->q('1')->foo <= $a->q('2')->bar);"
      );
  MVCR("<?php "
      "function x($a, $b, $c, $d) {}"
      "function p($x) { echo $x . \"\n\"; return $x; }"
      "class c {"
      "  function __construct($a, $b, $c, $d) {}"
      "  function f($a, $b, $c, $d) {}"
      "  static function g($a, $b, $c, $d) {}"
      "}"
      "function rt(&$a, $v) {"
      "  $a = $v;"
      "}"
      "function id($x) { return $x; }"
      "function dump($a, $b) {"
      "  var_dump($a, $b);"
      "}"
      "echo \"sfc\n\";"
      "x(p(1), p(2), p(3), 4);"
      "$y = 'x';"
      "echo \"dfc\n\";"
      "$y(p(1), p(2), p(3), 4);"
      "echo \"smc\n\";"
      "c::g(p(1), p(2), p(3), 4);"
      "$y = 'g';"
      "echo \"dsmc\n\";"
      "c::$y(p(1), p(2), p(3), 4);"
      "echo \"occ\n\";"
      "$q = new c(p(1), p(2), p(3), 4);"
      "echo \"omc\n\";"
      "$q->f(p(1), p(2), p(3), 4);"
      "echo \"rsfc\n\";"
      "rt($a, id(10));"
      "var_dump($a);"
      "dump($v++, $v++);"
      "$v = 10;"
      "dump($v, $v = 0);"
      "echo \"nest\n\";"
      "x(p(1), x(p(2), p(3), p(4), p(5)), p(6), x(p(7), p(8), p(9), p(10)));"
      "echo \"arr\n\";"
      "$z = array(p(1), p(2), x(p(3), p(4), p(5), p(6)), p(7));"
      "$q = 1;"
      "$z = array(1, 2, $q);"
      );
  MVCR("<?php "
      "error_reporting(E_ALL & ~E_NOTICE);"
      "function id($x,$y) { return $x; }"
      "function id1($x) { return $x; }"
      "function pid($x) { var_dump($x); return $x; }"
      "class cls {"
      "  function __construct() { print 'ctor\n'; }"
      "  function f($x) { return $this; }"
      "  function ttest() {"
      "    return $this->f(pid('arg1'),pid('arg2'));"
      "  }"
      "}"
      "$d = id1(new cls())"
      "  ->f('arg1')"
      "  ->f('arg2')"
      "  ->f('arg3');"
      "$d = id1(new cls())"
      "  ->f('arg1', 'argex1')"
      "  ->f('arg2', 'argex2')"
      "  ->f('arg3', 'argex3');"
      "$d = id(new cls(), pid('idarg'))"
      "  ->f(pid('arg1'), pid('argex1'))"
      "  ->f(pid('arg2'), pid('argex2'))"
      "  ->f(pid('arg3'), pid('argex3'));"
      "$d->ttest();");
  MVCR("<?php "
      "class a {"
      "  function r(&$x) {"
      "    $x = 20;"
      "  }"
      "}"
      "function id($x) { return $x; }"
      "$a = new a();"
      "id($a)->r($x);"
      "var_dump($x);");

  MVCR("<?php "
      "class c {"
      "  function x($y) {"
      "    echo $y . \"\n\";"
      "    return $this;"
      "  }"
      "}"
      "function p($x) {"
      "  echo $x . \"\n\";"
      "  return $x;"
      "}"
      "$x = new c;"
      "$x->x(3, p(1), p(2))->x(6, p(4), p(5));");
  MVCR("<?php "
      "class Q {"
      "  public $val;"
      "  function __construct($v) {"
      "    $this->val = $v;"
      "  }"
      "  public function blah() {"
      "    return $this;"
      "  }"
      "}"
      "class A {"
      "  public $v;"
      "  function set($v) {"
      "    $this->v = $v;"
      "    return $this;"
      "  }"
      "}"
      "function id($x) { return $x; }"
      "$x = new Q(0);"
      "$a = id(new A)->set($x);"
      "$x = id(new Q(1))->blah();"
      "var_dump($a);");
  MVCR("<?php "
      "$a = array(array($id = 1, $id), array($id = 2, $id));"
      "var_dump($a);"
      "$a = array(+($id = 1), $id, -($id = 2), $id, "
      "           !($id = 3), $id, ~($id = 4), $id, "
      "           isset($a[$id = 5]), $id);"
      "var_dump($a);");

  MVCR("<?php "
       "function test($a)"
       "{"
       "  echo \"$a\\n\";"
       "}"
       "test(1, test(2), test(3, test(4), test(5)));");

  MVCR("<?php "
      "$v = 1;"
      "function foo($a, $b, $c) {"
      "  var_dump($a, $b, $c);"
      "}"
      "function bar($a) {"
      "  foo($a, $a++, $a);"
      "  $arr = array($a, $a++, $a);"
      "  var_dump($arr);"
      "}"
      "bar($v);");
  MVCR("<?php "
      "$GLOBALS['t'] = 0;"
      "$GLOBALS['f'] = 0;"
      "$GLOBALS['i'] = 0;"
      "$GLOBALS['d'] = 0;"
      "$GLOBALS['v'] = 'a';"
      "function t() {"
      "  global $t;"
      "  $t++;"
      "  return true;"
      "}"
      "function f() {"
      "  global $f;"
      "  $f++;"
      "  return false;"
      "}"
      "function i() {"
      "  global $i;"
      "  $i++;"
      "  return 1;"
      "}"
      "function d() {"
      "  global $d;"
      "  $d++;"
      "  return 3.14;"
      "}"
      "function v() {"
      "  global $v;"
      "  $v++;"
      "  return $v;"
      "}"
      "function foo() {"
      "  var_dump(t() + t());"
      "  var_dump(t() + f());"
      "  var_dump(t() + i());"
      "  var_dump(t() + d());"
      "  var_dump(t() + v());"
      "  var_dump(f() + t());"
      "  var_dump(f() + f());"
      "  var_dump(f() + i());"
      "  var_dump(f() + d());"
      "  var_dump(f() + v());"
      "  var_dump(i() + t());"
      "  var_dump(i() + f());"
      "  var_dump(i() + i());"
      "  var_dump(i() + d());"
      "  var_dump(i() + v());"
      "  var_dump(d() + t());"
      "  var_dump(d() + f());"
      "  var_dump(d() + i());"
      "  var_dump(d() + d());"
      "  var_dump(d() + v());"
      "  var_dump(v() + t());"
      "  var_dump(v() + f());"
      "  var_dump(v() + i());"
      "  var_dump(v() + d());"
      "  var_dump(v() + v());"
      "  var_dump($GLOBALS['t'], $GLOBALS['f'],"
      "           $GLOBALS['i'], $GLOBALS['d'],"
      "           $GLOBALS['v']);"
      "  var_dump(t() - t());"
      "  var_dump(t() - f());"
      "  var_dump(t() - i());"
      "  var_dump(t() - d());"
      "  var_dump(t() - v());"
      "  var_dump(f() - t());"
      "  var_dump(f() - f());"
      "  var_dump(f() - i());"
      "  var_dump(f() - d());"
      "  var_dump(f() - v());"
      "  var_dump(i() - t());"
      "  var_dump(i() - f());"
      "  var_dump(i() - i());"
      "  var_dump(i() - d());"
      "  var_dump(i() - v());"
      "  var_dump(d() - t());"
      "  var_dump(d() - f());"
      "  var_dump(d() - i());"
      "  var_dump(d() - d());"
      "  var_dump(d() - v());"
      "  var_dump(v() - t());"
      "  var_dump(v() - f());"
      "  var_dump(v() - i());"
      "  var_dump(v() - d());"
      "  var_dump(v() - v());"
      "  var_dump($GLOBALS['t'], $GLOBALS['f'],"
      "           $GLOBALS['i'], $GLOBALS['d'],"
      "           $GLOBALS['v']);"
      "  var_dump(t() * t());"
      "  var_dump(t() * f());"
      "  var_dump(t() * i());"
      "  var_dump(t() * d());"
      "  var_dump(t() * v());"
      "  var_dump(f() * t());"
      "  var_dump(f() * f());"
      "  var_dump(f() * i());"
      "  var_dump(f() * d());"
      "  var_dump(f() * v());"
      "  var_dump(i() * t());"
      "  var_dump(i() * f());"
      "  var_dump(i() * i());"
      "  var_dump(i() * d());"
      "  var_dump(i() * v());"
      "  var_dump(d() * t());"
      "  var_dump(d() * f());"
      "  var_dump(d() * i());"
      "  var_dump(d() * d());"
      "  var_dump(d() * v());"
      "  var_dump(v() * t());"
      "  var_dump(v() * f());"
      "  var_dump(v() * i());"
      "  var_dump(v() * d());"
      "  var_dump(v() * v());"
      "  var_dump($GLOBALS['t'], $GLOBALS['f'],"
      "           $GLOBALS['i'], $GLOBALS['d'],"
      "           $GLOBALS['v']);"
      "}"
      "foo();");

  MVCR("<?php "
       "function f($a) { echo \"test$a\\n\"; return 1; }"
       "function bug2($a, $b) {"
       "  return isset($b[f($a++)], $b[f($a++)], $b[f($a++)]);"
       "}"
       "bug2(0, array());");

  MVCR("<?php var_dump($v++, $v++);");
  MVCR("<?php var_dump($v, $v = 0);");
  MVCR("<?php\n"
       "function f(&$a, &$b) { $a = 1; $b = 2; return 3; }\n"
       "class A { }\n"
       "function test() {\n"
       "  $a = array(); f($a[0], $a[1]); var_dump($a);\n"
       "  $a = array(); $a[0] = f($a[1], $a[2]); var_dump($a);\n"
       "  $a = new A(); f($a->f, $a->g); var_dump($a);\n"
       "}\n"
       "test();\n");
  MVCR("<?php "
       "class C {"
       "  function __construct() {"
       "    echo \"class C\\n\";"
       "  }"
       "  public function __get( $what ) {"
       "    echo \"get C\\n\";"
       "    return $this->_p[ $what ];"
       "  }"
       "  public function __set( $what, $value ) {"
       "    echo \"set C\\n\";"
       "    $this->_p[ $what ] = $value;"
       "  }"
       "  private $_p = array();"
       "}"
       "function f() {"
       "  echo \"f()\\n\";"
       "  return 1;"
       "}"
       "function foo() {"
       "  $obj = new C;"
       "  $obj->a = f();"
       "  $obj->b = new C;"
       "  $obj->b->a = f();"
       "}"
       "foo();");
  MVCR("<?php "
       "class C implements ArrayAccess {"
       "  private $data = array();"
       "  public function __construct() {"
       "    echo \"C\\n\";"
       "  }"
       "  public function offsetGet($name) {"
       "    echo \"offsetGet: $name\\n\";"
       "    return $this->data[$name];"
       "  }"
       "  public function offsetSet($name, $value) {"
       "    $a = serialize($value);"
       "    echo \"offsetSet: $name=$a\\n\";"
       "    $this->data[$name]=$value;"
       "  }"
       "  public function offsetExists($name) {"
       "    echo \"offsetExists: $name\\n\"; return true;"
       "  }"
       "  public function offsetUnset($name) {"
       "    echo \"offsetUnset: $name\\n\";"
       "  }"
       "}"
       "function f() {"
       "  echo \"f()\\n\";"
       "  return 1;"
       "}"
       "function f2() {"
       "  echo \"f2()\\n\";"
       "  return 'foo';"
       "}"
       "function foo($a) {"
       "  $a['foo'] = new C;"
       "  $a['foo']['bar'] = new C;"
       "  $a['foo']['bar']['goo'] = f();"
       "}"
       "foo(new C);");

  MVCR("<?php "
       "class X {"
       "  function __destruct() { var_dump('done'); }"
       "}"
       "function f() {"
       "  $x = new X;"
       "}"
       "function g() {"
       "  var_dump('start');"
       "  f();"
       "  var_dump('end');"
       "}"
       "g();");
  MVCR("<?php "
       "function foo($v) {"
       "  $a = array('key' => &$v);"
       "  return $a;"
       "}"
       "function goo($v) {"
       "  return $v . 1;"
       "}"
       "var_dump(foo('1.0'));"
       "var_dump(foo(foo('1.0')));"
       "var_dump(foo(goo('1.0')));");

  MVCR("<?php "
       "function f(&$a, $v = 5) {"
       "  $a = $v;"
       "  return 0;"
       "}"
       "$a = 2;"
       "var_dump($a . f($a));"
       "$a = 2;"
       "var_dump(($a.'') . f($a));"
       "$a = 2;"
       "var_dump(($a.$a) . f($a));"
       "f($a,2);"
       "var_dump($a . f($a));"
       "f($a,2);"
       "var_dump(($a.'') . f($a));"
       "f($a,2);"
       "var_dump(($a.$a) . f($a));"
       "class c {"
       "  public static $a;"
       "}"
       "c::$a = 2;"
       "var_dump(c::$a . f(c::$a));"
       "function g(&$a) {"
       "  $a[0] = 5;"
       "  return 0;"
       "}"
       "$a = array(2);"
       "var_dump($a[0] . g($a));"
       "$a = array(2);"
       "var_dump(($a[0] . '') . g($a));"
       "function h(&$a) {"
       "  $a = 5;"
       "  return 0;"
       "}"
       "$a = array(2);"
       "var_dump($a[0] . h($a[0]));"
       "$a = array(2);"
       "var_dump(($a[0] . '') . h($a[0]));"
       "function k($a) {"
       "  $a->prop = 5;"
       "  return 0;"
       "}"
       "$a = new stdclass;"
       "$a->prop = 2;"
       "var_dump($a->prop . k($a));"
       "$a = new stdclass;"
       "$a->prop = 2;"
       "var_dump(($a->prop . '') . k($a));"
       "$i = 0;"
       "var_dump($i . ++$i);"
       "$i = 0;"
       "var_dump(($i . '') . ++$i);"
       "function foo() {"
       "  return 'foo';"
       "}"
       "f($a, 'test');"
       "var_dump(($a . 'x') . foo($a = ''));"
       "$a = array(2);"
       "var_dump($a[$a = 0]);"
       "$a = new stdclass;"
       "$a->foo = 42;"
       "var_dump($a->{$a = 'foo'});"
       "var_dump($a);"
       "$b = new stdclass;"
       "$a = null;"
       "$a->{f($a,$b)} = 5;"
       "var_dump($a, $b);"
       "function dump($a, $b) {"
       "  var_dump($a, $b);"
       "}"
       "f($a, 'foo');"
       "dump($a, $a = 'bar');"
       "$a = 'foo';"
       "dump($a, $a = 'bar');"
       "f($a, 'foo');"
       "dump($a.'', $a = 'bar');"
       "f($a, 'foo');"
       "dump($a.$a, $a = 'bar');");

  MVCR("<?php "
       "$a = Array(1,2,3); "
       "$b = Array(4,5,6); "
       "$i = 1; "
       "$a[$i++] = $b[$i++]; "
       "var_dump($a);");

  MVCR("<?php "
       "function f($x) {"
       "  global $a;"
       "  var_dump($x, $a);"
       "  return $x;"
       "}"
       "class X implements ArrayAccess {"
       "  function OffsetGet($n) {"
       "    echo 'get:'; var_dump($n);"
       "    return (string)(int)$n == (string)$n ? $this : $n;"
       "  }"
       "  function OffsetSet($n, $v) {"
       "    $this->{$n} = $v;"
       "    echo 'set:'; var_dump($n, $v);"
       "  }"
       "  function OffsetExists($n) { return $n == 'foo'; }"
       "  function OffsetUnset($n) {}"
       "  function __get($n) { return $this->OffsetGet($n); }"
       "  function __set($n,$v) { return $this->OffsetSet($n, $v); }"
       "}"
       "$a = new X;"
       "function ref(&$a, &$b, &$c) {"
       "}"
       "function test() {"
       "  global $a;"
       "  $a[f(0)]->{f(1)}[f(2)] = $a[f(3)][f(4)][f(5)]->foo;"
       "  var_dump($a[f(6)]['fuz'] . f(7));"
       "  ref($a[f(10)][f(11)][f(12)],$a[f(20)][f(21)][f(22)],"
       "      $a[f(30)][f(31)][f(32)]);"
       "  $a->{f(0)}[f(1)]->{f(2)} = $a->{f(3)}->{f(4)}->bar;"
       "}"
       "test();");

  MVCR("<?php "
       "class X { function foo($a) { echo 'In foo:'; var_dump($a); } }"
       "function y($y) { echo 'In y:'; var_dump($y); }"
       "function test($x, $y) {"
       "  $x->foo($x = null);"
       "  $y($y = null);"
       "}"
       "test(new X, 'y');");

  return true;
}

bool TestCodeRun::TestGetObjectVars() {
  MVCR("<?php "
      "class Base"
      "{"
      "  public    $aaa = 1;"
      "  protected $bbb = 2;"
      "  private   $ccc = 3;"
      "}"
      "class Child extends Base"
      "{"
      "  private   $ddd = 4;"
      "}"
      "var_dump(get_object_vars(new Base()));"
      "var_dump(get_object_vars(new Child()));"
      );
  MVCR("<?php "
      "class Base"
      "{"
      "  public    $aaa = 1;"
      "  protected $bbb = 2;"
      "  private   $ccc = 3;"
      "}"
      "class Child extends Base"
      "{"
      "  public    $ddd = 4;"
      "}"
      "class Unrelated"
      "{"
      "  function foo($obj) {"
      "    var_dump(get_object_vars($obj));"
      "  }"
      "}"
      "$base_obj = new Base();"
      "$child_obj = new Child();"
      "$unrelated_obj = new Unrelated();"
      "$unrelated_obj->foo($child_obj);"
      "$unrelated_obj->foo($base_obj);"
      );
  MVCR("<?php "
      "class Base"
      "{"
      "  public    $aaa = 1;"
      "  protected $bbb = 2;"
      "  private   $ccc = 3;"
      "  function foo($obj) {"
      "    var_dump(get_class($obj));"
      "    var_dump(get_object_vars($obj));"
      "  }"
      "}"
      "class Child extends Base"
      "{"
      "  public    $ddd = 4;"
      "  protected $eee = 5;"
      "  private   $fff = 6;"
      "}"
      ""
      "$base_obj = new Base();"
      "$child_obj = new Child();"
      "$base_obj->foo($child_obj);"
      );
  MVCR("<?php "
      "class Base"
      "{"
      "  public    $aaa = 1;"
      "  protected $bbb = 2;"
      "  private   $ccc = 3;"
      "  function foo($obj) {"
      "    var_dump(get_class($obj));"
      "    var_dump(get_object_vars($obj));"
      "  }"
      "}"
      "class Child extends Base"
      "{"
      "  public    $ddd = 5;"
      "  protected $eee = 6;"
      "  private   $fff = 4;"
      "}"
      "$base_obj = new Base();"
      "$base_obj->foo($base_obj);"
      );
  MVCR("<?php "
      "class Base"
      "{"
      "  public    $aaa = 1;"
      "  protected $bbb = 2;"
      "  private   $ccc = 3;"
      "}"
      ""
      "class Child extends Base"
      "{"
      "  public    $ddd = 4;"
      "  protected $eee = 5;"
      "  private   $fff = 6;"
      "  function foo($obj) {"
      "    var_dump(get_class($obj));"
      "    var_dump(get_object_vars($obj));"
      "  }"
      "}"
      "$child_obj = new Child();"
      "$base_obj = new Base();"
      "$child_obj->foo($base_obj);"
      );

  MVCR("<?php "
       "var_dump(get_object_vars(false));"
       "var_dump(get_object_vars(true));"
       "var_dump(get_object_vars('hello'));"
       "var_dump(get_object_vars(5));");

  return true;
}

bool TestCodeRun::TestSerialization() {
  MVCR("<?php\n"
       "class A implements Serializable {\n"
       "  public $__foo = true;\n"
       "  public function serialize() {\n"
       "    return null;\n"
       "  }\n"
       "  public function unserialize($serialized) {\n"
       "  }\n"
       "} var_dump(unserialize(serialize(new A())));");

  MVCR("<?php\n"
       "class A implements Serializable {\n"
       "  public $__foo = true;\n"
       "  public function serialize() {\n"
       "    return serialize(array('a' => 'apple', 'b' => 'banana'));\n"
       "  }\n"
       "  public function unserialize($serialized) {\n"
       "    $props = unserialize($serialized);\n"
       "    $this->a = $props['a'];\n"
       "    $this->b = $props['b'];\n"
       "  }\n"
       "} $obj = unserialize(serialize(new A())); var_dump($obj->b);");

  MVCR("<?php "
      "class Small {"
      "  private static $nc = 0;"
      "  public $name;"
      "  public $num;"
      "  function __construct() {"
      "    $n = self::$nc++;"
      "    $this->name = 'foo'.$n;"
      "    $this->num = 3*$n;"
      "  }"
      "}"
      "class Big {"
      "  public $groupAll = array();"
      "  public $group1 = array();"
      "  public $group2 = array();"
      "  public $wacky;"
      "  public $nothing;"
      "  public $unrelated = array();"
      "  function add() {"
      "    $s = new Small();"
      "    $this->groupAll[] = $s;"
      "    if ($s->num % 2 == 0) {"
      "      $this->group1[]=array($s->name, $s);"
      "    } else {"
      "      $this->group2[]=array($s->name, $s);"
      "    }"
      "  }"
      "  function finish() {"
      "    $x = 10;"
      "    $this->wacky = array(&$x, &$x);"
      "    $s = new Small();"
      "    $this->unrelated[] = $s;"
      "    $this->unrelated[] = $s;"
      "    $this->unrelated[] = $s;"
      "  }"
      "}"
      "function t() {"
      "  $b = new Big;"
      "  for ($i = 0; $i < 10; ++$i) {"
      "    $b->add();"
      "  }"
      "  $b->finish();"
      "  var_dump($b);"
      "  $s = serialize($b);"
      "  var_dump($s);"
      "  $us = unserialize($s);"
      "  var_dump($us);"
      "}"
      "t();"
      );
  MVCR("<?php "
      "class t {"
      "  public $foo = 10;"
      "  protected $bar = 20;"
      "  private $derp = 30;"
      "}"
      "class t2 extends t {"
      "  private $derp2 = 40;"
      "  protected $bar2 = 50;"
      "}"
      "$x = new t;"
      "print_r($x);"
      "var_dump($x);"
      "echo serialize($x) . '\n';"
      ""
      "$x2 = new t2;"
      "print_r($x2);"
      "var_dump($x2);"
      "echo serialize($x2) . '\n';");

  MVCR("<?php "
      "class b {"
      "  private $foo = 1;"
      "  private $bar = 2;"
      "}"
      "class b2 extends b {"
      "  public $bar = 3;"
      "}"
      "$x = new b2;"
      "$x->foo = 100;"
      "var_dump((array)$x);"
      "var_dump(serialize($x));"
      "var_dump($x);");

  // Zend PHP 5.2 has a bug here, fixed in 5.3.
  MVCRO("<?php\n"
        "$a = array(array());\n"
        "$a[0][0] = &$a[0];\n"
        "var_dump(serialize($a));\n",

        "string(24) \"a:1:{i:0;a:1:{i:0;R:2;}}\"\n");

  MVCR("<?php "
       "var_dump(unserialize('a:1:{s:1:\"1\";s:3:\"foo\";}'));"
       "var_dump(unserialize('a:1:{d:1;s:3:\"foo\";}'));"
       "var_dump(unserialize('a:1:{a:1:{i:0;i:1;}}'));");

  return true;
}

bool TestCodeRun::TestJson() {
  MVCR("<?php "
      "$a = array();"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode($a));");
  MVCR("<?php "
      "$a = array(1.23456789e+34, 1E666, 1E666/1E666);"
      "$e = json_encode($a);"
      "var_dump($a);");
  MVCR("<?php "
      "var_dump(json_decode(\"[\\\"a\\\",1,true,false,null]\", true));");

  MVCR("<?php "
       "class A {"
       "  public $a = 'foo';"
       "  protected $b = 'bar';"
       "  private $c = 'blah';"
       "  public function aaaa() {"
       "    var_dump(json_encode($this));"
       "  }"
       "}"
       "$obj = new A();"
       "$obj->aaaa();");

  // recursive object
  MVCR("<?php\n"
       "class Foo { }\n"
       "$foo = new Foo(); $foo->foo = $foo;\n"
       "var_dump(json_encode($foo));\n");

#if 0
  MVCR("<?php "
      "$a = array(1);"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\");"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(1, \"2\");"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>1, \"2\"=>2);"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>\"1\", \"2\"=>\"2\");"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>1, 2=>\"2\");"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(1);"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\");"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(1, \"2\");"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>1, \"2\"=>2);"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>\"1\", \"2\"=>\"2\");"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>1, 2=>\"2\");"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(1);"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\");"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(1, \"2\");"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>1, \"2\"=>2);"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>\"1\", \"2\"=>\"2\");"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>1, 2=>\"2\");"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(1);"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\");"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(1, \"2\");"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>1, \"2\"=>2);"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>\"1\", \"2\"=>\"2\");"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(\"1\"=>1, 2=>\"2\");"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode($a));"
      ""
      "$a = array(1);"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\");"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(1, \"2\");"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>1, \"2\"=>2);"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>\"1\", \"2\"=>\"2\");"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>1, 2=>\"2\");"
      "$a[] = $a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(1);"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\");"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(1, \"2\");"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>1, \"2\"=>2);"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>\"1\", \"2\"=>\"2\");"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>1, 2=>\"2\");"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(1);"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\");"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(1, \"2\");"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>1, \"2\"=>2);"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>\"1\", \"2\"=>\"2\");"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>1, 2=>\"2\");"
      "$a[] = $a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(1);"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\");"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(1, \"2\");"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>1, \"2\"=>2);"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>\"1\", \"2\"=>\"2\");"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));"
      ""
      "$a = array(\"1\"=>1, 2=>\"2\");"
      "$a[] = &$a;"
      "var_dump(&$a);"
      "var_dump(json_encode(&$a));");
#endif
  return true;
}

bool TestCodeRun::TestThrift() {
  MVCR(
      "<?php "
      "class TType {"
      "  const STOP   = 0;"
      "  const VOID   = 1;"
      "  const BOOL   = 2;"
      "  const BYTE   = 3;"
      "  const I08    = 3;"
      "  const DOUBLE = 4;"
      "  const I16    = 6;"
      "  const I32    = 8;"
      "  const I64    = 10;"
      "  const STRING = 11;"
      "  const UTF7   = 11;"
      "  const STRUCT = 12;"
      "  const MAP    = 13;"
      "  const SET    = 14;"
      "  const LST    = 15;"
      "  const UTF8   = 16;"
      "  const UTF16  = 17;"
      "}"
      "class DummyProtocol {"
      "  public $t;"
      "  function __construct() {"
      "    $this->t = new DummyTransport();"
      "  }"
      "  function getTransport() {"
      "    return $this->t;"
      "  }"
      "}"
      "class DummyTransport {"
      "  public $buff = '';"
      "  public $pos = 0;"
      "  function flush() { }"
      "  function write($buff) {"
      "    $this->buff .= $buff;"
      "  }"
      "  function read($n) {"
      "    $r = substr($this->buff, $this->pos, $n);"
      "    $this->pos += $n;"
      "    return $r;"
      "  }"
      "}"
      "class TestStruct {"
      "  static $_TSPEC;"
      ""
      "  public $anInt = null;"
      "  public $aString = null;"
      "  public $aDouble = null;"
      "  public $anInt64 = null;"
      "  public $aList = null;"
      "  public $aMap = null;"
      "  public $aSet = null;"
      "  public $anByte = null;"
      "  public $anI16 = null;"
      ""
      "  public function __construct($vals=null) {"
      "    if (!isset(self::$_TSPEC)) {"
      "      self::$_TSPEC = array("
      "        1 => array("
      "          'var' => 'anInt',"
      "          'type' => TType::I32,"
      "                   ),"
      "        2 => array("
      "          'var' => 'aString',"
      "          'type' => TType::STRING,"
      "                   ),"
      "        3 => array("
      "          'var' => 'aDouble',"
      "          'type' => TType::DOUBLE,"
      "                   ),"
      "        4 => array("
      "          'var' => 'anInt64',"
      "          'type' => TType::I64,"
      "                   ),"
      "        5 => array("
      "          'var' => 'aList',"
      "          'type' => TType::LST,"
      "          'etype' => TType::DOUBLE,"
      "          'elem' => array("
      "            'type' => TType::DOUBLE,"
      "                          ),"
      "                   ),"
      "        6 => array("
      "          'var' => 'aMap',"
      "          'type' => TType::MAP,"
      "          'ktype' => TType::I32,"
      "          'vtype' => TType::DOUBLE,"
      "          'key' => array("
      "            'type' => TType::I32,"
      "                         ),"
      "          'val' => array("
      "            'type' => TType::DOUBLE,"
      "                         ),"
      "          ),"
      "       7 => array("
      "          'var' => 'aSet',"
      "          'type' => TType::SET,"
      "          'etype' => TType::I32,"
      "          'elem' => array("
      "            'type' => TType::I32,"
      "                          ),"
      "                  ),"
      "       8 => array("
      "          'var' => 'anByte',"
      "          'type' => TType::BYTE,"
      "                  ),"
      "       9 => array("
      "          'var' => 'anI16',"
      "          'type' => TType::I16,"
      "                  ),"
      "                            );"
      "    }"
      "  }"
      "}"
      ""
      "function test() {"
      "  $p = new DummyProtocol();"
      "  $v1 = new TestStruct();"
      "  $v1->anInt = 1234;"
      "  $v1->aString = 'abcdef';"
      "  $v1->aDouble = 1.2345;"
      "  $v1->anInt64 = 8589934592;"
      "  $v1->aList = array(13.3, 23.4, 3576.2);"
      "  $v1->aMap = array(10=>1.2, 43=>5.33);"
      "  $v1->aSet = array(10=>true, 11=>true);"
      "  $v1->anByte = 123;"
      "  $v1->anI16 = 1234;"
      "  var_dump($v1);"
      "  thrift_protocol_write_binary($p, 'foomethod', 1, $v1, 20, true);"
      "  var_dump(md5($p->getTransport()->buff));"
      "  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));"
      "}"
      "test();");

  MVCRO(
      "<?php "
      "class TType {"
      "  const STOP   = 0;"
      "  const VOID   = 1;"
      "  const BOOL   = 2;"
      "  const BYTE   = 3;"
      "  const I08    = 3;"
      "  const DOUBLE = 4;"
      "  const I16    = 6;"
      "  const I32    = 8;"
      "  const I64    = 10;"
      "  const STRING = 11;"
      "  const UTF7   = 11;"
      "  const STRUCT = 12;"
      "  const MAP    = 13;"
      "  const SET    = 14;"
      "  const LST    = 15;"
      "  const UTF8   = 16;"
      "  const UTF16  = 17;"
      "}"
      "class DummyProtocol {"
      "  public $t;"
      "  function __construct() {"
      "    $this->t = new DummyTransport();"
      "  }"
      "  function getTransport() {"
      "    return $this->t;"
      "  }"
      "}"
      "class DummyTransport {"
      "  public $buff = '';"
      "  public $pos = 0;"
      "  function flush() { }"
      "  function write($buff) {"
      "    $this->buff .= $buff;"
      "  }"
      "  function read($n) {"
      "    $r = substr($this->buff, $this->pos, $n);"
      "    $this->pos += $n;"
      "    return $r;"
      "  }"
      "}"
      "class TestStruct {"
      "  static $_TSPEC;"
      ""
      "  public $anInt = null;"
      "  public $aDouble = null;"
      "  public $anInt64 = null;"
      "  public $anByte = null;"
      "  public $anI16 = null;"
      ""
      "  public function __construct($vals=null) {"
      "    if (!isset(self::$_TSPEC)) {"
      "      self::$_TSPEC = array("
      "        1 => array("
      "          'var' => 'anInt',"
      "          'type' => TType::I32,"
      "                   ),"
      "        2 => array("
      "          'var' => 'aDouble',"
      "          'type' => TType::DOUBLE,"
      "                   ),"
      "        3 => array("
      "          'var' => 'anInt64',"
      "          'type' => TType::I64,"
      "                   ),"
      "        4 => array("
      "          'var' => 'anByte',"
      "          'type' => TType::BYTE,"
      "                  ),"
      "        5 => array("
      "          'var' => 'anI16',"
      "          'type' => TType::I16,"
      "                  ),"
      "                            );"
      "    }"
      "  }"
      "}"
      ""
      "function test() {"
      "  $p = new DummyProtocol();"
      "  $v1 = new TestStruct();"
      "  $v1->anInt = -1234;"
      "  $v1->aDouble = -1.2345;"
      "  $v1->anInt64 = -1;"
      "  $v1->anByte = -12;"
      "  $v1->anI16 = -123;"
      "  thrift_protocol_write_binary($p, 'foomethod', 1, $v1, 20, true);"
      "  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));"
      "}"
      "test();",

      "object(TestStruct)#4 (5) {\n"
      "  [\"anInt\"]=>\n"
      "  int(-1234)\n"
      "  [\"aDouble\"]=>\n"
      "  float(-1.2345)\n"
      "  [\"anInt64\"]=>\n"
      "  int(-1)\n"
      "  [\"anByte\"]=>\n"
      "  int(-12)\n"
      "  [\"anI16\"]=>\n"
      "  int(-123)\n"
      "}\n");
  return true;
}

bool TestCodeRun::TestExit() {
  MVCR("<?php "
      "function foo() { return false; }"
      "foo() or die(\"foobar\");");
  MVCR("<?php "
      "function foo() { return false; }"
      "foo() or exit(\"foobar\");");

  MVCR("<?php "
       "function callback($data) {"
       "  return \"callback: $data\";"
       "}"
       "ob_start();"
       "echo \"from first level\\n\";"
       "ob_start();"
       "ob_start(\"callback\");"
       "echo \"foobar!\\n\";"
       "exit;");

  MVCR("<?php "
       "declare(ticks=1);"
       "function foo() {"
       "  echo 'a';"
       "  exit(1);"
       "}"
       "pcntl_signal(SIGUSR1,  'foo');"
       "$pid = posix_getpid();"
       "posix_kill($pid, SIGUSR1);"
       "for ($i = 0; $i < 2; $i++) {"
       "  echo 'a';"
       "}");
  MVCR("<?php "
       "$command = \"exit 2\";"
       "system($command, $return);"
       "print \"$return\\n\";");
  MVCR("<?php "
       "$command = \"exit 2\";"
       "passthru($command, $return);"
       "print \"$return\\n\";");

  return true;
}

bool TestCodeRun::TestCreateFunction() {
  MVCR("<?php var_dump(array_filter(array(1, 1003, 34, 5006), "
      "create_function('$x', 'return $x > 1000;')));");
  MVCR("<?php var_dump(array_filter(array(1, 1003, 34, 5006), "
      "create_function('$x', 'return '.'$x > 1000;')));");
  return true;
}

bool TestCodeRun::TestConstructorDestructor() {
  MVCR("<?php "
      "class parent_c {"
      "  public function __construct() {"
      "    echo \"parent__construct\";"
      "  }"
      "  public function __destruct() {"
      "    echo \"parent__destruct\";"
      "  }"
      "}"
      "class child_c extends parent_c {"
      "  public function __construct() {"
      "    echo \"child__construct\";"
      "  }"
      "  public function __destruct() {"
      "    echo \"child__destruct\";"
      "  }"
      "}"
      "$v = new child_c;");
  MVCR("<?php "
      "class parent_c {"
      "  public function __construct() {"
      "    echo \"parent__construct\";"
      "  }"
      "  public function __destruct() {"
      "    echo \"parent__destruct\";"
      "  }"
      "}"
      "class child_c extends parent_c {"
      "  public function __construct() {"
      "    echo \"child__construct\";"
      "    parent::__construct();"
      "  }"
      "  public function __destruct() {"
      "    echo \"child__destruct\";"
      "    parent::__destruct();"
      "  }"
      "}"
      "$v = new child_c;");

  return true;
}

bool TestCodeRun::TestConcat() {
  MVCR("<?php\n"
       "$str = '';\n"
       "$arr1 = array('a', 'b');\n"
       "$arr2 = $arr1;\n"
       "foreach ($arr1 as $v1) {\n"
       "  $str .= $v1;\n"
       "  switch ($v1) {\n"
       "  default:\n"
       "    foreach ($arr2 as $v2) {\n"
       "      $str .= $v2;\n"
       "    }\n"
       "  }\n"
       "}\n"
       "var_dump($str);\n"
       );

  MVCR("<?php "
      "function foo($where_clause)"
      "{"
      "  $sql ="
      "    'SELECT p.property_name, p.property_value, p.property_id, '."
      "    'p.property_data_type, p.property_type, '."
      "    't.tier_id, t.tier_parent_id, t.tier_version, t.tier_type, '."
      "    't.tier_state, t.tier_name '."
      "    'FROM tier t LEFT OUTER JOIN property p ON '."
      "    'p.parent_id = t.tier_id AND '."
      "    'p.property_type = \"tier\" '."
      "    $where_clause;"
      "  echo $sql . \"\\n\";"
      "}"
      "foo(\"where 1 = 1\");");
  MVCR("<?php "
      "echo \"a\" . \"b\" . \"c\" . \"d\" . \"e\";"
      "echo 'a' . 'b' . 'c' . 'd' . 'e';"
      "echo 'a' . \"b\" . \"c\" . \"d\" . 'e';"
      "echo '\"a' . \"\\\"b\" . \"\\'c\" . \"\\'d\" . '\\\"e';"
      "echo 1 . 2 . 3 . 4 . 5;"
      "echo 1 . '2' . '3' . 4 . 5;"
      "echo 1 . \"2\" . \"3\" . 4 . 5;");
  MVCR("<?php "
      "$v = \"c\";"
      "echo \"a\" . \"b\" . $v . \"d\" . \"e\";"
      "echo \"a\" . \"b\" . $v . \"d\" . \"e\" . $v . \"f\" . \"g\";");
  MVCR("<?php "
      "echo '\\\\' . \"\\n\";"
      "echo '\\'' . \"\\n\";"
      "echo '\\\\' . '\\'' . \"\\n\";"
      "echo '\\\\' . \"'\" . \"\\n\";"
      "echo \"\\\\\" . \"'\" . \"\\n\";"
      "echo \"\\101\" . \"\\n\";"
      "echo \"\\\\\" . \"101\" . \"\\n\";"
      "echo \"\\1\" . \"101\" . \"\\n\";"
      "echo \"\\01\" . \"01\" . \"\\n\";"
      "echo \"\\01\" . \"g\" . \"\\n\";"
      "echo \"\\1\" . \"g\" . \"\\n\";"
      "echo \"\\011\" . \"01\" . \"\\n\";"
      "echo \"\\0111\" . \"01\" . \"\\n\";"
      "echo \"\\x\" . \"1\" . \"\\n\";"
      "echo \"\\x1\" . \"1\" . \"\\n\";"
      "echo \"\\x11\" . \"1\" . \"\\n\";"
      "echo \"\\x111\" . \"1\" . \"\\n\";"
      "echo \"\\x1111\" . \"1\" . \"\\n\";"
      "echo \"\\x11111\" . \"1\" . \"\\n\";"
      "echo \"\\777777\" . \"7\" . \"\\n\";"
      "echo \"\\0777777\" . \"7\" . \"\\n\";"
      "echo \"\\00777777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\\"7777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\\\7777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\a7777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\b7777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\f7777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\n7777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\r7777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\t7777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\v7777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\07777\" . \"7\" . \"\\n\";"
      "echo \"\\0077\\'7777\" . \"7\" . \"\\n\";");
  MVCR("<?php "
      "echo '\\\\';"
      "echo '\\'';"
      "echo '\\\\';"
      "echo '\\\\';"
      "echo \"\\\\\";"
      "echo \"\\101\";"
      "echo \"\\\\\";"
      "echo \"\\1\";"
      "echo \"\\01\";"
      "echo \"\\01\";"
      "echo \"\\1\";"
      "echo \"\\011\";"
      "echo \"\\0111\";"
      "echo \"\\x\";"
      "echo \"\\x1\";"
      "echo \"\\x11\";"
      "echo \"\\x111\";"
      "echo \"\\x1111\";"
      "echo \"\\x11111\";"
      "echo \"\\777777\";"
      "echo \"\\0777777\";"
      "echo \"\\00777777\";"
      "echo \"\\0077\\\"7777\";"
      "echo \"\\0077\\\\7777\";"
      "echo \"\\0077\\a7777\";"
      "echo \"\\0077\\b7777\";"
      "echo \"\\0077\\f7777\";"
      "echo \"\\0077\\n7777\";"
      "echo \"\\0077\\r7777\";"
      "echo \"\\0077\\t7777\";"
      "echo \"\\0077\\v7777\";"
      "echo \"\\0077\\07777\";"
      "echo \"\\0077\\'7777\";");
  MVCR("<?php "
      "echo <<<EOT\n"
      "\\t\n"
      "\\r\n"
      "\\a\n"
      "\\b\n"
      "\\f\n"
      "\\v\n"
      "\\\"\n"
      "\\\\\n"
      "'\\\\' . \\`\\n\\`;\n"
      "'\\'' . \\`\\n\\`;\n"
      "'\\\\' . '\\'' . \\`\\n\\`;\n"
      "'\\\\' . \\`'\\` . \\`\\n\\`;\n"
      "\\`\\\\\\` . \\`'\\` . \\`\\n\\`;\n"
      "\\`\\101\\` . \\`\\n\\`;\n"
      "\\`\\\\\\` . \\`101\\` . \\`\\n\\`;\n"
      "\\`\\1\\` . \\`101\\` . \\`\\n\\`;\n"
      "\\`\\01\\` . \\`01\\` . \\`\\n\\`;\n"
      "\\`\\01\\` . \\`g\\` . \\`\\n\\`;\n"
      "\\`\\1\\` . \\`g\\` . \\`\\n\\`;\n"
      "\\`\\011\\` . \\`01\\` . \\`\\n\\`;\n"
      "\\`\\0111\\` . \\`01\\` . \\`\\n\\`;\n"
      "\\`\\x\\` . \\`1\\` . \\`\\n\\`;\n"
      "\\`\\x1\\` . \\`1\\` . \\`\\n\\`;\n"
      "\\`\\x11\\` . \\`1\\` . \\`\\n\\`;\n"
      "\\`\\x111\\` . \\`1\\` . \\`\\n\\`;\n"
      "\\`\\x1111\\` . \\`1\\` . \\`\\n\\`;\n"
      "\\`\\x11111\\` . \\`1\\` . \\`\\n\\`;\n"
      "\\`\\777777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0777777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\00777777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\\\`7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\\\7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\a7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\b7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\f7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\n7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\r7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\t7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\v7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\07777\\` . \\`7\\` . \\`\\n\\`;\n"
      "\\`\\0077\\'7777\\` . \\`7\\` . \\`\\n\\`;\n"
      "EOT;\n");
  MVCR("<?php "
      "$v = 1;"
      "echo $v . b'a' . b\"b\" . `ls \\055\\144 \\x2ftmp`;"
      "echo b'a' . b\"b\" . `ls \\055\\144 \\x2ftmp` . $v;");
  MVCR("<?php "
      "function foo() {"
      "  $u = \"abc\";"
      "  $v = \"\\0\";"
      "  $w = \"def\\n\";"
      "  $x = $u . $v . $w;"
      "  echo $x;"
      "  echo \"abc\" . \"\\0\" . \"def\\n\";"
      "}"
      "foo();"
      "$u = \"abc\";"
      "$v = \"\\0\";"
      "$w = \"def\\n\";"
      "$x = $u . $v . $w;"
      "echo $x;"
      "echo \"abc\" . \"\\0\" . \"def\\n\";"
      "echo \"ab\\0c\\n\";");
  MVCR("<?php "
      "function foo() { return \"hello\" . \"\\0\" . \"world\n\"; }"
      "function bar() {"
      "  $s = foo();"
      "  echo $s;"
      "}"
      "bar();");
  MVCR("<?php "
      "define('FOO'.'BAR', 1);"
      "echo FOOBAR;");
  MVCR("<?php "
      "$a = \"1\";"
      "$a .= \"2\";"
      "$a .= \"3\";"
      "$a .= \"4\";"
      "var_dump($a);"
      "$a .= \"1\";"
      "$a .= \"2\";"
      "$a .= \"3\";"
      "$a .= \"4\";"
      "var_dump($a);");
  MVCR("<?php "
      "$a = array(1, array(1, array(1)));"
      "$a[1][1][1] = 3;"
      "var_dump($a);"
      "$a[1][1][1] = \"1\";"
      "$a[1][1][1] .= \"2\";"
      "$a[1][1][1] .= \"3\";"
      "$a[1][1][1] .= \"4\";"
      "$a[1][1][1] .= \"5\";"
      "var_dump($a);"
      "$payload['pane_html'] = null;"
      "$payload['pane_html'] = "
      "'<div id=\"beacon_accepted_pane\" class=\"beacon_status_pane\" "
      "style=\"display: none\">';"
      "$payload['pane_html'] .= '<div class=\"beacon_status_message\">';");
  MVCR("<?php "
      "$a1 = a;"
      "$a2 = b;"
      "$a3 = c;"
      "$a4 = d;"
      "$a5 = e;"
      "$a6 = f;"
      "$a7 = g;"
      "$a8 = h;"
      "$a9 = i;"
      "$a10 = j;"
      "$a11 = k;"
      "$a12 = l;"
      "$a13 = m;"
      "echo $a1.$a2.$a3.$a4.$a5.$a6;"
      "echo $a1.$a2.$a3.$a4.$a5.$a6.$a7;"
      "echo $a1.$a2.$a3.$a4.$a5.$a6.$a7.$a8;"
      "echo $a1.$a2.$a3.$a4.$a5.$a6.$a7.$a8.$a9;"
      "echo $a1.$a2.$a3.$a4.$a5.$a6.$a7.$a8.$a9.$a10;"
      "echo $a1.$a2.$a3.$a4.$a5.$a6.$a7.$a8.$a9.$a10.$a11;"
      "echo $a1.$a2.$a3.$a4.$a5.$a6.$a7.$a8.$a9.$a10.$a11.$a12;"
      "echo $a1.$a2.$a3.$a4.$a5.$a6.$a7.$a8.$a9.$a10.$a11.$a12.$a13;");

  MVCR("<?php "
      "function n_() {"
      "  return \"\n\" ."
      "  str_repeat($GLOBALS['n_indent_tab'], $GLOBALS['n_indent_level']);"
      "}"
      "function n_indent() {"
      "  $GLOBALS['n_indent_level']++;"
      "  return n_();"
      "}"
      "function n_unindent() {"
      "  $GLOBALS['n_indent_level']--;"
      "  return n_();"
      "}"
      "function render($arg1, $arg2) {"
      "    return"
      "      '<div id=\"captcha\" class=\"'.$arg1.'\">'."
      "      n_indent()."
      "      $arg2 ."
      "      n_unindent()."
      "      '</div>';"
      "}"
      "$GLOBALS['n_indent_level'] = 0;"
      "var_dump(render(\"foo\", \"bar\"));");
  MVCR("<?php "
      "$s = \" \";"
      "$a = \"hello\";"
      "$a .= $s;"
      "$a .= \"world\";"
      "var_dump($a);"
      "$a = \"a\";"
      "$a .= \"b\";"
      "$a .= $a;"
      "var_dump($a);"
      "$a = 3;"
      "echo 0 + \"1$a\";");

  MVCR("<?php "
       "function test($a)"
       "{"
       "  if ($a > 0) {"
       "    $sql = 'foo';"
       "  } else {"
       "    $sql = 'bar';"
       "  }"
       "  $sql .= ' baz';"
       "  return $sql;"
       "}"
       "echo test(1),test(-1),\"\\n\";");
  MVCR("<?php "
       "function foo() {"
       "  echo \" FOO \";"
       "  return \" foo \";"
       "}"
       "function bar() {"
       "  echo \" hello \" . foo() . \"\n\";"
       "  echo \" hello \" , foo() , \"\n\";"
       "}"
       "bar();");
  MVCR("<?php "
       "function foo() {"
       "  print \" FOO \";"
       "  return \" foo \";"
       "}"
       "class A implements ArrayAccess {"
       "  private $data = array();"
       "  public function offsetUnset($index) {}"
       ""
       "  public function offsetGet($index) {"
       "    print \" GET \";"
       "    return \" get \";"
       "  }"
       "  public function offsetSet($index, $value) {"
       "    $data[$index] = $value;"
       "  }"
       "  public function offsetExists($index) { }"
       "}"
       "class C {"
       "  public function __get( $what ) {"
       "    echo \"get C\\n\";"
       "    return $this->_p[ $what ];"
       "  }"
       "  public function __set( $what, $value ) {"
       "    echo \"set C\\n\";"
       "    $this->_p[ $what ] = $value;"
       "  }"
       "  private $_p = array();"
       "}"
       "function bar() {"
       "  print \" hello \" . foo() . \"\\n\";"
       "  $a = new A;"
       "  $a[0] = 0;"
       "  $a[1] = 1;"
       "  echo \" hello $a[0]\";"
       "  echo \" hello $a[1]\\n\";"
       "  print \" hello $a[0]\";"
       "  print \" hello $a[1]\\n\";"
       "  $b = new C;"
       "  $b->a = 'aaaa';"
       "  $b->b = 'bbbb';"
       "  echo \" hello $b->a\";"
       "  echo \" hello $b->b\\n\";"
       "  print \" hello $b->a\";"
       "  print \" hello $b->b\\n\";"
       "  echo \" hello $b->a $b->b $b->a $b->b\";"
       "}"
       "bar();");

  MVCR("<?php "
       "function test($a, $b) {"
       "  return $a . \"\\0\" . $b . \"\\0\" . $a . $b . $a . $b;"
       "}"
       "var_dump(json_encode(test('x', 'y')));");

  return true;
}

bool TestCodeRun::TestConstant() {
  MVCR("<?php define('A', 'B'); define('A_'.A, 'B'); var_dump(A, A_B);");

  MVCR("<?php "
      "define('AAA', true);"
      "define('BBB', false);"
      "define('CCC', null);"
      "if (AAA){"
      "  echo \"AAA\";"
      "} else {"
      "  echo \"!AAA\";"
      "}"
      "if (BBB) {"
      "  echo \"BBB\";"
      "} else {"
      "  echo \"!BBB\";"
      "}"
      "if (CCC) {"
      "  echo \"CCC\";"
      "} else {"
      "  echo \"!CCC\";"
      "}"
      "$a = AAA ? \"AAA\" : \"!AAA\";"
      "$b = BBB ? \"BBB\" : \"!BBB\";"
      "$c = CCC ? \"CCC\" : \"!CCC\";"
      "echo \"$a$b$c\\n\";");
  MVCR("<?php "
      "echo strlen(\"he\\0llo\");"
      "echo php_uname();"
      "echo md5('1f3870be274f6c49b3e31a0c6728957f');"
      "echo sha1('1f3870be274f6c49b3e31a0c6728957f');"
      "echo crc32('1f3870be274f6c49b3e31a0c6728957f');"
      "echo pi();"
      "echo getrandmax();"
      "echo mt_getrandmax();"
      "echo ord('abc');"
      "echo chr(27);"
      "echo strtoupper(\"Mary Had A \\0 Lamb\");"
      "echo strtolower(\"Mary Had A \\0 Lamb\");"
      "echo strcmp(\"Mary Had A \\0Lamb\", \"Mary Had A Lamb\") < 0;"
      "echo strncmp(\"Mary Had A \\0 Lamb\", \"Mary Had A  Lamb\", 5) == 0;"
      "echo strcasecmp(\"Mary Had A \\0 Lamb\", \"Mary Had A  Lamb\") < 0;"
      "echo strncasecmp(\"Mary Had A \\0 Lamb\", \"Mary Had A  Lamb\", 5) == 0;"
      "echo addslashes(\"Is your name \\0O'reilly?\");"
      "echo htmlspecialchars(\"<a ref='test'>Test</a>\", ENT_QUOTES);"
      "echo htmlentities(\"<a ref='test'>Test</a>\", ENT_QUOTES);"
      "echo dirname(\"<a ref='test'>Test</a>\");"
      "echo basename(\"<a ref='test'>Test</a>\");"
      "echo version_compare(\"5.1.2\", \"5.1.3\");"
      "echo dechex(10);"
      "echo hexdec(\"See\");"
      "echo decbin(12);"
      "echo decbin('000110011');"
      "echo decoct(264);"
      "echo octdec('77');"
      "echo octdec(decoct(45));"
      "echo str_repeat(\"-=\", 10);"
      "echo intval(42);"
      "echo intval(4.2);"
      "echo intval('42');"
      "echo intval('+42');"
      "echo intval('-42');"
      "echo intval(042);"
      "echo intval('042');"
      "echo intval(1e10);"
      "echo intval('1e10');"
      "echo intval(0x1A);"
      "echo intval(42000000);"
      "echo intval(42, 8);"
      "echo intval('42', 8);"
      "echo substr(\"abcdef\", -1);"
      "echo substr(\"abcdef\", -2);"
      "echo substr(\"abcdef\", -3, 1);"
      "echo substr(\"abcdef\", 0, -1);"
      "echo substr(\"abcdef\", 2, -1);"
      "echo substr(\"abcdef\", 4, -4);"
      "echo substr(\"abcdef\", -3, -1);"
      "echo substr(\"abcdef\", 1);"
      "echo substr(\"abcdef\", 1, 3);"
      "echo substr(\"abcdef\", 0, 4);"
      "echo substr(\"abcdef\", 0, 8);"
      "echo substr(\"abcdef\", -1, 1);"
      "echo trim(\"\\t\\t\\0These are a few words :) ... \");"
      "echo trim(\"\\t\\t\\0These are a few words :) ... \", \" \\t.\");"
      "echo trim(\"\\x09Example string\\x0A\", \"\\x00..\\x1F\");"
      "echo ltrim(\"\\t\\t\\0These are a few words :) ... \");"
      "echo ltrim(\"\\t\\t\\0These are a few words :) ... \", \" \\t.\");"
      "echo ltrim(\"\\x09Example string\\x0A\", \"\\x00..\\x1F\");"
      "echo rtrim(\"\\t\\t\\0These are a few words :) ... \");"
      "echo rtrim(\"\\t\\t\\0These are a few words :) ... \", \" \\t.\");"
      "echo rtrim(\"\\x09Example string\\x0A\", \"\\x00..\\x1F\");"
      "echo chop(\"\\t\\t\\0These are a few words :) ... \");"
      "echo chop(\"\\t\\t\\0These are a few words :) ... \", \" \\t.\");"
      "echo chop(\"\\x09Example string\\x0A\", \"\\x00..\\x1F\");"
      "echo acos(0.5);"
      "echo acosh(0.5);"
      "echo asin(0.5);"
      "echo asinh(0.5);"
      "echo atan(0.5);"
      "echo atan2(0.5, 0.5);"
      "echo atanh(0.5);"
      "echo cos(0.5);"
      "echo cosh(0.5);"
      "echo sin(0.5);"
      "echo sinh(0.5);"
      "echo tan(0.5);"
      "echo tanh(0.5);"
      "echo exp(5.7);"
      "echo exp(12);"
      "echo log10(12);"
      "echo log(12);"
      "echo sqrt(2);"
      "echo ceil(7.9);"
      "echo floor(7.9);"
      "echo fmod(5.7, 1.3);"
      "echo ip2long(\"127.0.0.1\");"
      "echo ip2long(\"10.0.0\");"
      "echo ip2long(\"10.0.256\");"
      "echo long2ip(pow(2,32) + 1024);"
      "echo rad2deg(M_PI_4);"
      "echo deg2rad(45);");
  MVCR("<?php "
      "var_dump(067);"
      "var_dump(077);"
      "var_dump(078);"
      "var_dump(0x78);"
      "var_dump(0x78f);"
      "var_dump(0xef);"
      "var_dump(-067);"
      "var_dump(-077);"
      "var_dump(-078);"
      "var_dump(-0x78);"
      "var_dump(-0x78f);"
      "var_dump(-0xef);");
  MVCR("<?php "
      "define('FOO', \"\\n\");"
      "define('BAR', \"\\r\");"
      "var_dump(PHP_EOL);"
      "var_dump(FOO);"
      "var_dump(BAR);");
  MVCR("<?php "
      "var_dump(INF);"
      "var_dump(NAN);");
  MVCR("<?php "
      "define('A_B', 555);"
      "define('A_'. 'B', 'B');");
  MVCR("<?php "
      "var_dump(define('AF_UNIX', 5));"
      "var_dump(AF_UNIX);");
  return true;
}

bool TestCodeRun::TestClassConstant() {
  MVCR("<?php "
      "class parent_c {"
      "  const ZERO   = 0;"
      "  const TWENTY = 20;"
      "  const FORTY  = 40;"
      "}"
      "class child_c extends parent_c {"
      "  const FIFTY = 50;"
      "}"
      ""
      "function foo"
      "($a = parent_c::ZERO, $b = child_c::FIFTY, $c = child_c::FORTY) {"
      "  echo $a;"
      "  echo $b;"
      "  echo $c;"
      "}"
      "foo();"
      "print parent_c::ZERO;");
  MVCR("<?php "
      "class FooConstants {"
      "  const ZERO        = 0;"
      "  const TWENTY_FOUR3 = FooConstants::TWENTY_FOUR2;"
      "  const TWENTY_FOUR2 = FooConstants::TWENTY_FOUR;"
      "  const TWENTY_FOUR = 24;"
      "  const FORTY_EIGHT = 48;"
      "}"
      "class BarConstants {"
      "  const ZERO        = FooConstants::ZERO;"
      "  const TWENTY_FOUR = FooConstants::TWENTY_FOUR;"
      "  const FORTY_EIGHT = FooConstants::FORTY_EIGHT;"
      "}"
      "class GooConstants {"
      "  const ZERO        = BarConstants::ZERO;"
      "  const TWENTY_FOUR = BarConstants::TWENTY_FOUR;"
      "  const FORTY_EIGHT = BarConstants::FORTY_EIGHT;"
      "}"
      "function a_better_pickle() {"
      "  return FooConstants::ZERO;"
      "}"
      "a_better_pickle();"
      "print GooConstants::ZERO;"
      "print FooConstants::TWENTY_FOUR2;"
      "print FooConstants::TWENTY_FOUR3;");
  MVCR("<?php "
      "define('FOO', 3);"
      "define('BAR', true);"
      "define('GOO', FOO + 4);"
      "define('HOO', FOO);"
      "var_dump(FOO);"
      "var_dump(BAR);"
      "var_dump(GOO);"
      "var_dump(HOO);"
      "class A {"
      "  const C1 = 1;"
      "  const C2 = '2';"
      "  const C3 = FOO;"
      "  const C4 = BAR;"
      "  const C5 = GOO;"
      "  const C6 = HOO;"
      "}"
      "var_dump(a::C1);"
      "var_dump(a::C2);"
      "var_dump(a::C3);"
      "var_dump(a::C4);"
      "var_dump(a::C5);"
      "var_dump(a::C6);");
  MVCR("<?php "
      "define('FOO', 3);"
      "function foo($a = FOO) {"
      "  echo $a;"
      "}"
      "foo();");
  MVCR("<?php "
      "class c {"
      "  function foo($x = self::BLAH) {}"
      "}");

  MVCR("<?php "
      "interface X {"
      "  const A=1;"
      "}"
      "class Y {"
      "  const B = 2;"
      "}"
      "class Z extends Y implements X {"
      "  function x() {"
      "    print self::A;"
      "    print self::B;"
      "    print Z::A;"
      "    print Z::B;"
      "    print X::A;"
      "    print Y::B;"
      "  }"
      "}"
      "$z = new Z;"
      "$z->x();");
  MVCR("<?php "
      "class Dummy {}"
      ""
      "class foo {"
      "  public static $v = array(Dummy::c => 'foo');"
      "}"
      ""
      "interface A {"
      "  const CONSTANT = 'CONSTANT';"
      "}"
      ""
      "class B implements A { }"
      ""
      "class C {"
      "  static $A_CONSTANT = A::CONSTANT;"
      "  static $B_CONSTANT = B::CONSTANT;"
      "}"
      ""
      "var_dump(A::CONSTANT);"
      "var_dump(B::CONSTANT);"
      "var_dump(C::$A_CONSTANT);"
      "var_dump(C::$B_CONSTANT);");
  MVCR("<?php "
      "abstract class TB {"
      "  const PARAM_A = 'aaa';"
      "  const PARAM_B = 'bbb';"
      "  const PARAM_C = 'ccc';"
      "  const PARAM_D = 'ddd';"
      "}"
      "abstract class ATB extends TB {"
      "}"
      "class ABCD extends ATB {"
      "  static public function foo() {"
      "    return array("
      "      'a_ids'   => array("
      "        ATB::PARAM_A => true,"
      "        ATB::PARAM_C   => array("
      "          array('tcks', 'none'),"
      "          array('tcks', 'ids'),"
      "          ),"
      "        ATB::PARAM_B     =>"
      "          'aaaa',"
      "      ),"
      "      'user_id'   => array("
      "        ATB::PARAM_A => true,"
      "        ATB::PARAM_C   => array("
      "          array('tcks', 'none'),"
      "          array('tcks', 'id'),"
      "          ),"
      "        ATB::PARAM_B     =>"
      "          'bbbb',"
      "      ),"
      "    );"
      "  }"
      "}"
      "var_dump(ABCD::foo());");
  return true;
}

bool TestCodeRun::TestConstantFunction() {
  MVCR("<?php "
      "class JavaScriptPacker {"
      "  public function foo() {"
      "    $encode10 = $this->_getJSFunction('_encode10');"
      "    var_dump($encode10);"
      "    $encode36 = $this->_getJSFunction('_encode36');"
      "    var_dump($encode36);"
      "  }"
      "  private function _getJSFunction($aName) {"
      "    if (defined('self::JSFUNCTION'.$aName))"
      "      return constant('self::JSFUNCTION'.$aName);"
      "    else"
      "      return '';"
      "  }"
      "      const JSFUNCTION_encode10 ="
      "        'function($charCode) {"
      "    return $charCode;"
      "}';"
      ""
      "}"
      "$obj = new JavaScriptPacker;"
      "$obj->foo(); ");
  MVCR("<?php "
      "var_dump(constant('M_PI'));"
      "$a = 'M_PI';"
      "var_dump(constant($a));"
      "define('FOO', M_PI);"
      "var_dump(constant('FOO'));"
      "define('BAR', php_uname());"
      "var_dump(constant('BAR'));"
      "define(/*|Dynamic|*/'GOO', 1);"
      "var_dump(constant('GOO'));"
      "if (false) {"
      "  define('C', 1);"
      "} else {"
      "  define('C', 2);"
      "}"
      "var_dump(constant('C'));");

  return true;
}

bool TestCodeRun::TestDefined() {
  MVCR("<?php "
      "if (defined('FOO')) echo 'defined'; else echo 'undefined';");
  MVCR("<?php "
      "define('FOO', 1);"
      "if (defined('FOO')) echo 'defined'; else echo 'undefined';");
  MVCR("<?php "
      "echo FOO;"
      "if (defined('FOO')) echo 'defined'; else echo 'undefined';");

  MVCR("<?php "
      "define('THIRTEEN', 13);"
      "define('ONE', 1);"
      "class Foo {"
      "  const ZERO        = 0;"
      "  function f() {"
      "    if (defined('self::ZERO')) {"
      "      var_dump(self::ZERO);"
      "    }"
      "    if (defined('THIRTEEN')) {"
      "      var_dump(THIRTEEN);"
      "    }"
      "    if (defined('ONE')) {"
      "      var_dump(ONE);"
      "    }"
      "    $a = 'self::ZERO';"
      "    if (defined($a)) {"
      "      var_dump(self::ZERO);"
      "    }"
      "  }"
      "}"
      "class Bar extends Foo {"
      "  function f() {"
      "    if (defined('self::ZERO')) {"
      "      var_dump(self::ZERO);"
      "    }"
      "    if (defined('parent::ZERO')) {"
      "      var_dump(parent::ZERO);"
      "    }"
      "    $a = 'parent::ZERO';"
      "    if (defined($a)) {"
      "      var_dump(parent::ZERO);"
      "    }"
      "  }"
      "}"
      "class Goo {"
      "  const ZERO = Bar::ZERO;"
      "}"
      "$a = 'Foo::ZERO';"
      "if (defined($a)) {"
      "  var_dump(Foo::ZERO);"
      "}"
      "$a = 'Bar::ZERO';"
      "if (defined($a)) {"
      "  var_dump(Bar::ZERO);"
      "}"
      "$a = 'Goo::ZERO';"
      "if (defined($a)) {"
      "  var_dump(Goo::ZERO);"
      "}"
      "$obj = new Foo;"
      "$obj->f();"
      "$obj = new Bar;"
      "$obj->f();");

  MVCR("<?php "
       "function handler($errno, $errstr) {"
       "  var_dump($errno);"
       "  return true;"
       "}"
       "set_error_handler('handler');"
       "unserialize();"
       "define();"
       "define('u');"
       "define('a','X');"
       "define('b','Y','d');"
       "define('c',1,2,3,4,foo());"
       "var_dump(a,b,c);"
       "var_dump(defined('a'),defined('b'),defined('c'));"
       "function foo() {"
       "  var_dump('FOO');"
       "}");

  return true;
}

bool TestCodeRun::TestAssignment() {
  MVCR("<?php "
      "function f($a) {"
      "  var_dump($a);"
      "}"
      "class ClassA {"
      "  var $val;"
      "  function foo() { f($val = 1); }"
      "  function bar() { f($this->val = 1); }"
      "  function goo() { f($val = 'val'); f($this->$val = 2); }"
      "  function zoo() {"
      "    var_dump($val); var_dump($this->val);"
      "  }"
      "}"
      "function foo() {"
      "  f($val2 = 1);"
      "}"
      "$obj = new ClassA();"
      "var_dump($obj);"
      "$obj->foo();"
      "var_dump($obj);"
      "$obj->bar();"
      "var_dump($obj);"
      "$obj->goo();"
      "var_dump($obj);"
      "$obj->zoo();");
  MVCR("<?php "
      "function f($a) {"
      "  var_dump($a);"
      "}"
      "class ClassA {"
      "  static $val = 1;"
      "  function foo() { f($val = 'val'); f($this->$val = 2); }"
      "  function foo2() { f($this->val = 3); }"
      "  function bar() {"
      "    var_dump($val); var_dump($this->val);"
      "  }"
      "}"
      "$obj = new ClassA();"
      "var_dump($obj);"
      "$obj->foo();"
      "var_dump($obj);"
      "$obj->bar();"
      "$obj->foo2();"
      "var_dump($obj);"
      "$obj->bar();");

  MVCR("<?php "
       "class X"
       "{"
       "  function __destruct() { var_dump('destruct'); }"
       "}"
       "function foo() {"
       "  $x = new X;"
       "  var_dump('before');"
       "  $x = null;"
       "  var_dump('after');"
       "}"
       "foo();");

  MVCR("<?php "
       "function e() { return 'hello'; }"
       "function foo() {"
       "  $expected = e();"
       "  $list_expected = \"[$expected,$expected]\";"
       "  var_dump($expected, $list_expected);"
       "}"
       "foo();");
  return true;
}

bool TestCodeRun::TestSimpleXML() {
  MVCR("<?php\n"
       "function addChildNode(SimpleXMLElement $parent, "
       "SimpleXMLElement $node) {\n"
       "  $newchild = $parent->addChild($node->getName(), (string)$node);\n"
       "  foreach ($node->attributes() as $name => $value) {\n"
       "    $newchild->addAttribute($name, $value);\n"
       "  }\n"
       "  foreach ($node->children() as $child) {\n"
       "    addChildNode($newchild, $child);\n"
       "  }\n"
       "}\n"
       "\n"
       "$xmlreq = '<a><item><node><sub>1st</sub>"
       "<sub>2nd</sub></node></item></a>';\n"
       "$quote = simplexml_load_string($xmlreq);\n"
       "$req = new SimpleXMLElement('<node/>');\n"
       "foreach ($quote->attributes() as $name => $value) {\n"
       "  $req->addAttribute($name, $value);\n"
       "}\n"
       "foreach ($quote->children() as $child) {\n"
       "  addChildNode($req, $child);\n"
       "}\n"
       "\n"
       "$vertex = new SimpleXMLElement('<root/>');\n"
       "addChildNode($vertex, $req);\n"
       "var_dump($vertex->asXML());\n"
      );

  MVCR("<?php $x = new SimpleXMLElement('<foo><bar>345.234</bar></foo>');"
       "var_dump((double)$x->bar);");
  MVCR("<?php $x = new SimpleXMLElement('<foo><bar></bar></foo>');"
       "var_dump((bool)$x->bar);");
  MVCR("<?php $x = new SimpleXMLElement('<foo><bar>0</bar></foo>');"
       "var_dump((bool)$x->bar);");

  MVCR("<?php "
       "$x = new SimpleXMLElement('<foo/>'); "
       "$x->addAttribute('attr', 'one'); "
       "$x['attr'] = 'two'; "
       "var_dump((string)$x['attr']); "
       "var_dump($x->asXML());");

  MVCR("<?php\n"
       "$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');\n"
       "var_dump((array)$node->bar);");

  MVCR("<?php\n"
       "$node = new SimpleXMLElement('<foo><bar name=\"value\">"
       "whoops</bar></foo>');\n"
       "var_dump((array)$node->bar);");

  MVCR("<?php\n"
       "$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');\n"
       "var_dump((string)$node[0]);");

  MVCR("<?php\n"
       "$node = simplexml_load_string('<foo><bar>whoops</bar></foo>');"
       "var_dump((string)$node);");

  MVCR("<?php\n"
       "$node = new SimpleXMLElement('<foo><bar>whoops</bar></foo>');"
       "var_dump((string)$node);");

  MVCR("<?php\n"
       "$node = new SimpleXMLElement('<foo>whoops</foo>');"
       "var_dump((string)$node);");

  MVCR("<?php\n"
       "$sxe = new SimpleXMLElement('<image-definition />');\n"
       "$sxe->addChild('path', 'some/path/to/my.file');\n"
       "$sxe->addChild('options');\n"
       "$sxe->options->addChild('paddingbottom', 1);\n"
       "var_dump((string)$sxe->path);\n"
       "var_dump((string)$sxe->options->paddingbottom);\n"
      );

  MVCR("<?php\n"
       "$x = new SimpleXMLElement('<foo/>');\n"
       "$x->addChild('bar', 'whoops');\n"
       "var_dump((string)$x);\n");

  MVCR(
    "<?php\n"
    "function convert_simplexml_to_array($sxml) {\n"
    "  $arr = array();\n"
    "  if ($sxml) {\n"
    "    foreach ($sxml as $k => $v) {\n"
    "      if ($sxml['list']) {\n"
    "        if (isset($v['key'])) {\n"
    "          $arr[(string)$v['key']] = convert_simplexml_to_array($v);\n"
    "        } else {\n"
    "          $arr[] = convert_simplexml_to_array($v);\n"
    "        }\n"
    "      } else {\n"
    "        $arr[$k] = convert_simplexml_to_array($v);\n"
    "      }\n"
    "    }\n"
    "  }\n"
    "  if (sizeof($arr) > 0) {\n"
    "    return $arr;\n"
    "  } else {\n"
    "    return (string)$sxml;\n"
    "  }\n"
    "}\n"
    "\n"
    "$xml = <<<EOM\n"
    "<root list=\"true\">\n"
    "  <node key=\"key1\" list=\"true\">\n"
    "    <subnode key=\"subkey1\" list=\"true\">\n"
    "      <name>value1-1</name>\n"
    "      <name>value1-2</name>\n"
    "    </subnode>\n"
    "    <subnode key=\"subkey2\">value1</subnode>\n"
    "  </node>\n"
    "  <node key=\"key2\" list=\"true\">\n"
    "    <subnode>value2</subnode>\n"
    "  </node>\n"
    "</root>\n"
    "EOM;\n"
    "\n"
    "$sxml = simplexml_load_string($xml);\n"
    "var_dump(convert_simplexml_to_array($sxml));\n"
  );

  MVCR("<?php\n"
       "$xml = '<?xml version=\"1.0\" encoding=\"UTF-8\"?><response>test</response>';\n"
       "$sxml = simplexml_load_string($xml);\n"
       "foreach ($sxml as $k => $v) {\n"
       "  var_dump($k, (string)$v);\n"
       "}\n");

  MVCR("<?php\n"
       "$xml = '<?xml version=\"1.0\" encoding=\"UTF-8\"?><response><t>6</t><t>7</t><t>8</t></response>';\n"
       "$sxml = simplexml_load_string($xml);\n"
       "foreach ($sxml as $k => $v) {\n"
       "  var_dump($k, (int)$v);\n"
       "}\n");

  MVCR("<?php\n"
       "$xml = '<?xml version=\"1.0\" encoding=\"UTF-8\"?><response><t>6</t><t>7</t><t>8</t></response>';\n"
       "$sxml = simplexml_load_string($xml);\n"
       "foreach ($sxml as $k => $v) {\n"
       "  var_dump($k, (string)$v);\n"
       "}\n");

  MVCR("<?php\n"
       "$xml = '<?xml version=\"1.0\" encoding=\"UTF-8\"?><root>"
       "<invalidations><invalidation id=\"12345\"/></invalidations></root>';\n"
       "$dom = new SimpleXMLElement($xml);\n"
       "$invalidations = $dom->invalidations;\n"
       "var_dump((string)$invalidations->invalidation[\"id\"]);\n"
       "foreach ($invalidations as $node) {\n"
       "  var_dump((string)$node->invalidation[\"id\"]);\n"
       "}\n");

  MVCR("<?php\n"
       "\n"
       "$file = <<<EOM\n"
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       "<wurfl-config>\n"
       "  <persistence>\n"
       "    <provider>memcache</provider>\n"
       "    <params></params>\n"
       "  </persistence>\n"
       "\n"
       "  <cache>\n"
       "    <provider>memcache</provider>\n"
       "    <params></params>\n"
       "  </cache>\n"
       "</wurfl-config>\n"
       "EOM;\n"
       "var_dump($file);\n"
       "\n"
       "$xml = simplexml_load_string($file);\n"
       "foreach ($xml->children() as $parent_name => $xml_ele) {\n"
       "  var_dump($parent_name);\n"
       "\n"
       "  foreach ($xml_ele->children() as $key => $value) {\n"
       "    var_dump((string)$key, (string)$value);\n"
       "  }\n"
       "}\n"
      );

  MVCR("<?php\n"
       "$xml = '<?xml version=\"1.0\" encoding=\"UTF-8\"?><response><t>6</t></response>';\n"
       "$sxml = simplexml_load_string($xml);\n"
       "function convert_simplexml_to_array($sxml) {\n"
       "  if ($sxml) {\n"
       "    foreach ($sxml as $k => $v) {\n"
       "      var_dump($k, (string)$v);\n"
       "      convert_simplexml_to_array($v);\n"
       "    }\n"
       "  }\n"
       "}\n"
       "convert_simplexml_to_array($sxml);");

  MVCR("<?php $doc = new SimpleXMLElement('<?xml version=\"1.0\"?><root><node><option>1</option></node></root>'); $doc->node->option = false; var_dump($doc->asXML());");

  MVCR("<?php $doc = new SimpleXMLElement('<?xml version=\"1.0\"?><root><node><option>1</option></node></root>'); $doc->node->option = 0; var_dump($doc->asXML());");

  MVCR("<?php $doc = new SimpleXMLElement('<?xml version=\"1.0\"?><root><node><option>1</option></node></root>'); unset($doc->node->option); var_dump($doc->asXML());");

  MVCR("<?php $doc = simplexml_load_String('<?xml version=\"1.0\"?><lists><list path=\"svn+ssh\"><entry kind=\"dir\"></entry><entry kind=\"file\"></entry></list></lists>'); foreach ($doc->list[0]->entry as $r) { var_dump((array)$r->attributes());}");

  MVCR("<?php "
       "$sxe = new SimpleXMLElement('<foo />');"
       "$sxe->addChild('options');"
       "$sxe->options->addChild('paddingtop', 0);"
       "echo 'Success\n';");

  MVCR("<?php $doc = simplexml_load_string('<?xml version=\"1.0\"?><root xmlns:foo=\"http://example.com\"><foo:b1>c1</foo:b1><foo:b2>c2</foo:b2><foo:b2>c3</foo:b2></root>'); $foo_ns_bar = $doc->children('http://example.com');"
      "var_dump($doc->getName());"
      "foreach ($foo_ns_bar as $v) var_dump((string)$v);"
      "var_dump($foo_ns_bar->getName());"
      "var_dump(count($foo_ns_bar->b1));"
      "var_dump((string)$foo_ns_bar->b1);"
      "var_dump((string)$foo_ns_bar->b1[0]);"
      "foreach ($foo_ns_bar->b1 as $v) var_dump((string)$v);"
      "var_dump(count($foo_ns_bar->b2));"
      "var_dump((string)$foo_ns_bar->b2[0]);"
      "var_dump((string)$foo_ns_bar->b2[1]);"
      "foreach ($foo_ns_bar->b2 as $v) var_dump((string)$v);"
      );

  MVCR("<?php function printElement($el, $indent='') {"
      "  if (strlen($indent) > 10) {"
      "    var_dump('Recursed to deep, backing out');"
      "    return;"
      "  }"
      "  print $indent.$el->getName().\"\\n\";"
      "  foreach ($el->attributes() as $k => $v) {"
      "    print $indent.$k.' => '.$v.\"\\n\";"
      "  }"
      "  foreach ($el->children() as $child) {"
      "    printElement($child, $indent.'  ');"
      "  }"
      "}"
      "$a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><xx><yy><node a=\"b\">hi</node></yy><yy><node a=\"b\">hi</node></yy></xx>');"
      "printElement($a);"
      );

  MVCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node a=\"b\"><subnode attr1=\"value1\" attr2=\"value2\">test</subnode><subnode><subsubnode>test</subsubnode></subnode><test>v</test></node>');"
      "var_dump((array)$a->attributes());"
      "var_dump((string)$a->subnode[0]);"
      "var_dump((string)$a->subnode[0]['attr1']);"
      "var_dump((string)$a->subnode[1]['subsubnode']);"
      "var_dump((string)$a->subnode[1]->subsubnode);"
      "var_dump((string)$a->test);"
      "var_dump((array)$a->subnode[0]->attributes());"
      "var_dump((array)$a->subnode[1]->attributes());"
      "var_dump($a->asxml());"
      "var_dump((string)$a->addchild('newnode', 'newvalue'));"
      "$a->addattribute('newattr', 'newattrvalue');"
      "var_dump($a->asxml());"
      "var_dump((array)$a->attributes());"
      "var_dump((string)$a->newnode);"
      "var_dump($a->getname());"
      "var_dump((array)$a->children()->subnode[0]->subsubnode);"
      "$nodes = $a->xpath('//node/subnode');"
      "var_dump((string)$nodes[1]->subsubnode);"
      "$nodes = $nodes[1]->xpath('subsubnode');"
      "var_dump((string)$nodes[0]);"
      );

  MVCR("<?php $a = new SimpleXMLElement('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');"
      "var_dump((array)($a->subnode->subsubnode));"
      "var_dump((string)($a->subnode->subsubnode));"
      );

  MVCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');"
      "var_dump((array)($a->subnode->subsubnode));"
      "var_dump((string)($a->subnode->subsubnode));"
      );
  MVCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');"
      "var_dump((string)($a->subnode->subsubnode['0']));"
      "var_dump((string)($a->subnode->subsubnode[0]));"
      );

  MVCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode attr1=\"value1\">test</subnode></node>');"
      "var_dump((string)($a->subnode['attr1']));"
      );
  MVCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode attr1=\"value1\">test</subsubnode></subnode></node>');"
      "var_dump((string)($a->subnode->subsubnode['attr1']));"
      );

  MVCR("<?php $a = new SimpleXMLElement('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode><sssnode>test</sssnode></subsubnode></subnode></node>');"
      "var_dump((string)($a->subnode->subsubnode->sssnode));"
      );
  MVCR("<?php "
      "$post_xml = '<?xml version=\"1.0\" encoding=\"utf-8\"?><ScanResults version=\"1.0\"><scannedItem itemType=\"5\" itemSize=\"1079856\" "
      "itemName=\"C:\\\\Program Files\\\\VMware\\\\VMware Tools\\\\VMwareUser.exe\" "
      "IsScanned=\"1\" IsInfected=\"0\" ObjectSummary=\"0\" "
      "ScanError=\"0\"/></ScanResults>';"
      "$xml = new SimpleXMLElement($post_xml);"
      "foreach ($xml->scannedItem as $item) {"
      "  echo $item['itemName'] . \"\\n\";"
      "}");

  MVCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><?mso-application progid=\"Excel.Sheet\"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');"
      "var_dump((string)($a->subnode->subsubnode[0]));"
      );

  MVCR("<?php\n"
       "$xml = '<?xml version=\"1.0\" encoding=\"UTF-8\"?><response><t a=\"apple\" b=\"banana\">6</t><t>7</t><t>8</t></response>';\n"
       "$sxml = simplexml_load_string($xml);\n"
       "var_dump(count($sxml->t));\n"
       "var_dump((string)$sxml->t[0]);\n"
       "var_dump((string)$sxml->t[1]);\n"
       "var_dump((string)$sxml->t[2]);\n"
       "var_dump(count($sxml->t->bogus));\n"
       "var_dump(count($sxml->t->attributes()));\n"
       "foreach ($sxml->bogus as $v) {}");

  return true;
}

bool TestCodeRun::TestXML() {
  MVCR(
    "<?php\n"
    "class xml {\n"
    "  var $parser;\n"
    "  function xml() {\n"
    "    $this->parser = xml_parser_create();\n"
    "    xml_set_object($this->parser, $this);\n"
    "    xml_set_element_handler($this->parser, 'tag_open', 'tag_close');\n"
    "    xml_set_character_data_handler($this->parser, 'cdata');\n"
    "  }\n"
    "  function parse($data) { xml_parse($this->parser, $data);}\n"
    "  function tag_open($parser, $tag, $attributes) {\n"
    "    var_dump($tag, $attributes);\n"
    "  }\n"
    "  function cdata($parser, $cdata) { var_dump($cdata);}\n"
    "  function tag_close($parser, $tag){ var_dump($tag);}\n"
    "}\n"
    "\n"
    "$xml_parser = new xml();\n"
    "$xml_parser->parse('<A ID=\"hallo\">PHP</A>');\n"
  );
  return true;
}

bool TestCodeRun::TestDOMDocument() {
  MVCR("<?php $obj = new DOMText(); var_dump($obj instanceof DOMNode);");

  MVCR("<?php $xml = '<?xml version=\"1.0\"?><dependencies><dependency dependency_id=\"0\" dependent_id=\"1\"/><dependency dependency_id=\"4\" dependent_id=\"5\"/><dependency dependency_id=\"5\" dependent_id=\"6\"/><dependency dependency_id=\"9\" dependent_id=\"8\"/><dependency dependency_id=\"10\" dependent_id=\"8\"/><dependency dependency_id=\"12\" dependent_id=\"13\"/><dependency dependency_id=\"12\" dependent_id=\"14\"/></dependencies>';\n"
       "$dom = new domDocument;\n"
       "$dom->loadxml($xml);\n"
       "$xpath = new DOMXPath($dom);\n"
       "$node_list = $xpath->query('//dependencies/dependency[@dependency_id = 0 and @dependent_id = 1]');\n"
       "$dependencies = $xpath->query('//dependencies')->item(0);\n"
       "$dependencies->removeChild($node_list->item(0));\n"
      );

  MVCR("<?php $xml = '<?xml version=\"1.0\"?><dependencies><dependency dependency_id=\"0\" dependent_id=\"1\"/><dependency dependency_id=\"4\" dependent_id=\"5\"/><dependency dependency_id=\"5\" dependent_id=\"6\"/><dependency dependency_id=\"9\" dependent_id=\"8\"/><dependency dependency_id=\"10\" dependent_id=\"8\"/><dependency dependency_id=\"12\" dependent_id=\"13\"/><dependency dependency_id=\"12\" dependent_id=\"14\"/></dependencies>';\n"
       "$dom = new domDocument;\n"
       "$dom->loadxml($xml);\n"
       "$xpath = new DOMXPath($dom);\n"
       "$node_list = $xpath->query('//dependencies/dependency[@dependent_id = 8]');\n"
       "foreach ($node_list as $node) {\n"
       "  var_dump($node->getAttribute($attribute));\n"
       "}\n"
      );

  MVCR("<?php\n"
       "$xmlstr = \"<?xml version='1.0' standalone='yes'?>\n"
       "<!DOCTYPE chapter SYSTEM '/share/sgml/Norman_Walsh/"
       "db3xml10/db3xml10.dtd'\n"
       "[ <!ENTITY sp \\\"spanish\\\">\n"
       "]>\n"
       "<!-- lsfj  -->\n"
       "<chapter language='en'><title language='en'>Title</title>\n"
       "<para language='ge'>\n"
       "&sp;\n"
       "<!-- comment -->\n"
       "<informaltable language='&sp;kkk'>\n"
       "<tgroup cols='3'>\n"
       "<tbody>\n"
       "<row><entry>a1</entry><entry morerows='1'>b1</entry>"
       "<entry>c1</entry></row>\n"
       "<row><entry>a2</entry><entry>c2</entry></row>\n"
       "<row><entry>a3</entry><entry>b3</entry><entry>c3</entry></row>\n"
       "</tbody>\n"
       "</tgroup>\n"
       "</informaltable>\n"
       "</para>\n"
       "</chapter> \";\n"
       "\n"
       "function print_node($node)\n"
       "{\n"
       "  print \"Node Name: \" . $node->nodeName;\n"
       "  print \"\nNode Type: \" . $node->nodeType;\n"
       "  if ($node->nodeType != 3) {\n"
       "      $child_count = $node->childNodes->length;\n"
       "  } else {\n"
       "      $child_count = 0;\n"
       "  }\n"
       "  print \"\nNum Children: \" . $child_count;\n"
       "  if($child_count <= 1){\n"
       "    print \"\nNode Content: \" . $node->nodeValue;\n"
       "  }\n"
       "  print \"\n\n\";\n"
       "}\n"
       "\n"
       "function print_node_list($nodelist)\n"
       "{\n"
       "  foreach($nodelist as $node)\n"
       "  {\n"
       "    print_node($node);\n"
       "  }\n"
       "}\n"
       "\n"
       "echo \"Test 1: accessing single nodes from php\n\";\n"
       "$dom = new domDocument;\n"
       "$dom->loadxml($xmlstr);\n"
       "if(!$dom) {\n"
       "  echo \"Error while parsing the document\n\";\n"
       "  exit;\n"
       "}\n"
       "\n"
       "// children() of of document would result in a memleak\n"
       "//$children = $dom->children();\n"
       "//print_node_list($children);\n"
       "\n"
       "echo \"--------- root\n\";\n"
       "$rootnode = $dom->documentElement;\n"
       "print_node($rootnode);\n"
       "\n"
       "echo \"--------- children of root\n\";\n"
       "$children = $rootnode->childNodes;\n"
       "print_node_list($children);\n"
       "\n"
       "// The last node should be identical with the last "
       "entry in the children array\n"
       "echo \"--------- last\n\";\n"
       "$last = $rootnode->lastChild;\n"
       "print_node($last);\n"
       "\n"
       "// The parent of this last node is the root again\n"
       "echo \"--------- parent\n\";\n"
       "$parent = $last->parentNode;\n"
       "print_node($parent);\n"
       "\n"
       "// The children of this parent are the same children as one above\n"
       "echo \"--------- children of parent\n\";\n"
       "$children = $parent->childNodes;\n"
       "print_node_list($children);\n"
       "\n"
       "echo \"--------- creating a new attribute\n\";\n"
       "//This is worthless\n"
       "//$attr = $dom->createAttribute(\"src\", \"picture.gif\");\n"
       "//print_r($attr);\n"
       "\n"
       "//$rootnode->set_attributeNode($attr);\n"
       "$attr = $rootnode->setAttribute(\"src\", \"picture.gif\");\n"
       "$attr = $rootnode->getAttribute(\"src\");\n"
       "print_r($attr);\n"
       "print \"\n\";\n"
       "\n"
       "echo \"--------- Get Attribute Node\n\";\n"
       "$attr = $rootnode->getAttributeNode(\"src\");\n"
       "print_node($attr);\n"
       "\n"
       "echo \"--------- Remove Attribute Node\n\";\n"
       "$attr = $rootnode->removeAttribute(\"src\");\n"
       "print \"Removed \" . $attr . \" attributes.\n\";\n"
       "\n"
       "echo \"--------- attributes of rootnode\n\";\n"
       "$attrs = $rootnode->attributes;\n"
       "print_node_list($attrs);\n"
       "\n"
       "echo \"--------- children of an attribute\n\";\n"
       "$children = $attrs->item(0)->childNodes;\n"
       "print_node_list($children);\n"
       "\n"
       "echo \"--------- Add child to root\n\";\n"
       "$myelement = new domElement(\"Silly\", \"Symphony\");\n"
       "$newchild = $rootnode->appendChild($myelement);\n"
       "print_node($newchild);\n"
       "print $dom->saveXML();\n"
       "print \"\n\";\n"
       "\n"
       "echo \"--------- Find element by tagname\n\";\n"
       "echo \"    Using dom\n\";\n"
       "$children = $dom->getElementsByTagname(\"Silly\");\n"
       "print_node_list($children);\n"
       "\n"
       "echo \"    Using elem\n\";\n"
       "$children = $rootnode->getElementsByTagName(\"Silly\");\n"
       "print_node_list($children);\n"
       "\n"
       "echo \"--------- Unlink Node\n\";\n"
       "print_node($children->item(0));\n"
       "$rootnode->removeChild($children->item(0));\n"
       "print_node_list($rootnode->childNodes);\n"
       "print $dom->savexml();\n"
       );

  // dom002.phpt
  MVCR("<?php\n"
       "$xml = <<<HERE\n"
       "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"
       "<foo xmlns=\"http://www.example.com/ns/foo\"\n"
       "     xmlns:fubar=\"http://www.example.com/ns/fubar\">\n"
       "  <bar><test1 /></bar>\n"
       "  <bar><test2 /></bar>\n"
       "  <fubar:bar><test3 /></fubar:bar>\n"
       "  <fubar:bar><test4 /></fubar:bar>\n"
       "</foo>\n"
       "HERE;\n"
       "\n"
       "function dump($elems) {\n"
       " foreach ($elems as $elem) {\n"
       "  var_dump($elem->nodeName);\n"
       "  dump($elem->childNodes);\n"
       "}\n"
       "}\n"
       "\n"
       "$dom = new DOMDocument();\n"
       "$dom->loadXML($xml);\n"
       "$doc = $dom->documentElement;\n"
       "dump($dom->getElementsByTagName('bar'));\n"
       "dump($doc->getElementsByTagName('bar'));\n"
       "dump($dom->getElementsByTagNameNS('http://www.example.com/ns/fubar',"
       " 'bar'));\n"
       "dump($doc->getElementsByTagNameNS('http://www.example.com/ns/fubar',"
       " 'bar'));\n"
       );

  // dom005.phpt
  MVCR("<?php\n"
       "$dom = new domdocument;\n"
       "$html = <<<EOM\n"
       "<html><head>\n"
       "<title>Hello world</title>\n"
       "</head>\n"
       "<body>\n"
       "This is a not well-formed<br>\n"
       "html files with undeclared entities&nbsp;\n"
       "</body>\n"
       "</html>\n"
       "EOM;\n"
       "$dom->loadHTML($html);\n"
       "print  \"--- save as XML\n\";\n"
       "\n"
       "print adjustDoctype($dom->saveXML());\n"
       "print  \"--- save as HTML\n\";\n"
       "\n"
       "print adjustDoctype($dom->saveHTML());\n"
       "\n"
       "function adjustDoctype($xml) {\n"
       "    return str_replace(array(\"DOCTYPE HTML\",'<p>','</p>'),"
       "array(\"DOCTYPE html\",'',''),$xml);\n"
       "}\n"
       );

  // dom006.phpt
  MVCR("<?php\n"
       "class books extends domDocument {\n"
       "  function addBook($title, $author) {\n"
       "    $titleElement = $this->createElement('title');\n"
       "    $titleElement->appendChild($this->createTextNode($title));\n"
       "    $authorElement = $this->createElement('author');\n"
       "    $authorElement->appendChild($this->createTextNode($author));\n"
       "    $bookElement = $this->createElement('book');\n"
       "    $bookElement->appendChild($titleElement);\n"
       "    $bookElement->appendChild($authorElement);\n"
       "    $this->documentElement->appendChild($bookElement);\n"
       "  }\n"
       "}\n"
       "\n"
       "$dom = new books;\n"
       "\n"
       "$xml = <<<EOM\n"
       "<?xml version='1.0' ?>\n"
       "<books>\n"
       " <book>\n"
       "  <title>The Grapes of Wrath</title>\n"
       "  <author>John Steinbeck</author>\n"
       " </book> <book>\n"
       "  <title>The Pearl</title>  <author>John Steinbeck</author>\n"
       " </book></books>\n"
       "EOM;\n"
       "\n"
       "$dom->loadXML($xml);\n"
       "$dom->addBook('PHP de Luxe', 'Richard Samar, Christian Stocker');\n"
       "print $dom->saveXML();"
       );

  // dom007.phpt
  MVCR("<?php\n"
       "$xml = <<< EOXML\n"
       "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
       "<!DOCTYPE courses [\n"
       "<!ELEMENT courses (course+)>\n"
       "<!ELEMENT course (title, description, temp*)>\n"
       "<!ATTLIST course cid ID #REQUIRED>\n"
       "<!ELEMENT title (#PCDATA)>\n"
       "<!ELEMENT description (#PCDATA)>\n"
       "<!ELEMENT temp (#PCDATA)>\n"
       "<!ATTLIST temp vid ID #REQUIRED>\n"
       "<!ENTITY test 'http://www.hpl.hp.com/semweb/2003/query_tester#'>\n"
       "<!ENTITY rdf  'http://www.w3.org/1999/02/22-rdf-syntax-ns#'>\n"
       "<!NOTATION GIF PUBLIC \"-\" \"image/gif\">\n"
       "<!ENTITY myimage PUBLIC \"-\" \"mypicture.gif\" NDATA GIF>\n"
       "]>\n"
       "<courses>\n"
       "   <course cid=\"c1\">\n"
       "      <title>Basic Languages</title>\n"
       "      <description>Introduction to Languages</description>\n"
       "   </course>\n"
       "   <course cid=\"c6\">\n"
       "      <title>French I</title>\n"
       "      <description>Introduction to French</description>\n"
       "      <temp vid=\"c7\">\n"
       "      </temp>\n"
       "   </course>\n"
       "</courses>\n"
       "EOXML;\n"
       "\n"
       "$dom = new DOMDocument();\n"
       "$dom->loadXML($xml);\n"
       "\n"
       "$dtd = $dom->doctype;\n"
       "\n"
       "/* Notation Tests */\n"
       "$nots = $dtd->notations;\n"
       "\n"
       "$length = $nots->length; var_dump($length);\n"
       "echo \"Length: \".$length.\"\n\";\n"
       "\n"
       "foreach ($nots AS $key=>$node) {\n"
       " echo \"Key $key: \".$node->nodeName.\" (\".\n"
       "$node->systemId.\") (\".$node->publicId.\")\n\";\n"
       "}\n"
       "print \"\n\";\n"
       "for($x=0; $x < $length; $x++) {\n"
       " echo \"Index $x: \".$nots->item($x)->nodeName.\" (\".\n"
       "  $nots->item($x)->systemId.\") "
       "(\".$nots->item($x)->publicId.\")\n\";\n"
       "}\n"
       "\n"
       "echo \"\n\";\n"
       "$node = $nots->getNamedItem('xxx');\n"
       "var_dump($node);\n"
       "\n"
       "echo \"\n\";\n"
       "/* Entity Decl Tests */\n"
       "$ents = $dtd->entities;\n"
       "$length = $ents->length;\n"
       "echo \"Length: \".$length.\"\n\";\n"
       "foreach ($ents AS $key=>$node) {\n"
       " echo \"Key: $key Name: \".$node->nodeName.\"\n\";\n"
       "}\n"
       "echo \"\n\";\n"
       "for($x=0; $x < $length; $x++) {\n"
       " echo \"Index $x: \".$ents->item($x)->nodeName.\"\n\";\n"
       "}\n"
       "\n"
       "echo \"\n\";\n"
       "$node = $ents->item(3);\n"
       "var_dump($node);\n"
       "$node = $ents->getNamedItem('xxx');\n"
       "var_dump($node);\n"
       );

  MVCR("<?php"
       "    function rerender($html, $frag = false) {"
       "    $doc = new DOMDocument();"
       "    if ($frag) {"
       "      $body = $doc->createDocumentFragment();"
       "      $body->appendXML($html);"
       "    } else {"
       "      $doc->loadHTML($html);"
       "      $body = $doc->documentElement;"
       "    }"
       "    return helper($body);"
       "  }"
       ""
       "  function helper($element) {"
       "    if ($element instanceof DOMText) {"
       "      return htmlspecialchars($element->nodeValue);"
       "    } else {"
       "      $body = '';"
       "      foreach ($element->childNodes as $child) {"
       "        $body .= helper($child);"
       "      }"
       ""
       "      if ($element instanceof DOMElement) {"
       "        $attrs = array();"
       "        foreach ($element->attributes as $attr) {"
       "          $attrs[] = htmlspecialchars($attr->name) . '=\"' . "
       "            htmlspecialchars($attr->value) . '\"';"
       "        }"
       "        if ($attrs) {"
       "          $attrs = ' ' . implode(' ', $attrs);"
       "        } else {"
       "          $attrs = '';"
       "        }"
       "        return '<' . $element->tagName . $attrs . '>' . $body . "
       "          '</' . $element->tagName . '>';"
       "      } else {"
       "        return $body;"
       "      }"
       "    }"
       "  }"
       ""
       "  $fragment = 'Hello, <b>world</b>.';"
       "  $document = '<html><body><div style=\"color:red\">"
       "    <p class=\"thing\">'.$fragment.'</p></div>';"
       ""
       "  echo rerender($fragment, true).\"\n\n\";"
       "  echo rerender($document, false).\"\n\n\";"
       );

  MVCR("<?php "
       "$xml ="
       "  '<root>$1 - <template><title>SITENAME</title></template></root>';"
       "$dom = new DOMDocument();"
       "$dom->loadXML($xml);"
       "new foo($dom->documentElement);"
       "class foo {"
       "  function foo($a) {"
       "    var_dump($a);"
       "  }"
       "}");

  MVCR("<?php\n"
       "$dom = new DOMDocument();\n"
       "$dom->loadXML('<a><b><c /></b></a>');\n"
       "$remove = array();\n"
       "foreach ($dom->getElementsByTagName('b') as $data) {\n"
       "  foreach ($data->childNodes as $element) {\n"
       "    if ($element instanceof DOMElement) {\n"
       "      $remove[] = $element;\n"
       "    }\n"
       "  }\n"
       "}\n"
       "foreach ($remove as $r) {\n"
       "  $r->parentNode->removeChild($r);\n"
       "}\n"
       "echo $dom->saveXML();\n"
      );

  return true;
}

bool TestCodeRun::TestFile() {
  //MVCR("<?php "
  //    "$gif = imagecreatefromgif('http://www.php.net/images/php.gif');"
  //    "imagegif($gif);"
  //    "imagedestroy($gif);");
  MVCR("<?php "
      "file_put_contents(\"/tmp/temp.txt\","
      "                  \"put this in the txt file\\n\");"
      "$txt = file_get_contents(\"/tmp/temp.txt\");"
      "echo $txt;"
      "file_put_contents(\"compress.zlib:///tmp/temp.zip\","
      "                  \"put this in the zip file\\n\");"
      "$zip = file_get_contents(\"compress.zlib:///tmp/temp.zip\");"
      "echo $zip;");
  MVCR("<?php "
      "$fh = fopen('php://output', 'w');"
      "if (!$fh) {"
      "  throw new Exception('foo');"
      "}"
      "fprintf($fh, \"hello\\n\");"
      "var_dump(fflush($fh));"
      "var_dump(fclose($fh));"
      "$fh = fopen('php://output', 'a');"
      "if (!$fh) {"
      "  throw new Exception('foo');"
      "}"
      "fprintf($fh, \"hello\\n\");"
      "var_dump(fflush($fh));"
      "var_dump(fclose($fh));"
      "$fh = fopen('php://output', 'r');"
      "if (!$fh) {"
      "  throw new Exception('foo');"
      "}"
      "fprintf($fh, \"hello\\n\");"
      "var_dump(fflush($fh));"
      "var_dump(fclose($fh));");
  MVCR("<?php "
      "var_dump(filetype('test/test_ext_file2.tmp'));"
      "var_dump(is_link('test/test_ext_file2.tmp'));"
      "$a = lstat('test/test_ext_file2.tmp');"
      "var_dump($a['mtime']);");
  MVCR("<?php "
      "$f = fopen('php://stdout', 'w');"
      "fprintf($f, 'stdout');");
  MVCR("<?php "
      "$input = fopen('/tmp/junk.txt', 'w+');"
      "fwrite($input, 'hello world');"
      ""
      "fseek($input, 0);"
      "$output = fopen('php://memory', 'w+');"
      "stream_copy_to_stream($input, $output);"
      "fseek($output, 0);"
      "$bytes = fread($output, 1024);"
      "print \"From file, without Maxlen: <\".serialize($bytes).\">.\\n\";"
      ""
      "fseek($input, 0);"
      "$output = fopen('php://memory', 'w+');"
      "stream_copy_to_stream($input, $output, null);"
      "fseek($output, 0);"
      "$bytes = fread($output, 1024);"
      "print \"From file, using Maxlen null: <\".serialize($bytes).\">.\\n\";"
      ""
      ""
      "fseek($input, 0);"
      "$output = fopen('php://memory', 'w+');"
      "stream_copy_to_stream($input, $output, -1);"
      "fseek($output, 0);"
      "$bytes = fread($output, 1024);"
      "print \"From file, using Maxlen -1: <\".serialize($bytes).\">.\\n\";");
  MVCR("<?php "
      "mkdir('test/tmp_dir');"
      "print_r(glob('test/tmp_dir/*'));"
      "rmdir('test/tmp_dir');");
  MVCR("<?php "
      "$src = tmpfile();"
      "$dst = tmpfile();"
      "fwrite($src, 'a');"
      "fseek($src, 0);"
      "stream_copy_to_stream($src, $dst);"
      "fseek($dst, 0);"
      "$str = stream_get_contents($dst);"
      "fseek($src, 0);"
      "stream_copy_to_stream($src, $dst);"
      "fseek($dst, 0);"
      "$str = stream_get_contents($dst);"
      "echo $str;");
  MVCR("<?php "
      "define('FILENAME', '/tmp/flock_file.dat');"
      "function flock_or_die($filename, $resource, $flock_op) {"
      "  $r = flock($resource, $flock_op);"
      "  var_dump($r); "
      "}"
      "$resource = fopen(FILENAME, 'w');"
      "flock_or_die(FILENAME, $resource, LOCK_EX);"
      "flock_or_die(FILENAME, $resource, LOCK_UN);"
      "unlink(FILENAME);");
  MVCR("<?php "
       "$h = popen(\"echo foo; exit 2\", 'r');"
       "$content = stream_get_contents($h);"
       "$result = pclose($h);"
       "echo trim($content).\"/\".$result.\"/\".gettype($result).\"\\n\";");
  MVCR("<?php "
       "$fp = fopen('test/test_ext_file.txt', 'r');"
       "var_dump(pclose($fp));");
  MVCR("<?php "
       "$fp = fopen('test/nonexist.txt', 'r');"
       "var_dump(pclose($fp));");
  return true;
}

bool TestCodeRun::TestDirectory() {
  MVCR("<?php "
      "$d = dir(\"test/\");"
      "echo \"Path: \" . $d->path . \"\\n\";"
      "while (false !== ($entry = $d->read())) {"
      "   echo $entry.\"\\n\";"
      "}"
      "$d->rewind();"
      "while (false !== ($entry = $d->read())) {"
      "   echo $entry.\"\\n\";"
      "}"
      "$d->close();");
  return true;
}

bool TestCodeRun::TestBadFunctionCalls() {
  // make sure no error
  MVCR("<?php "
      "error_reporting(E_ALL & ~E_NOTICE);"
      "class A { function __construct() {}} $obj = new A(10);");

  // make sure foo() is still called
  MVCR("<?php "
      "error_reporting(E_ALL & ~E_NOTICE);"
      "function foo($a) { print $a;} "
      "class A { function __construct() {}} $obj = new A(foo(10));");

  // make sure 1st parameter is corrected passed in
  MVCR("<?php "
      "error_reporting(E_ALL & ~E_NOTICE);"
      "function foo($a) { print $a;} function bar($a) { return $a;}"
      " foo('ok', bar('bad'));");
  // Too many args
  MVCR("<?php "
      "error_reporting(E_ALL & ~E_NOTICE);"
      "function foo($x) {}"
      "function z() {"
      "  $yay = 1;"
      "  $snarf = 2;"
      "  foo(1,foo(1), $yay,$snarf);"
      "}"
      "z();");

  // Ensure all arguments are evaluated
  MVCR("<?php\n"
      "function f() {\n"
      "  $a = array();\n"
      "  array_push($a[] = 1);\n"
      "  var_dump($a);\n"
      "}\n"
      "f();\n");
  MVCR("<?php\n"
      "function f() {\n"
      "  $a = 0;\n"
      "  array_chunk($a = 1);\n"
      "  var_dump($a);\n"
      "}\n"
      "f();\n");
  MVCR("<?php\n"
      "function f() {\n"
      "  $a = 0;\n"
      "  $b = 0;\n"
      "  $c = 0;\n"
      "  $d = 0;\n"
      "  array_chunk($a = 1, $b = 2, $c = 3, $d = 4);\n"
      "  var_dump($a, $b, $c, $d);\n"
      "}\n"
      "f();\n");
  MVCR("<?php\n"
      "function f() {\n"
      "  $arr = array();\n"
      "  sort($arr[0]);\n"
      "  var_dump($arr);\n"
      "  unset($arr);\n"
      "  $arr = array();\n"
      "  sort($arr[0],0,0,0,0,0,0,0,0);\n"
      "  var_dump($arr);\n"
      "}\n"
      "f();\n");

  return true;
}

bool TestCodeRun::TestConstructor() {
  // class-name constructors should not be renamed
  MVCR("<?php class A { function a() { echo \"A\n\"; }}"
      "function test() { $obj = new A(); $obj->a(); }"
      "test();");
  // __construct takes priority
  MVCR("<?php "
      "class A {"
      "  function a() { echo \"A\n\"; }"
      "  function __construct() { echo \"cons\n\"; }"
      "} "
      "function test() { $obj = new A(); $obj->a(); } "
      "test();");
  MVCR("<?php "
      "class A {"
      "  public function A() {"
      "    echo \"In A\\n\";"
      "    $this->__construct();"
      "  }"
      "  public function __construct() {"
      "    echo \"In A::__construct\\n\";"
      "  }"
      "}"
      "class B extends A {"
      "  public function B() {"
      "    echo \"In B\\n\";"
      "    $this->A();"
      "  }"
      "}"
      "$obj = new B();"
      ""
      "class A2 {"
      "  public function __construct() {"
      "    echo \"In A2::__construct\\n\";"
      "    $this->B2();"
      "  }"
      "  public function B2() {"
      "    echo \"In B2\\n\";"
      "  }"
      "}"
      "class B2 extends A2 {"
      "  public function __construct() {"
      "    echo \"In B2::__construct\\n\";"
      "    parent::__construct();"
      "  }"
      "}"
      "$obj = new B2();"
      "class C {"
      "  public function C() {}"
      "}"
      "class D extends C {"
      "  public function __construct() {"
      "    echo \"In D::__construct\\n\";"
      "    C::__construct();"
      "  }"
      "}"
      "$obj = new D;"
      "$obj->c();"
      "class E {"
      "  public function E() {"
      "    echo \"In E\\n\";"
      "  }"
      "  public function foo() {"
      "    $this->E();"
      "    E::__construct();"
      "  }"
      "}"
      "$obj = new E;"
      "$obj->foo();");

  return true;
}

bool TestCodeRun::TestTernary() {
  MVCR("<?php $t = true; $a = $t ? \"hello\" : \"world\"; var_dump($a);");
  MVCR("<?php $f = false; $a = $f ? 5 : \"hello\"; var_dump($a);");
  MVCR("<?php $t = true; $a = $t ? \"hello\" : null; var_dump($a);");
  MVCR("<?php "
      "function memcache_init_split_vars() {"
      "  global $_SERVER;"
      "  global $MEMCACHED_SPLIT_HASH;"
      "  $MEMCACHED_SPLIT_HASH ="
      "    crc32(empty($_SERVER['SERVER_ADDR']) ? php_uname('n')"
      "                                         : $_SERVER['SERVER_ADDR']);"
      "}");
  MVCR("<?php "
      "function f() {} function g() {} "
      "$t = true;"
      "$a = $t ? f() : g();"
      "var_dump($a);");
  MVCR("<?php function test($a) { $b = $a + 1 == 5 ? 5 : 7; } test(4);");
  MVCR("<?php $t = true; $f = false;"
      "$a = $t ? null : ($f ? \"hello\" : \"world\");");
  MVCR("<?php $t = true; $a = $t ? \"\" : \"a\" . $t . \"b\";");
  MVCR("<?php "
      "function add_cssclass($add, $class) {"
      "  $class = empty($class) ? $add : $class .= ' ' . $add;"
      "  return $class;"
      "}"
      "add_cssclass('test', $a);");
  MVCR("<?php "
      "$a = 123;"
      "echo $a ? @mysql_data_seek(null, null) : false;");

  MVCR("<?php "
       "function foo($a) {"
       "  $x = $a ? 1 : 0;"
       "  return $x - 5;"
       "}"
       "var_dump(foo(1, 2, 3));"
       "var_dump(foo(0, 2, 3));");

  return true;
}

bool TestCodeRun::TestUselessAssignment() {
  MVCR("<?php "
      "class MyDestructableClass {"
      "   function __construct() {"
      "       print \"In constructor\\n\";"
      "       $this->name = \"MyDestructableClass\";"
      "   }"
      ""
      "   function __destruct() {"
      "       print \"Destroying \" . $this->name . \"\\n\";"
      "   }"
      "}"
      "function foo($a) {"
      "  if ($a) return new MyDestructableClass();"
      "  return false;"
      "}"
      "function bar($a) {"
      "  if ($a) {"
      "    $obj = foo(1);"
      "    $obj = 1;"
      "    var_dump(2);"
      "  }"
      "  var_dump(1);"
      "}"
      "bar(1);");
  MVCR("<?php "
      "class MyDestructableClass {"
      "   function __construct() {"
      "       print \"In constructor\\n\";"
      "       $this->name = \"MyDestructableClass\";"
      "   }"
      ""
      "   function __destruct() {"
      "       print \"Destroying \" . $this->name . \"\\n\";"
      "   }"
      "}"
      "function foo(&$a) {"
      "  $a = new MyDestructableClass();"
      "}"
      "function bar($a) {"
      "  if ($a) {"
      "    $b = array(1, 2, 3);"
      "    var_dump($b);"
      "    foo($dummy = array(1, 2, 3));"
      "    $c = array(1, 2, 3);"
      "    var_dump($c);"
      "  }"
      "}"
      "bar(1);");

  MVCR("<?php "
       "function out($a) {"
       "  echo $a,'\\n';"
       "}"
       "function test($a) {"
       "  $a ? out('?a') : out(':a');"
       "  $a ? out('+a') : 0;"
       "  $a ? 0 : out('-a');"
       "  $a && out('&&a');"
       "  $a || out('||a');"
       "  $a and out('and a');"
       "  $a or out('or a');"
       "  $b = $c = 0;"
       "  $a || (($b = 5) + ($c = 6));"
       "  out($b); out($c);"
       "}"
       "test(0);"
       "test('foo');");
  MVCR("<?php "
      "class A {"
      "  function __destruct() {"
      "    var_dump('done');"
      "  }"
      "}"
      ""
      "function foo() {"
      "  $a = 10;"
      "  if ($a == 11) {"
      "    return null;"
      "  }"
      "  return new A();"
      "}"
      ""
      "function bar() {"
      "  $a = foo();"
      "  var_dump('doing');"
      "}"
      ""
      "bar();");
  MVCR("<?php "
      "function foo($p) {"
      "  global $b;"
      "  for ($i = 0; $i < 5; $i++) {"
      "    if ($i > $p) {"
      "      $a = 10;"
      "    } else {"
      "      $a = &$b;"
      "    }"
      "  }"
      "}"
      "function bar() {"
      "  $a = foo(2);"
      "  var_dump($GLOBALS['b']);"
      "}"
      "bar();");
  MVCR("<?php "
      "function bar() {}"
      "function foo() {"
      "  $foo = bar();"
      "  unset($foo);"
      "}"
      "foo();");

  MVCR("<?php "
       "function bug( $flag ) {"
       "  $tag = '';"
       "  if ($flag) {"
       "    $tag .= 'x';"
       "  }"
       "  $tag='33';"
       "  if ( $flag ) ; else var_dump($tag);"
       "}"
       "bug(false);");

  return true;
}

bool TestCodeRun::TestTypes() {
  MVCR("<?php "
      "function foo($m, $n) {"
      "  $offset_change = 10;"
      "  $offset_change -= strlen($m) - strlen($n);"
      "  var_dump($offset_change);"
      "}"
      "foo('abc', 'efg');");
  MVCR("<?php "
      "function p(array $i = null) {"
      "  var_dump($i);"
      "  $i = array();"
      "}"
      "p();"
      "function q() {"
      "  p(null);"
      "}");
  MVCR("<?php "
      "function foo($p) {"
      "  if ($p) {"
      "    $a = array();"
      "  }"
      "  var_dump((string)$a);"
      "}"
      "foo(false);");

  MVCR("<?php "
       "class X {};"
       "function bug() {"
       "  if (!$GLOBALS['x']) {"
       "    return;"
       "  }"
       "  return new X;"
       "}"
       "var_dump(bug());");

  return true;
}

bool TestCodeRun::TestSwitchStatement() {
  MVCR("<?php class A {} $a = new A();"
      "switch ($a) { "
      "case 'foo': "
      "default:"
      "}");

  MVCR("<?php class A {};"
      "switch (new A()) { "
      "case 'foo': "
      "default:"
      "}");

  MVCR("<?php "
       "function test() {"
       "  $a = 2;"
       "  switch ($a) {"
       "    case ++$a: var_dump('ok'); break;"
       "    case 2: var_dump('broken'); break;"
       "    case 3: var_dump('really broken'); break;"
       "    default: var_dump('fail'); break;"
       "  }"
       "  $a = 'b';"
       "  $b = 2;"
       "  switch ($$a) {"
       "    case ++$$a: var_dump('broken'); break;"
       "    case 2: var_dump('ok'); break;"
       "    case 3: var_dump('really broken'); break;"
       "    default: var_dump('fail'); break;"
       "  }"
       "}"
       "$a = 2;"
       "switch ($a) {"
       "  case ++$a: var_dump('ok'); break;"
       "  case 2: var_dump('broken'); break;"
       "  case 3: var_dump('really broken'); break;"
       "  default: var_dump('fail'); break;"
       "}"
       "$a = 'b';"
       "$b = 2;"
       "switch ($$a) {"
       "  case ++$$a: var_dump('broken'); break;"
       "  case 2: var_dump('ok'); break;"
       "  case 3: var_dump('really broken'); break;"
       "  default: var_dump('fail'); break;"
       "}"
       "test();");

  return true;
}

bool TestCodeRun::TestExtString() {
  MVCR("<?php "
      "var_dump(strtr(\"\", \"ll\", \"a\"));"
      "var_dump(strtr(\"hello\", \"\", \"a\"));"
      "var_dump(strtr(\"hello\", \"ll\", \"a\"));"
      "var_dump(strtr(\"hello\", array(\"\" => \"a\")));"
      "var_dump(strtr(\"hello\", array(\"ll\" => \"a\")));");
  MVCR("<?php "
      "var_dump(explode('', ''));"
      "$str = 'Hello Friend';"
      "var_dump(str_split($str, -3));"
      "var_dump(chunk_split('-=blender=-', -3, '-=blender=-')); "
      "var_dump(strpbrk('hello', ''));"
      "var_dump(substr_count('hello', ''));"
      "var_dump(substr_count('hello', 'o', -1));"
      "var_dump(substr_count('hello', 'o', 2, -1));"
      "var_dump(substr_count('hello', 'o', 2, 100));"
      "var_dump(count_chars('hello', 100));"
      "var_dump(str_word_count('hello', 100));"
      "var_dump(strtr('hello', 100));"
      "var_dump(implode('abcd', 'abcd'));");
  MVCR("<?php "
      "error_reporting(0);"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', array(0)));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', array(0), 3));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', array(0), 1.0));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', array(0), null));"
      "$obj = new stdClass();"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', 0, $obj));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', '0', '1.0'));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', '0.0', 1.0));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', '0.0', 1));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob', 0.0, '1'));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', 'bob',"
      "                        array(0), array(1)));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', array('bob'),"
      "                        array(0), array(3,4)));"
      "var_dump(substr_replace('ABCDEFGH:/MNRPQR/', array('bob'),"
      "                        array(0), array(3)));"
      "var_dump(substr_replace(array('ABCDEFGH:/MNRPQR/'), array(),"
      "                        array(0,1), array(3, 4)));"
      "var_dump(substr_replace(array('ABCDEFGH:/MNRPQR/'), array('bob'),"
      "                        array(0,1), array(3)));"
      "var_dump(substr_replace(array('ABCDEFGH:/MNRPQR/'),"
      "                        array('bob', 'cat'), 0));"
      "var_dump(substr_replace(array('ABCDEFGH:/MNRPQR/'),"
      "                        array('bob'), array(0,1)));"
      "var_dump(substr_replace('abc', 'xyz', 3, 0));"

      "var_dump(sscanf(\"SN/2350001\", \"SN/%d\"));"
      "var_dump(sscanf(\"SN/2350001\", \"SN/%d\", $out));"
      "var_dump($out);"
      "var_dump(sscanf(\"SN/abc\", \"SN/%d\", $out));"
      "var_dump($out);"
      "var_dump(sscanf(\"30\", \"%da\", $out));"
      "var_dump($out);"
      "var_dump(sscanf(\"-\", \"%da\", $out));"
      "var_dump($out);");
  return true;
}

bool TestCodeRun::TestExtArray() {
  MVCR("<?php "
      "error_reporting(0);"
      "var_dump(range('', '', 1));"
      "var_dump(range('', '', -1));"
      "var_dump(range('9', '10', -1));"
      "var_dump(range(9, 10, -1));"
      "var_dump(range(9, 10, -1.5));"
      "var_dump(range(9, 10, 33333333.33));"
      "var_dump(range(9, 10, -33333333.33));"
      "var_dump(range(9223372036854775807, 9223372036854775805, -1));"
      "var_dump(range(9223372036854775807, 9223372036854775805,"
      "               9223372036854775807));"
      "var_dump(range(9223372036854775807, 9223372036854775805,"
      "               -9223372036854775807));"
      "var_dump(range(9223372036854775807, 9223372036854775805,"
      "               2147483648));"
      "var_dump(range(9223372036854775807, 9223372036854775805,"
      "               -2147483648));"
      "var_dump(range('9', '10', '-1'));"
      "var_dump(range('9', '10', '-1.5'));"
      "var_dump(range('9', '10', '33333333.33'));"
      "var_dump(range('9', '10', '-33333333.33'));"
      "var_dump(range('9223372036854775807', '9223372036854775805', '-1'));"
      "var_dump(range('9223372036854775807', '9223372036854775805',"
      "               '9223372036854775807'));"
      "var_dump(range('9223372036854775807', '9223372036854775805',"
      "               '-9223372036854775807'));"
      "var_dump(range(null, null, -2.5));"
      "var_dump(range(null, null, 3.5));"
      "var_dump(range(null, null, null));"
      "var_dump(range(3.5, -4.5, null));");
  MVCR("<?php "
      "var_dump(array_fill(-2, -2, 'pear'));"
      "var_dump(array_combine(array(1, 2), array(3)));"
      "var_dump(array_combine(array(), array()));"
      "var_dump(array_chunk(1));"
      "var_dump(array_chunk(array()));"
      "$a = array(1, 2);"
      "var_dump(asort($a, 100000));");
  MVCR("<?php\n"
      "function f(&$val,$key) {\n"
      "  echo \"k=$key v=$val\\n\";\n"
      "  $val = $val + 1;\n"
      "}\n"
      "$arr = array(0,1,2);\n"
      "array_walk($arr,'f');\n"
      "var_dump($arr);\n");
  MVCR("<?php\n"
      "function f($val,$key) {\n"
      "  echo \"k=$key v=$val\\n\";\n"
      "}\n"
      "$arr = array(0,1,2);\n"
      "array_walk($arr,'f');\n");
  MVCR("<?php\n"
      "$arr = array(0,1,2);\n"
      "function f($val,$key) {\n"
      "  global $arr;\n"
      "  echo \"k=$key v=$val\\n\";\n"
      "  if ($key == 0) {\n"
      "    unset($arr[1]);\n"
      "  }\n"
      "}\n"
      "array_walk($arr,'f');\n"
      "var_dump($arr);\n");
  MVCR("<?php\n"
      "$a = array('foo'=>array('bar'=>1));\n"
      "function fix(&$v, $k) { $v *= 2; }\n"
      "array_walk_recursive($a, 'fix');\n"
      "var_dump($a['foo']);\n");

  return true;
}

bool TestCodeRun::TestExtFile() {
  MVCR("<?php "
      "error_reporting(0);"
      "$fp = fopen('/tmp/lock.txt', 'w');"
      "fclose($fp);"
      "$fp = fopen('/tmp/lock.txt', 'r+');"
      "var_dump(flock($fp, 0xf0));"
      "fclose($fp);");

  return true;
}

bool TestCodeRun::TestExtDate() {
  MVCR("<?php "
      "error_reporting(0);"
      "var_dump(idate('@@'));"
      "var_dump(idate('@'));"
      "var_dump(date(''));"
      "var_dump(date('@'));"
      "var_dump(strftime(''));"
      "setlocale(LC_ALL, 'nl_NL');"
      "var_dump(strftime('%p'));");

  return true;
}

bool TestCodeRun::TestExtImage() {
  MVCR("<?php "
      "$data = 'iVBORw0KGgoAAAANSUhEUgAAABwAAAASCAMAAAB/2U7WAAAABl'"
      "       . 'BMVEUAAAD///+l2Z/dAAAASUlEQVR4XqWQUQoAIAxC2/0vXZDr'"
      "       . 'EX4IJTRkb7lobNUStXsB0jIXIAMSsQnWlsV+wULF4Avk9fLq2r'"
      "       . '8a5HSE35Q3eO2XP1A1wQkZSgETvDtKdQAAAABJRU5ErkJggg==';"
      "$data = base64_decode($data);"
      ""
      "$im = imagecreatefromstring($data);"
      "if ($im !== false) {"
      "    header('Content-Type: image/png');"
      "    imagepng($im);"
      "    imagedestroy($im);"
      "}"
      "else {"
      "    echo 'An error occurred.';"
      "}");
  MVCR("<?php "
      "header ('Content-type: image/png');"
      "$im = imagecreatetruecolor(120, 20);"
      "$text_color = imagecolorallocate($im, 233, 14, 91);"
      "imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);"
      "imagepng($im);"
      "imagedestroy($im);");
  MVCR("<?php "
      "// Create a 55x30 image"
      "$im = imagecreatetruecolor(55, 30);"
      "$red = imagecolorallocate($im, 255, 0, 0);"
      "$black = imagecolorallocate($im, 0, 0, 0);"
      ""
      "// Make the background transparent"
      "imagecolortransparent($im, $black);"
      ""
      "// Draw a red rectangle"
      "imagefilledrectangle($im, 4, 4, 50, 25, $red);"
      ""
      "// Save the image"
      "imagepng($im, './imagecolortransparent.png');"
      "imagedestroy($im);");
  MVCR("<?php "
      "$image = imagecreatefromgif('test/images/php.gif');"
      ""
      "$emboss = array(array(2, 0, 0), array(0, -1, 0), array(0, 0, -1));"
      "imageconvolution($image, $emboss, 1, 127);"
      ""
      "header('Content-Type: image/png');"
      "imagepng($image, null, 9);");
  MVCR("<?php "
      "$image = imagecreatetruecolor(180,40);"
      ""
      "// Writes the text and apply a gaussian blur on the image"
      "imagestring($image, 5, 10, 8, 'Gaussian Blur Text', 0x00ff00);"
      "$gaussian = array(array(1.0, 2.0, 1.0),"
      "                  array(2.0, 4.0, 2.0),"
      "                  array(1.0, 2.0, 1.0));"
      "imageconvolution($image, $gaussian, 16, 0);"
      ""
      "// Rewrites the text for comparison"
      "imagestring($image, 5, 10, 18, 'Gaussian Blur Text', 0x00ff00);"
      ""
      "header('Content-Type: image/png');"
      "imagepng($image, null, 9);");
  MVCR("<?php "
      "// File and new size"
      "$filename = 'test/images/simpletext.jpg';"
      "$percent = 0.5;"
      "// Content type"
      "header('Content-type: image/jpeg');"
      "// Get new sizes"
      "list($width, $height) = getimagesize($filename, $nfo);"
      "var_dump($nfo);"
      "$newwidth = $width * $percent;"
      "$newheight = $height * $percent;"
      "// Load"
      "$thumb = imagecreatetruecolor($newwidth, $newheight);"
      "$source = imagecreatefromjpeg($filename);"
      "// Resize"
      "imagecopyresized($thumb, $source, 0, 0, 0, 0,"
      "                 $newwidth, $newheight, $width, $height);"
      "// Output"
      "imagejpeg($thumb);");
  MVCR("<?php "
      "// create image"
      "$image = imagecreatetruecolor(100, 100);"
      ""
      "// allocate some solors"
      "$white    = imagecolorallocate($image, 0xFF, 0xFF, 0xFF);"
      "$gray     = imagecolorallocate($image, 0xC0, 0xC0, 0xC0);"
      "$darkgray = imagecolorallocate($image, 0x90, 0x90, 0x90);"
      "$navy     = imagecolorallocate($image, 0x00, 0x00, 0x80);"
      "$darknavy = imagecolorallocate($image, 0x00, 0x00, 0x50);"
      "$red      = imagecolorallocate($image, 0xFF, 0x00, 0x00);"
      "$darkred  = imagecolorallocate($image, 0x90, 0x00, 0x00);"
      ""
      "// make the 3D effect"
      "for ($i = 60; $i > 50; $i--) {"
      "   imagefilledarc($image, 50, $i, 100, 50, 0, 45,"
      "                  $darknavy, IMG_ARC_PIE);"
      "   imagefilledarc($image, 50, $i, 100, 50, 45, 75,"
      "                  $darkgray, IMG_ARC_PIE);"
      "   imagefilledarc($image, 50, $i, 100, 50, 75, 360,"
      "                  $darkred, IMG_ARC_PIE);"
      "}"
      ""
      "imagefilledarc($image, 50, 50, 100, 50, 0, 45, $navy, IMG_ARC_PIE);"
      "imagefilledarc($image, 50, 50, 100, 50, 45, 75 , $gray, IMG_ARC_PIE);"
      "imagefilledarc($image, 50, 50, 100, 50, 75, 360 , $red, IMG_ARC_PIE);"
      ""
      ""
      "// flush image"
      "header('Content-type: image/png');"
      "imagepng($image);"
      "imagedestroy($image);");
  MVCR("<?php "
      "// Create a new image instance"
      "$im = imagecreatetruecolor(100, 100);"
      ""
      "// Make the background white"
      "imagefilledrectangle($im, 0, 0, 99, 99, 0xFFFFFF);"
      ""
      "// Draw a text string on the image"
      "imagestring($im, 3, 40, 20, 'GD Library', 0xFFBA00);"
      ""
      "// Output the image to browser"
      "header('Content-type: image/gif');"
      ""
      "imagegif($im);"
      "imagedestroy($im);");
  MVCR("<?php "
      "$png = imagecreatefrompng('test/images/smile.happy.png');"
      ""
      "// Save the image as a GIF"
      "imagegif($png, '/tmp/php.gif');"
      ""
      "// Free from memory"
      "imagedestroy($png);");
  MVCR("<?php "
      "// Create an image instance"
      "$im = imagecreatefromgif('test/images/php.gif');"
      ""
      "// Enable interlancing"
      "imageinterlace($im, true);"
      ""
      "// Save the interfaced image"
      "imagegif($im, './php_interlaced.gif');"
      "imagedestroy($im);");
  MVCR("<?php "
      "// Create a blank image and add some text"
      "$im = imagecreatetruecolor(120, 20);"
      "$text_color = imagecolorallocate($im, 233, 14, 91);"
      "imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);"
      ""
      "// Save the image as 'simpletext.jpg'"
      "imagejpeg($im, 'simpletext.jpg');"
      ""
      "// Free up memory"
      "imagedestroy($im);");
  MVCR("<?php "
      "// Create a blank image and add some text"
      "$im = imagecreatetruecolor(120, 20);"
      "$text_color = imagecolorallocate($im, 233, 14, 91);"
      "imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);"
      ""
      "// Set the content type header - in this case image/jpeg"
      "header('Content-type: image/jpeg');"
      ""
      "// Skip the filename parameter using NULL, then set the quality to 75%"
      "imagejpeg($im, NULL, 75);"
      ""
      "// Free up memory"
      "imagedestroy($im);");
  MVCR("<?php "
      "// Create a blank image and add some text"
      "$im = imagecreatetruecolor(120, 20);"
      "$text_color = imagecolorallocate($im, 233, 14, 91);"
      "imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);"
      ""
      "// Set the content type header - in this case image/jpeg"
      "header('Content-type: image/jpeg');"
      ""
      "// Output the image"
      "imagejpeg($im);"
      ""
      "// Free up memory"
      "imagedestroy($im);");
  MVCR("<?php "
      "// Create a 100*30 image"
      "$im = imagecreate(100, 30);"
      ""
      "// White background and blue text"
      "$bg = imagecolorallocate($im, 255, 255, 255);"
      "$textcolor = imagecolorallocate($im, 0, 0, 255);"
      ""
      "// Write the string at the top left"
      "imagestring($im, 5, 0, 0, 'Hello world!', $textcolor);"
      ""
      "// Output the image"
      "header('Content-type: image/png');"
      ""
      "imagepng($im);"
      "imagedestroy($im);");
  MVCR("<?php "
      "var_dump(image_type_to_mime_type(IMAGETYPE_GIF));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_JPEG));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_PNG));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_SWF));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_PSD));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_BMP));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_TIFF_II));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_TIFF_MM));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_JPC));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_JP2));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_JPX));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_JB2));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_SWC));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_IFF));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_WBMP));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_XBM));"
      "var_dump(image_type_to_mime_type(IMAGETYPE_ICO));");
  MVCR("<?php "
      "function foo($text, $fsize) {"
      ""
      "  $font = 'test/tahoma.ttf';"
      "  $font_angle = 0;"
      "  $background1 = 125;"
      "  $background2 = 125;"
      "  $background3 = 125;"
      "  $font1 = 60;"
      "  $font2 = 60;"
      "  $font3 = 60;"
      ""
      "  $bbox = imagettfbbox($fsize, $font_angle, $font, $text);"
      "  $text_width = abs(max($bbox[2],"
      "                    $bbox[4]) - min($bbox[0], $bbox[6]));"
      "  $text_height = abs(max($bbox[7],"
      "                     $bbox[5]) - min($bbox[1], $bbox[3]));"
      "  $image_width = $text_width + 2;"
      ""
      "  $image_height = max($text_height, $fsize) + 2;"
      ""
      "  $image = imagecreate($image_width, $image_height);"
      ""
      "  imagecolorallocate($image,"
      "                     $background1,"
      "                     $background2,"
      "                     $background3);"
      ""
      "  $black = imagecolorallocate($image,"
      "                              $font1,"
      "                              $font2,"
      "                              $font3);"
      ""
      "  $y = $fsize + 1;"
      "  imagettftext($image, $fsize, $font_angle, 0,"
      "               $y, $black, $font, $text);"
      "  imagepng($image);"
      ""
      "  imagedestroy($image);"
      "}"
      ""
      "$text = 'foobar@yahoo.com';"
      "$fsize = '9.8';"
      "foo($text, $fsize);");
  MVCR("<?php "
      "for ($i = 0; $i < 100000; $i++) {"
      "  $str =  exif_tagname($i);"
      "  if ($str) {"
      "    echo \"$i: $str\\n\";"
      "  }"
      "}");
  MVCR("<?php "
      "$filename = 'test/images/test1pix.jpg';"
      "$image = exif_thumbnail($filename, $width, $height, $type);"
      "if ($image!==false) {"
      "  header('Content-type: ' .image_type_to_mime_type($type));"
      "  var_dump($width, $height, $type);"
      "} else {"
      "  echo 'No thumbnail available';"
      "}");
  MVCR("<?php "
      "$filename = 'test/images/simpletext.jpg';"
      "$degrees = 90;"
      "header('Content-type: image/jpeg');"
      "$source = imagecreatefromjpeg($filename);"
      "$rotate = imagerotate($source, $degrees, 0);"
      "imagejpeg($rotate);");
  MVCR("<?php "
      "$exif = exif_read_data('test/images/246x247.png');"
      "print_r($exif);"
      "$exif = exif_read_data('test/images/php.gif');"
      "print_r($exif);"
      "$exif = exif_read_data('test/images/simpletext.jpg');"
      "print_r($exif);"
      "$exif = exif_read_data('test/images/smile.happy.png');"
      "print_r($exif);"
      "$exif = exif_read_data('test/images/test1pix.jpg');"
      "print_r($exif);"
      "$exif = exif_read_data('test/images/test2.jpg');"
      "print_r($exif);");
  return true;
}

bool TestCodeRun::TestExtSplFile() {
  MVCR("<?php "
      "$info = new SplFileInfo('test');"
      "if (!$info->isFile()) {"
      "    echo $info->getRealPath();"
      "}"
      "$info = new SplFileInfo('test/test_code_run.cpp');"
      "var_dump($info->getbaseName());"
      "var_dump($info->getbaseName('.cpp'));"
      "echo 'Last changed at ' . date('g:i a', $info->getCTime());"
      "var_dump($info->getGroup());"
      "var_dump($info->getInode());"
      "var_dump($info->getMTime());"
      "var_dump($info->getOwner());"
      "var_dump($info->getPerms());"
      "var_dump($info->getSize());"
      "var_dump($info->getType());"
      "var_dump($info->isDir());"
      "var_dump($info->isFile());"
      "var_dump($info->isLink());"
      "var_dump($info->isReadable());"
      "var_dump($info->isWritable());");
  MVCR("<?php "
      "$info = new SplFileInfo('test');"
      "var_dump($info->getRealPath());"
      "var_dump($info->getPath());"
      "var_dump($info->getPathName());"
      "$info = new SplFileInfo('test/');"
      "var_dump($info->getRealPath());"
      "var_dump($info->getPath());"
      "var_dump($info->getPathName());"
      "$info = new SplFileInfo('test//../test');"
      "var_dump($info->getRealPath());"
      "var_dump($info->getPath());"
      "var_dump($info->getPathName());"
      "$info = new SplFileInfo('test/../../external-centos');"
      "var_dump($info->getLinkTarget());"
      "var_dump($info->getRealPath());"
      "var_dump($info->getPath());"
      "var_dump($info->getPathName());");
  MVCR("<?php "
      "$info = new SplFileInfo('does-not-exist-will-fail-on-getLinkTarget');"
      "//readlink('does-not-throw-but-warns');"
      "try{"
      "  $info->getLinkTarget();"
      "}"
      "catch (Exception $e) {"
      "  echo 'Caught exception: ',  $e->getMessage(), \"\\n\";"
      "  return;"
      "}"
      "echo \"failed to throw\\n\";"
      "  return true;"
      "}");
  return true;
}

bool TestCodeRun::TestExtIterator() {
  MVCR("<?php "
      "$files = array();"
      "foreach (new DirectoryIterator('test/') as $file) {"
      "  $files[] = $file;"
      "}"
      "var_dump(count($files));"
      "$dir = new DirectoryIterator(dirname('test/'));"
      "foreach ($dir as $fileinfo) {"
      "  if (!$fileinfo->isDot()) {"
      "    var_dump($fileinfo->getFilename());"
      "  }"
      "}"
      "$iterator = new DirectoryIterator(\"test\");"
      "foreach ($iterator as $fileinfo) {"
      "  if ($fileinfo->isFile()) {"
      "    echo \"BEGIN: \" . $fileinfo->getFilename() . \"\\n\";"
      "    echo $fileinfo->getCTime() . \"\\n\";"
      "    echo $fileinfo->getBasename() . \"\\n\";"
      "    echo $fileinfo->getBasename('.cpp') . \"\\n\";"
      "    echo $fileinfo->getGroup() . \"\\n\";"
      "    echo $fileinfo->getInode() . \"\\n\";"
      "    echo $fileinfo->getMTime() . \"\\n\";"
      "    echo $fileinfo->getOwner() . \"\\n\";"
      "    echo $fileinfo->getPerms() . \"\\n\";"
      "    echo $fileinfo->getSize() . \"\\n\";"
      "    echo $fileinfo->getType() . \"\\n\";"
      "    echo $fileinfo->isDir() . \"\\n\";"
      "    echo $fileinfo->isDot() . \"\\n\";"
      "    echo $fileinfo->isExecutable() . \"\\n\";"
      "    echo $fileinfo->isLink() . \"\\n\";"
      "    echo $fileinfo->isReadable() . \"\\n\";"
      "    echo $fileinfo->isWritable() . \"\\n\";"
      "    echo \"END\" . \"\\n\";"
      "  }"
      "}"
      "$iterator = new RecursiveDirectoryIterator(\"test\");"
      "foreach ($iterator as $fileinfo) {"
      "  if ($fileinfo->isFile()) {"
      "    echo $fileinfo->getFilename() . \"\\n\";"
      "    echo $fileinfo->getCTime() . \"\\n\";"
      "    echo $fileinfo->getBasename() . \"\\n\";"
      "    echo $fileinfo->getBasename('.cpp') . \"\\n\";"
      "    echo $fileinfo->getFilename() . \"\\n\";"
      "    echo $fileinfo->getGroup() . \"\\n\";"
      "    echo $fileinfo->getInode() . \"\\n\";"
      "    echo $fileinfo->getMTime() . \"\\n\";"
      "    echo $fileinfo->getOwner() . \"\\n\";"
      "    echo $fileinfo->getPerms() . \"\\n\";"
      "    echo $fileinfo->getSize() . \"\\n\";"
      "    echo $fileinfo->getType() . \"\\n\";"
      "    echo $fileinfo->isDir() . \"\\n\";"
      "    echo $fileinfo->isExecutable() . \"\\n\";"
      "    echo $fileinfo->isLink() . \"\\n\";"
      "    echo $fileinfo->isReadable() . \"\\n\";"
      "    echo $fileinfo->isWritable() . \"\\n\";"
      "  }"
      "}");
  MVCR("<?php "
      "$dir = new DirectoryIterator('test');"
      "while($dir->valid()) {"
      "  if(!$dir->isDot()) {"
      "    print $dir->current().\"\\n\";"
      "  }"
      "  $dir->next();"
      "}");
  MVCR("<?php "
      "$ite=new RecursiveDirectoryIterator('test/');"
      "$bytestotal=0;"
      "$nbfiles=0;"
      "foreach ($ite as $filename=>$cur) {"
      "  $filesize=$cur->getSize();"
      "  $bytestotal+=$filesize;"
      "  $nbfiles++;"
      "  echo \"$filename => $filesize\\n\";"
      "}"
      "$bytestotal=number_format($bytestotal);"
      "echo \"Total: $nbfiles files, $bytestotal bytes\\n\";");
  MVCR("<?php "
      "$ite=new RecursiveDirectoryIterator('test/');"
      ""
      "$bytestotal=0;"
      "$nbfiles=0;"
      "foreach (new RecursiveIteratorIterator($ite) as $filename=>$cur) {"
      "  $filesize=$cur->getSize();"
      "  $bytestotal+=$filesize;"
      "  $nbfiles++;"
      "  echo \"$filename => $filesize\\n\";"
      "}"
      ""
      "$bytestotal=number_format($bytestotal);"
      "echo \"Total: $nbfiles files, $bytestotal bytes\\n\";");
  MVCR("<?php "
      "$path = \"test/\";"
      "foreach (new RecursiveIteratorIterator("
      "  new RecursiveDirectoryIterator($path,"
      "  RecursiveDirectoryIterator::KEY_AS_PATHNAME),"
      "  RecursiveIteratorIterator::CHILD_FIRST) as $file => $info) {"
      "  if ($info->isDir()) {"
      "    echo $file.\"\\n\";"
      "  }"
      "}");
  MVCR("<?php "
      "$directory = \"test\";"
      "$fileSPLObjects =  new RecursiveIteratorIterator("
      "  new RecursiveDirectoryIterator($directory),"
      "  RecursiveIteratorIterator::SELF_FIRST);"
      "foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {"
      "  print $fullFileName . \" \" .$fileSPLObject->getFilename(). \"\\n\";"
      "}"
      "$fileSPLObjects =  new RecursiveIteratorIterator("
      "  new RecursiveDirectoryIterator($directory),"
      "  RecursiveIteratorIterator::CHILD_FIRST);"
      "foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {"
      "  print $fullFileName . \" \" .$fileSPLObject->getFilename(). \"\\n\";"
      "}"
      "$fileSPLObjects =  new RecursiveIteratorIterator("
      "  new RecursiveDirectoryIterator($directory),"
      "  RecursiveIteratorIterator::LEAVES_ONLY);"
      "foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {"
      "  print $fullFileName . \" \" .$fileSPLObject->getFilename(). \"\\n\";"
      "}"
      "// invalid mode -100"
      "$fileSPLObjects =  new RecursiveIteratorIterator("
      "  new RecursiveDirectoryIterator($directory), -100);"
      "foreach( $fileSPLObjects as $fullFileName => $fileSPLObject ) {"
      "  print $fullFileName . \" \" .$fileSPLObject->getFilename(). \"\\n\";"
      "}");
  MVCR("<?php "
      "function getFiles(&$rdi,$depth=0) {"
      "  if (!is_object($rdi)) return;"
      "  for ($rdi->rewind();$rdi->valid();$rdi->next()) {"
      "    if ($rdi->isDot()) continue;"
      "    if ($rdi->isDir() || $rdi->isFile()) {"
      "      for ($i = 0; $i<=$depth;++$i) echo \" \";"
      "      echo $rdi->current().\"\\n\";"
      "      if ($rdi->hasChildren()) getFiles($rdi->getChildren(),1+$depth);"
      "    }"
      "  }"
      "}"
      "getFiles(new RecursiveDirectoryIterator('test'));");

  MVCR("<?php "
      "try {"
      "  $y = new RecursiveDirectoryIterator('/fake_path');"
      "  $z = new RecursiveIteratorIterator($y);"
      "  $z->next();"
      "} catch (UnexpectedValueException $e) {"
      "}"
      "var_dump('ok');");

  return true;
}

bool TestCodeRun::TestExtSoap() {
  MVCR("<?php "
      "function add($a, $b) { return $a + $b; }"
      "$server = new SoapServer(NULL, array('uri' => 'http://test-uri'));"
      "$str = '<?xml version=\"1.0\" '."
      "       'encoding=\"ISO-8859-1\"?>'."
      "       '<SOAP-ENV:Envelope SOAP-ENV:encodingStyle='."
      "       '\"http://schemas.xmlsoap.org/soap/encoding/\"'."
      "       ' xmlns:SOAP-ENV='."
      "       '\"http://schemas.xmlsoap.org/soap/envelope/\"'."
      "       ' xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"'."
      "       ' xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"'."
      "       ' xmlns:si=\"http://soapinterop.org/xsd\"><SOAP-ENV:Body>'."
      "       '<ns1:Add xmlns:ns1=\"http://testuri.org\">'."
      "       '<x xsi:type=\"xsd:hexBinary\">16</x>'."
      "       '<y xsi:type=\"xsd:hexBinary\">21</y>'."
      "       '</ns1:Add>  </SOAP-ENV:Body></SOAP-ENV:Envelope>';"
      "$server->addFunction('Add');"
      "$server->handle($str);");
  return true;
}

bool TestCodeRun::TestExtCollator() {
  MVCR("<?php "
      "function ut_run($mainFunc) {\n"
      "    $GLOBALS['oo-mode'] = true;\n"
      "    $oo_result = $mainFunc();\n"
      "    $GLOBALS['oo-mode'] = false;\n"
      "    $proc_result = $mainFunc();\n"
      "    if($proc_result !== $oo_result) {\n"
      "      echo \"ERROR: OO- and procedural APIs produce different results!\\n\";\n"
      "      echo \"OO API output:\\n\";\n"
      "      echo str_repeat( '=', 78 ) . \"\\n\";\n"
      "      echo $oo_result;\n"
      "      echo str_repeat( '=', 78 ) . \"\\n\";\n"
      "      echo \"procedural API output:\\n\";\n"
      "      echo str_repeat( '=', 78 ) . \"\\n\";\n"
      "      echo $proc_result;\n"
      "      echo str_repeat( '=', 78 ) . \"\\n\";\n"
      "      return;\n"
      "    }\n"
      "    echo $oo_result;\n"
      "}\n"
      "function dump($val) {\n"
      "    return var_export( $val, true );\n"
      "}\n"
      "function ut_coll_create( $locale )\n"
      "{\n"
      "    return $GLOBALS['oo-mode'] ? Collator::create( $locale ) : collator_create( $locale );\n"
      "}\n"
      "function ut_coll_compare( $coll, $str1, $str2 )\n"
      "{\n"
      "    return $GLOBALS['oo-mode'] ? \n"
      "      $coll->compare( $str1, $str2 ) : collator_compare( $coll, $str1, $str2 );\n"
      "}\n"
      "function ut_coll_sort_with_sort_keys( $coll, &$arr )\n"
      "{\n"
      "    return $GLOBALS['oo-mode'] ? \n"
      "      $coll->sortWithSortKeys( $arr ) : collator_sort_with_sort_keys( $coll, $arr );\n"
      "}\n"
      "function ut_coll_sort( $coll, &$arr, $sort_flag = Collator::SORT_REGULAR )\n"
      "{\n"
      "    return $GLOBALS['oo-mode'] ? \n"
      "      $coll->sort( $arr, $sort_flag ) : collator_sort( $coll, $arr, $sort_flag );\n"
      "}\n"
      "function ut_coll_asort( $coll, &$arr, $sort_flag = Collator::SORT_REGULAR )\n"
      "{\n"
      "    return $GLOBALS['oo-mode'] ? \n"
      "      $coll->asort( $arr, $sort_flag ) : collator_asort( $coll, $arr, $sort_flag );\n"
      "}\n"
      "function ut_coll_get_locale( $coll, $type )\n"
      "{\n"
      "    return $GLOBALS['oo-mode'] ? \n"
      "      $coll->getLocale( $type ) : collator_get_locale( $coll, $type );\n"
      "}\n"
      "function ut_coll_set_strength( $coll, $strength )\n"
      "{\n"
      "    return $GLOBALS['oo-mode'] ? \n"
      "      $coll->setStrength( $strength ) : collator_set_strength( $coll, $strength );\n"
      "}\n"
      "function ut_coll_set_attribute( $coll, $attr, $val )\n"
      "{\n"
      "    return $GLOBALS['oo-mode'] ? \n"
      "      $coll->setAttribute( $attr, $val ) : collator_set_attribute( $coll, $attr, $val );\n"
      "}\n"
      "function ut_coll_set_default( $coll )\n"
      "{\n"
      "    return $GLOBALS['oo-mode'] ? Collator::setDefault( $coll ) : collator_set_default( $coll );\n"
      "}\n"
      "$test_num = 1;\n"
      "function sort_arrays( $locale, $arrays, $sort_flag = Collator::SORT_REGULAR )\n"
      "{\n"
      "    $res_str = '';\n"
      "    $coll = ut_coll_create( $locale );\n"
      "    foreach( $arrays as $array )\n"
      "    {\n"
      "        // Sort array values\n"
      "        $res_val = ut_coll_sort( $coll, $array, $sort_flag );\n"
      "        // Concatenate the sorted array and function result\n"
      "        // with output string.\n"
      "        $res_dump = \"\\n\" . dump( $array ) .\n"
      "                    \"\\n Result: \" . dump( $res_val );\n"
      "        // Preppend test signature to output string\n"
      "        $md5 = md5( $res_dump );\n"
      "        global $test_num;\n"
      "        \n"
      "        $res_str .= \"\\n\\n\".\n"
      "                    \"Test $test_num.$md5:\" .\n"
      "                    $res_dump;\n"
      "        ++$test_num;\n"
      "    }\n"
      "    return $res_str;\n"
      "}\n"
      "function ut_main1()\n"
      "{\n"
      "    global $test_num;\n"
      "    $test_num = 1;\n"
      "    $res_str = '';\n"
      "    // Sort an array in SORT_REGULAR mode using en_US locale.\n"
      "    $test_params = array(\n"
      "        array( 'abc', 'abd', 'aaa' ),\n"
      "        array( 'm'  , '1'  , '_'   ),\n"
      "        array( 'a'  , 'aaa', 'aa'  ),\n"
      "        array( 'ba' , 'b'  , 'ab'  ),\n"
      "        array( 'e'  , 'c'  , 'a'   ),\n"
      "        array( '100', '25' , '36'  ), // test 6\n"
      "        array( 5    , '30' , 2     ),\n"
      "        array( 'd'  , ''   , ' a'  ),\n"
      "        array( 'd ' , 'f ' , ' a'  ),\n"
      "        array( 'a'  , null , '3'   ),\n"
      "        array( 'y'  , 'k'  , 'i' )\n"
      "    );\n"
      "    $res_str .= sort_arrays( 'en_US', $test_params );\n"
      "    $test_params = array(\n"
      "        array( '100', '25' , '36'  ),\n"
      "        array( 5    , '30' , 2     ), // test 13\n"
      "        array( 'd'  , ''   , ' a'  ),\n"
      "        array( 'y'  , 'k'  , 'i' )\n"
      "    );\n"
      "    // Sort in en_US locale with SORT_STRING flag\n"
      "    $res_str .= sort_arrays( 'en_US', $test_params, Collator::SORT_STRING );\n"
      "    // Sort a non-ASCII array using ru_RU locale.\n"
      "    $test_params = array(\n"
      "        array( '\xd0\xb0\xd0\xb1\xd0\xb3', "
      "               '\xd0\xb0\xd0\xb1\xd0\xb2', "
      "               '\xd0\xb0\xd0\xb0\xd0\xb0', "
      "               '\xd0\xb0\xd0\xb1\xd0\xb2' ),\n"
      "        array( '\xd0\xb0\xd0\xb0', '\xd0\xb0\xd0\xb0\xd0\xb0',"
      "               '\xd0\xb0' )\n"
      "    );\n"
      "    $res_str .= sort_arrays( 'ru_RU', $test_params );\n"
      "    // Sort an array using Lithuanian locale.\n"
      "    $test_params = array(\n"
      "        array( 'y'  , 'k'  , 'i' )\n"
      "    );\n"
      "    $res_str .= sort_arrays( 'lt_LT', $test_params );\n"
      "    return $res_str;\n"
      "}\n"
      "ut_run('ut_main1');\n"
      "function ut_main2() {\n"
      "  $obj = ut_coll_create('en_US');\n"
      "  $arr0 = array( 100, 25, 36, '30.2', '30.12' ); // test 6\n"
      "  $arr1 = array( '100', '25', '36'  ); // test 6\n"
      "  $arr2 = array( 11, 5, '2', 64, 17, '30', 10, 2, '54' );\n"
      "  // strcmp 17 and 30, ret = 1\n"
      "  // Comparing values 17 and 30, ret = 1\n"
      "  $arr3 = array( 11, 5, 2, 64, 17, 30, 10, 2, 54 );\n"
      "  $arrA = $arr0;\n"
      "  $arrB = $arr0;\n"
      "  $arrC = $arr0;\n"
      "  ut_coll_sort($obj, $arrA, Collator::SORT_REGULAR);\n"
      "  ut_coll_sort($obj, $arrB, Collator::SORT_STRING);\n"
      "  ut_coll_sort($obj, $arrC, Collator::SORT_NUMERIC);\n"
      "  var_dump($arrA, $arrB, $arrC);\n"
      "  $arrA = $arr1;\n"
      "  $arrB = $arr1;\n"
      "  $arrC = $arr1;\n"
      "  ut_coll_sort($obj, $arrA, Collator::SORT_REGULAR);\n"
      "  ut_coll_sort($obj, $arrB, Collator::SORT_STRING);\n"
      "  ut_coll_sort($obj, $arrC, Collator::SORT_NUMERIC);\n"
      "  var_dump($arrA, $arrB, $arrC);\n"
      "  $arrA = $arr2;\n"
      "  $arrB = $arr2;\n"
      "  $arrC = $arr2;\n"
      "  ut_coll_sort($obj, $arrA, Collator::SORT_REGULAR);\n"
      "  ut_coll_sort($obj, $arrB, Collator::SORT_STRING);\n"
      "  ut_coll_sort($obj, $arrC, Collator::SORT_NUMERIC);\n"
      "  var_dump($arrA, $arrB, $arrC);\n"
      "  $arrA = $arr3;\n"
      "  $arrB = $arr3;\n"
      "  $arrC = $arr3;\n"
      "  ut_coll_sort($obj, $arrA, Collator::SORT_REGULAR);\n"
      "  ut_coll_sort($obj, $arrB, Collator::SORT_STRING);\n"
      "  ut_coll_sort($obj, $arrC, Collator::SORT_NUMERIC);\n"
      "  var_dump($arrA, $arrB, $arrC);\n"
      "}\n"
      "ut_run('ut_main2');\n"
      "function ut_main3()\n"
      "{\n"
      "    $res_str = '';\n"
      "    $locales = array(\n"
      "        'EN-US-ODESSA',\n"
      "        'UK_UA_ODESSA',\n"
      "        'uk-ua_CALIFORNIA@currency=;currency=GRN',\n"
      "        '',\n"
      "        'root',\n"
      "        'uk@currency=EURO'\n"
      "    );\n"
      "    foreach( $locales as $locale )\n"
      "    {\n"
      "        // Create Collator with the current locale.\n"
      "        $coll = ut_coll_create( $locale );\n"
      "        if( !is_object($coll) )\n"
      "        {\n"
      "            $res_str .= \"Error creating collator with '$locale' locale: \" .\n"
      "                 intl_get_error_message() . \"\\n\";\n"
      "            continue;\n"
      "        }\n"
      "        // Get the requested, valid and actual locales.\n"
      "        $vloc = ut_coll_get_locale( $coll, Locale::VALID_LOCALE );\n"
      "        // Show them.\n"
      "        $res_str .= \"Locale: '$locale'\\n\" .\n"
      "            \"  ULOC_VALID_LOCALE     = '$vloc'\\n\";\n"
      "    }\n"
      "    return $res_str;\n"
      "}\n"
      "ut_run('ut_main3');\n"
      "function test_COW( $locale, $test_array )\n"
      "{\n"
      "    $res_str = '';\n"
      "    $coll = ut_coll_create( $locale );\n"
      "    // Create two copies of the given array.\n"
      "    $copy1 = $test_array;\n"
      "    $copy2 = $test_array;\n"
      "    // Sort given array and the first copy of it.\n"
      "    ut_coll_sort( $coll, $test_array );\n"
      "    ut_coll_sort( $coll, $copy1      );\n"
      "    // Return contents of all the arrays.\n"
      "    // The second copy should remain unsorted.\n"
      "    $res_str .= dump( $test_array ) . \"\\n\";\n"
      "    $res_str .= dump( $copy1 ) . \"\\n\";\n"
      "    $res_str .= dump( $copy2 ) . \"\\n\";\n"
      "    return $res_str;\n"
      "}\n"
      "function ut_main4()\n"
      "{\n"
      "    $res_str = '';\n"
      "    $a1 = array( 'b', 'a', 'c' );\n"
      "    $a2 = array( '\xd0\xb1', '\xd0\xb0', '\xd0\xb2' );\n"
      "    $res_str .= test_COW( 'en_US', $a1 );\n"
      "    $res_str .= test_COW( 'ru_RU', $a2 );\n"
      "    return $res_str;\n"
      "}\n"
      "ut_run('ut_main4');\n"
      "function cmp_array( &$coll, $a )\n"
      "{\n"
      "    $res = '';\n"
      "    $prev = null;\n"
      "    foreach( $a as $i )\n"
      "    {\n"
      "        if( is_null( $prev ) )\n"
      "            $res .= \"$i\";\n"
      "        else\n"
      "        {\n"
      "            $eqrc = ut_coll_compare( $coll, $prev, $i );\n"
      "            $eq = $eqrc < 0 ? \"<\" : ( $eqrc > 0 ? \">\" : \"=\" );\n"
      "            $res .= \" $eq $i\";\n"
      "        }\n"
      "        $prev = $i;\n"
      "    }\n"
      "    $res .= \"\\n\";\n"
      "    return $res;\n"
      "}\n"
      "function check_alternate_handling( &$coll )\n"
      "{\n"
      "    $res = '';\n"
      "    ut_coll_set_strength( $coll, Collator::TERTIARY );\n"
      "    ut_coll_set_attribute( $coll, Collator::ALTERNATE_HANDLING, Collator::NON_IGNORABLE );\n"
      "    $res .= cmp_array( $coll, array( 'di Silva', 'Di Silva', 'diSilva', 'U.S.A.', 'USA' ) );\n"
      "    ut_coll_set_attribute( $coll, Collator::ALTERNATE_HANDLING, Collator::SHIFTED );\n"
      "    $res .= cmp_array( $coll, array( 'di Silva', 'diSilva', 'Di Silva', 'U.S.A.', 'USA' ) );\n"
      "    ut_coll_set_strength( $coll, Collator::QUATERNARY );\n"
      "    $res .= cmp_array( $coll, array( 'di Silva', 'diSilva', 'Di Silva', 'U.S.A.', 'USA' ) );\n"
      "    $res .= \"\\n\";\n"
      "    return $res;\n"
      "}\n"
      "function ut_main5()\n"
      "{\n"
      "    $coll = ut_coll_create( 'en_US' );\n"
      "    return\n"
      "        check_alternate_handling( $coll );\n"
      "}\n"
      "ut_run('ut_main5');\n"
      "function sort_arrays_with_sort_keys( $locale, $arrays )\n"
      "{\n"
      "    $res_str = '';\n"
      "    $coll = ut_coll_create( $locale );\n"
      "    foreach( $arrays as $array )\n"
      "    {\n"
      "        // Sort array values\n"
      "        $res_val = ut_coll_sort_with_sort_keys( $coll, $array );\n"
      "        // Concatenate the sorted array and function result\n"
      "        // with output string.\n"
      "        $res_dump = \"\\n\" . dump( $array ) .\n"
      "                    \"\\n Result: \" . dump( $res_val );\n"
      "        \n"
      "        \n"
      "        // Preppend test signature to output string\n"
      "        $md5 = md5( $res_dump );\n"
      "        global $test_num;\n"
      "        $res_str .= \"\\n\\n\".\n"
      "                    \"Test $test_num.$md5:\" .\n"
      "                    $res_dump;\n"
      "        ++$test_num;\n"
      "    }\n"
      "    return $res_str;\n"
      "}\n"
      "function ut_main6()\n"
      "{\n"
      "    global $test_num;\n"
      "    $test_num = 1;\n"
      "    $res_str = '';\n"
      "    // Sort an array in SORT_REGULAR mode using en_US locale.\n"
      "    $test_params = array(\n"
      "        array( 'abc', 'abd', 'aaa' ),\n"
      "        array( 'm'  , '1'  , '_'   ),\n"
      "        array( 'a'  , 'aaa', 'aa'  ),\n"
      "        array( 'ba' , 'b'  , 'ab'  ),\n"
      "        array( 'e'  , 'c'  , 'a'   ),\n"
      "        array( 'd'  , ''   , ' a'  ),\n"
      "        array( 'd ' , 'f ' , ' a'  ),\n"
      "        array( 'a'  , null , '3'   ),\n"
      "        array( 'y'  , 'i'  , 'k'   )\n"
      "    );\n"
      "    $res_str .= sort_arrays_with_sort_keys( 'en_US', $test_params );\n"
      "    // Sort a non-ASCII array using ru_RU locale.\n"
      "    $test_params = array(\n"
      "        array( '\xd0\xb0\xd0\xb1\xd0\xb3', "
      "               '\xd0\xb0\xd0\xb1\xd0\xb2', "
      "               '\xd0\xb0\xd0\xb0\xd0\xb0', "
      "               '\xd0\xb0\xd0\xb1\xd0\xb2' ),\n"
      "        array( '\xd0\xb0\xd0\xb0', '\xd0\xb0\xd0\xb0\xd0\xb0',"
      "               '\xd0\xb0' )\n"
      "    );\n"
      "    $res_str .= sort_arrays_with_sort_keys( 'ru_RU', $test_params );\n"
      "    // Array with data for sorting.\n"
      "    $test_params = array(\n"
      "        array( 'y'  , 'i'  , 'k'   )\n"
      "    );\n"
      "    // Sort an array using Lithuanian locale.\n"
      "    $res_str .= sort_arrays_with_sort_keys( 'lt_LT', $test_params );\n"
      "    return $res_str . \"\\n\";\n"
      "}\n"
      "ut_run('ut_main6');\n"
  );

  return true;
}

bool TestCodeRun::TestExtSocket() {
  MVCR("<?php "
      "$r = socket_create(AF_INET, SOCK_RAW, 0);"
      "var_dump(socket_last_error());");

  return true;
}

bool TestCodeRun::TestFiber() {
  // test parameter and return value passing
  MVCRO("<?php "
        "function fiber($a) { var_dump($a); return 'fiber';}"
        "$f = call_user_func_async('fiber', array(123, 456));"
        "var_dump(end_user_func_async($f));",

        "array(2) {\n"
        "  [0]=>\n"
        "  int(123)\n"
        "  [1]=>\n"
        "  int(456)\n"
        "}\n"
        "string(5) \"fiber\"\n"
       );

  // test references
  MVCRO("<?php "
        "function fiber(&$a) { $a = 123;}"
        "$b = 456;"
        "call_user_func('fiber', $b);"
        "var_dump($b);",

        "int(123)\n"
       );
  MVCRO("<?php "
        "function fiber(&$a) { $a = 123;}"
        "$b = 456;"
        "end_user_func_async(call_user_func_async('fiber', $b));"
        "var_dump($b);",

        "int(123)\n"
       );
  MVCRO("<?php "
        "function fiber($a) { $a = 123;}"
        "$b = 456;"
        "call_user_func('fiber', $b);"
        "var_dump($b);",

        "int(456)\n"
       );
  MVCRO("<?php "
        "function fiber($a) { $a = 123;}"
        "$b = 456;"
        "end_user_func_async(call_user_func_async('fiber', $b));"
        "var_dump($b);",

        "int(456)\n"
       );

  // test objects
  MVCRO("<?php "
        "class A { public $data = 456;}"
        "function fiber($a) { $a->data = 123;}"
        "$obj = new A();"
        "call_user_func('fiber', $obj);"
        "var_dump($obj->data);",

        "int(123)\n"
       );
  MVCRO("<?php "
        "class A { public $data = 456;}"
        "function fiber($a) { $a->data = 123;}"
        "$obj = new A();"
        "end_user_func_async(call_user_func_async('fiber', $obj));"
        "var_dump($obj->data);",

        "int(123)\n"
       );
  MVCRO("<?php "
        "class A { private $data = 456; "
        "function foo() { var_dump($this->data);}"
        "function bar() { $this->data = 123;}}"
        "function fiber($a) { $a->bar();}"
        "$obj = new A();"
        "end_user_func_async(call_user_func_async('fiber', $obj));"
        "$obj->foo();",

        "int(123)\n"
       );

  MVCRO("<?php "
        "class A { private $data = 456; "
        "function foo() { var_dump($this->data);}"
        "function bar() { $this->data = 123;}}"
        "function fiber() { $a = new A(); $a->bar(); return $a;}"
        "$obj = end_user_func_async(call_user_func_async('fiber'));"
        "$obj->foo();",

        "int(123)\n"
       );

  MVCRO("<?php "
        "class A { "
        "function foo() { var_dump($this->data);}"
        "function bar() { $this->data = 123;}}"
        "function fiber() { $a = new A(); $a->bar(); return $a;}"
        "$obj = end_user_func_async(call_user_func_async('fiber'));"
        "$obj->foo();",

        "int(123)\n"
       );

  // test arrays of references and objects
  MVCRO("<?php "
        "class A { public $data = 456;}"
        "function fiber($a) { $a[0]->data = 123; $a[1] = 234;}"
        "$obj = new A();"
        "$b = 456;"
        "$arr = array($obj, &$b);"
        "call_user_func('fiber', $arr);"
        "var_dump($obj->data);"
        "var_dump($b);",

        "int(123)\n"
        "int(234)\n"
       );
  MVCRO("<?php "
        "class A { public $data = 456;}"
        "function fiber($a) { $a[0]->data = 123; $a[1] = 234;}"
        "$obj = new A();"
        "$b = 456;"
        "$arr = array($obj, &$b);"
        "end_user_func_async(call_user_func_async('fiber', $arr));"
        "var_dump($obj->data);"
        "var_dump($b);",

        "int(123)\n"
        "int(234)\n"
       );

  // test global states
  MVCRO("<?php "
        "function fiber() { global $foo; $foo = 456;}"
        "$foo = 123;"
        "end_user_func_async(call_user_func_async('fiber'),"
        " GLOBAL_STATE_OVERWRITE);"
        "var_dump($foo);",

        "int(456)\n"
       );
  MVCRO("<?php "
        "function fiber() { global $foo; $foo = 456;}"
        "$foo = 123;"
        "end_user_func_async(call_user_func_async('fiber'),"
        " GLOBAL_STATE_IGNORE);"
        "var_dump($foo);",

        "int(123)\n"
       );
  MVCRO("<?php "
        "function fiber() { global $foo; $foo = 456;}"
        "$foo = 123;"
        "end_user_func_async(call_user_func_async('fiber'),"
        " GLOBAL_STATE_OVERWRITE,"
        " array(GLOBAL_SYMBOL_GLOBAL_VARIABLE =>"
        "       array('foo' => GLOBAL_STATE_IGNORE)));"
        "var_dump($foo);",

        "int(123)\n"
       );

  // test dynamic globals
  MVCRO("<?php "
        "function fiber() { $a = 'foo'; global $$a; $$a = 456;}"
        "$a = 'foo'; $$a = 123;"
        "end_user_func_async(call_user_func_async('fiber'),"
        " GLOBAL_STATE_OVERWRITE);"
        "var_dump($$a);",

        "int(456)\n"
       );

  // test static variables
  MVCRO("<?php "
        "function fiber() { static $a = 123; var_dump(++$a); }"
        "end_user_func_async(call_user_func_async('fiber'),"
        " GLOBAL_STATE_OVERWRITE);"
        "fiber();",

        "int(124)\n"
        "int(125)\n"
       );
  MVCRO("<?php "
    "class CS {"
    "  public static $x = 10;"
    "}"
    "function f() {"
    "  CS::$x = 20;"
    "}"
    "end_user_func_async(call_user_func_async('f'), GLOBAL_STATE_OVERWRITE);"
    "var_dump(CS::$x);",

    "int(20)\n");

  // test execution context
  MVCRO("<?php "
        "function fiber() { date_default_timezone_set('America/New_York');}"
        "$task = call_user_func_async('fiber');"
        "end_user_func_async($task);"
        "var_dump(date_default_timezone_get());",

        "string(16) \"America/New_York\"\n"
       );
  // test definitions
  MVCRO("<?php "
        "function f() {"
        "  class zz {}"
        "  function gg() { return 1; }"
        "  define('con', 123);"
        "}"
        "end_user_func_async(call_user_func_async('f'), "
        "                    GLOBAL_STATE_OVERWRITE);"
        "var_dump(new zz);"
        "var_dump(gg());"
        "var_dump(con);",

        "object(zz)#1 (0) {\n"
        "}\n"
        "int(1)\n"
        "int(123)\n"
        );
  MVCRO("<?php "
        "function fiber($a) {var_dump($a[0][1]);}"
        "$arr = array(array(1,2),3);"
        "apc_store('key', $arr);"
        "$arr2 = apc_fetch('key');"
        "$f = call_user_func_async('fiber', $arr2);"
        "end_user_func_async($f);"
        "var_dump($arr2[0][1]);",
        "int(2)\n"
        "int(2)\n"
        );

  // recursive objects
  MVCRO("<?php\n"
        "class A { private $v; function set($a) { $this->v = $a; } }\n"
        "$o1 = new A;\n"
        "$o2 = new A;\n"
        "$o1->set($o2); $o2->set($o1);\n"
        "function f($a) { return $a; }\n"
        "$h = call_user_func_async('f', $o1);\n"
        "$ret = end_user_func_async($h);\n"
        "echo \"Success!\n\";",
        "Success!\n");

  // params that are not by-ref
  MVCRO("<?php\n"
        "function run($a) { var_dump($a); }\n"
        "$a = 0;\n"
        "$h = call_user_func_async('run', $a);\n"
        "$a = 1;\n"
        "end_user_func_async($h);\n"
        "var_dump($a);\n",
        "int(0)\n"
        "int(1)\n");

  return true;
}

bool TestCodeRun::TestAPC() {
  MVCR("<?php "
       "$a = array();\n"
       "$a[] =& $a;\n"
       "print_r($a);\n"
       "apc_store('table', $a);\n"
      );

  // PHP doesn't have APC for CLI mode, so have to use MVCRO
  MVCRO("<?php "
        "$a = array(1);\n"
        "$a[] =& $a[0];\n"
        "$a[0] = 2;\n"
        "print_r($a);\n"
        "\n"
        "apc_store('table', $a);\n"
        "$b = apc_fetch('table', $b);\n"
        "print_r($b);\n"
        "$b[0] = 3;\n"
        "print_r($b);\n",
        "Array\n"
        "(\n"
        "    [0] => 2\n"
        "    [1] => 2\n"
        ")\n"
        "Array\n"
        "(\n"
        "    [0] => 2\n"
        "    [1] => 2\n"
        ")\n"
        "Array\n"
        "(\n"
        "    [0] => 3\n"
        "    [1] => 3\n"
        ")\n"
      );

  MVCRO("<?php\n"
        "class A { var $v = 10; function f() { $this->v = 100; } }\n"
        "$a = array(array(1, 2, 3), new A());\n"
        "apc_store('0', $a);\n"
        "$b = apc_fetch(0);\n"
        "var_dump($b[1]->v);\n"
        "$b[1]->f();\n"          // SharedMap::get() needs caching
        "var_dump($b[1]->v);\n"
        "$b[2] = 1;\n"           // SharedMap::escalate() needs caching
        "var_dump($b[1]->v);\n",
        "int(10)\n"
        "int(100)\n"
        "int(100)\n"
      );

  MVCRO("<?php\n"
        "class A { private $b = 10; }\n"
        "class B extends A { private $b = 100; }\n"
        "apc_store('key', new B());\n"
        "var_dump(apc_fetch('key'));\n",
        "object(B)#1 (2) {\n"
        "  [\"b:private\"]=>\n"
        "  int(100)\n"
        "  [\"b:private\"]=>\n"
        "  int(10)\n"
        "}\n"
      );

  MVCRO("<?php\n"
        "class A { var $a, $b; }\n"
        "$a = new A(); $a->a = 5; $a->b = &$a->a;\n"
        "apc_store('key', $a);\n"
        "var_dump(apc_fetch('key'));\n",
        "object(A)#2 (2) {\n"
        "  [\"a\"]=>\n"
        "  &int(5)\n"
        "  [\"b\"]=>\n"
        "  &int(5)\n"
        "}\n"
      );
  MVCRO("<?php\n"
      "class a {"
      "  protected $foo = 10;"
      "}"
      "$x = new a;"
      "apc_store('x', array($x));"
      "$x = apc_fetch('x');"
      "var_dump($x[0]);",
      "object(a)#1 (1) {\n"
      "  [\"foo:protected\"]=>\n"
      "  int(10)\n"
      "}\n"
      );

  // objects in an apc array can be changed without escalating the array
  MVCRO("<?php\n"
        "class A { var $i = 10; }\n"
        "$a = array(new A);\n"
        "apc_store('key1', $a);\n"
        "$b = apc_fetch('key1');\n"
        "$c = $b[0];\n"
        "$c->i = 100;\n"
        "apc_store('key2', $b);\n"
        "$t = apc_fetch('key2');\n"
        "var_dump($t[0]->i);\n",
        "int(100)\n");

  // Serializable object in APC
  MVCRO("<?php\n"
        "class A implements Serializable {\n"
        "  var $a = 123;\n"
        "  function serialize() { return serialize($this->a); }\n"
        "  function unserialize($s) { $this->a = unserialize($s); }\n"
        "}\n"
        "$o = new A;\n"
        "apc_store('key', $o);\n"
        "$r = apc_fetch('key');\n"
        "var_dump($r);\n",
        "object(A)#2 (1) {\n"
        "  [\"a\"]=>\n"
        "  int(123)\n"
        "}\n");

  return true;
}

bool TestCodeRun::TestInlining() {
  bool save = Option::AutoInline;
  Option::AutoInline = true;

  MVCR("<?php "
       "function id($x) {"
       " return $x;"
       " }"
       "class X {"
       "  public function f() { return 'hello'; }"
       "}"
       "function test($a, $b) {"
       "  return $a ? $b : id(new X)->f();"
       "}"
       "var_dump(test());");

  MVCR("<?php "
       "function foo($e='e') {"
       "  return '<a name=\"'.$e.'\" id=\"'.$e.'\"></a>';"
       "}"
       "function test() {"
       "  echo foo();"
       "}"
       "test();");

  MVCR("<?php "
       "function foo($e, $m) {"
       "  $_REQUEST['_foo'] = $e;"
       "  $_REQUEST['_bar'] = $m;"
       "  return $e;"
       "}"
       "function test($x) {"
       "  return foo('a', $x);"
       "}"
       "var_dump(test('b'));");

  MVCR("<?php "
       "function h() { class X{}; }"
       "function f($a, $b, $c) { return h(); }"
       "function g($a, $b, $c) {"
       "  return f($a++, $b++ + $a++, $c);"
       "}");

  MVCR("<?php "
       "function f($name, $unique_id=false, $id=null) {"
       "  $id = $id ? $id : ($unique_id ? uniqid($name) : $name);"
       "  return $id;"
       "  }"
       "function test($a, $b, $c) {"
       "  return f($name = 'status', $unique_id = true, $id = 'status_active');"
       "  }"
       "var_dump(test(1,2,3));");

  MVCR("<?php "
      "function g($a) { function t(){}; return $a ? array(1,2,3) : 'foo'; }"
      "function f($a) { return g($a); }"
      "function test($a) {"
      "  return reset((f($a)));"
      "  }"
      "var_dump(test(1));"
      "function &h(&$a) { return $a['foo']; }"
      "function i($a) { $x = &h($a); $x = 'hello'; return $a; }"
      "var_dump(i(false));");

  MVCR("<?php "
       "function bar($name) {"
       "  $name = trim($name);"
       "  var_dump($name);"
       "}"
       "function f($x, $y) { if ($x) return $x; return $y; }"
       "function foo() {"
       "  $name = 'ab.' . f('x', 'y');"
       "  bar($name);"
       "  bar($name);"
       "}"
       "foo();");

  MVCR("<?php "
       "function id($x) { return $x; }"
       "class B { function __construct($x) { $this->x = $x; } }"
       "class X extends B {"
       "  function __construct() { parent::__construct(array()); }"
       "  function foo() { echo \"foo\n\"; }"
       "}"
       "function bar($x=0) { if ($x) return 1; return ''; }"
       "function test($foo) {"
       "  id(new X(bar()))->foo();"
       "  id(new $foo(bar()))->foo();"
       "}"
       "test('X');");

  MVCR("<?php "
       "function id($a) { return $a; }"
       "class X {}"
       "if (0) {"
       "  class X {}"
       "}"
       "class Y extends X { function t() {} }"
       "function test() {"
       "  id(new Y)->t();"
       "}");

  Option::AutoInline = save;
  return true;
}

bool TestCodeRun::TestVariableClassName() {
  MVCRO(
    "<?php\n"
    "class A {\n"
    "  const C = 123;\n"
    "  static public $foo = 456;\n"
    "  public function bar() {\n"
    "    return 789;\n"
    "  }\n"
    "}\n"
    "$cls = 'a';\n"
    "\n"
    "var_dump($cls::C); // ClassConstant\n"
    "\n"
    "var_dump($cls::$foo); // StaticMember\n"
    "$cls::$foo = 'test';\n"
    "var_dump($cls::$foo); // l-value\n"
    "\n"
    "var_dump($cls::bar()); // SimpleFunctionCall\n"
    "\n"
    "$func = 'bar';\n"
    "var_dump($cls::$func()); // DynamicFunctionCall\n",

    "int(123)\n"
    "int(456)\n"
    "string(4) \"test\"\n"
    "int(789)\n"
    "int(789)\n"
  );

  MVCRO(
    "<?php\n"
    "class B {\n"
    "  function f4($arguments) {\n"
    "    var_dump($arguments);\n"
    "  }\n"
    "}\n"
    "class G extends B {\n"
    "  function f4($a) {\n"
    "    $b='B';\n"
    "    $b::f4(5); // __call\n"
    "  }\n"
    "}\n"
    "$g = new G(5);\n"
    "$g->f4(3);\n",

    "int(5)\n"
  );

  MVCRO("<?php\n"
        "class B {\n"
        "  function __call($name, $arguments) {\n"
        "    echo \"Calling B object method '$name' \" . "
        "implode(', ', $arguments). \"\n\";\n"
        "  }\n"
        "}\n"
        "class G extends B {\n"
        "  function __call($name, $arguments) {\n"
        "    echo \"Calling G object method '$name' \" . "
        "implode(', ', $arguments). \"\n\";\n"
        "  }\n"
        "  function f4missing($a) {\n"
        "    $b=\"B\";\n"
        "    echo \"Calling G object method 'f4missing' 5 == \", "
        "$b::f4missing(5);\n"
        "  }\n"
        "}\n"
        "\n"
        "$g = new G();\n"
        "$g->f4missing(3);\n"
        "echo \"finish\n\";\n",

        "Calling G object method 'f4missing' 5 == "
        "Calling G object method 'f4missing' 5\nfinish\n"
       );

  MVCRO("<?php\n"
        "function func() { return 'B';}\n"
        "class B {\n"
        "  function foo() { var_dump(__CLASS__);}\n"
        "  function f4missing() { $this->foo();}\n"
        "}\n"
        "class G extends B {\n"
        "  function foo() { var_dump(__CLASS__);}\n"
        "  function f4missing() { $b = func(); $b::f4missing();}\n"
        "}\n"
        "$g = new G; $g->f4missing();\n",

        "string(1) \"G\"\n"
       );

  return true;
}

bool TestCodeRun::TestLateStaticBinding() {
  MVCRO(
    "<?php\n"
    "class B {\n"
    "  public static $a = 100;\n"
    "  static function f() {\n"
    "    var_dump(static::$a);\n"
    "  }\n"
    "}\n"
    "class C extends B {\n"
    "  public static $a = 1000;\n"
    "}\n"
    "call_user_func(array('C', 'f'));\n",
    "int(1000)\n"
  );

  MVCRO(
    "<?php\n"
    "class A {\n"
    "    const NAME = 'A';\n"
    "    public static function test() {\n"
    "        $args = func_get_args();\n"
    "        echo static::NAME, \" \".join(',', $args).\" \\n\";\n"
    "    }\n"
    "}\n"
    "class B extends A {\n"
    "    const NAME = 'B';\n"
    "    public static function test() {\n"
    "        echo self::NAME, \"\\n\";\n"
    "        forward_static_call(array('A', 'test'), 'more', 'args');\n"
    "        forward_static_call( 'test', 'other', 'args');\n"
    "    }\n"
    "}\n"
    "B::test('foo');\n"
    "function test() {\n"
    "    $args = func_get_args();\n"
    "    echo \"C \".join(',', $args).\" \\n\";\n"
    "}\n",
    "B\n"
    "B more,args \n"
    "C other,args \n"
  );

  MVCRO(
    "<?php\n"
    "class X {\n"
    "  function f() {\n"
    "    $y = new Y;\n"
    "    $y->foo();\n"
    "    static::g();\n"
    "    $y->foo();\n"
    "    self::g();\n"
    "    Y::foo() && static::g();\n"
    "  }\n"
    "  static function g() { var_dump(__CLASS__); }\n"
    "}\n"
    "class Y extends X {\n"
    "  static function g() { var_dump(__CLASS__); }\n"
    "  static function foo() { return true; }\n"
    "}\n"
    "function test() {\n"
    "  $x = new X;\n"
    "  $y = new Y;\n"
    "  $x->f();\n"
    "  $y->f();\n"
    "}\n"
    "test();\n",
    "string(1) \"X\"\n"
    "string(1) \"X\"\n"
    "string(1) \"X\"\n"
    "string(1) \"Y\"\n"
    "string(1) \"X\"\n"
    "string(1) \"Y\"\n"
  );

  MVCRO(
    "<?php\n"
    "class A {\n"
    "  static public function foo() {\n"
    "    static::bar();\n"
    "  }\n"
    "  public function bar() {\n"
    "    var_dump(__CLASS__);\n"
    "  }\n"
    "  public function foo2() {\n"
    "    B::foo();    // B always changes 'static'\n"
    "    self::foo(); // 'self' doesn't change 'static'\n"
    "  }\n"
    "}\n"
    "class B extends A {\n"
    "  public function bar() {\n"
    "    var_dump(__CLASS__);\n"
    "  }\n"
    "  public function foo3() {\n"
    "    $this->foo();  // $this changes 'static'\n"
    "    parent::foo(); // 'parent' doesn't change 'static'\n"
    "  }\n"
    "}\n"
    "\n"
    "$a = new A();\n"
    "$b = new B();\n"
    "\n"
    "B::foo();   // B\n"
    "$b->foo();  // B\n"
    "\n"
    "$b->foo2(); // BB\n"
    "$b->foo3(); // BB\n"
    "\n"
    "A::foo();   // A\n"
    "$a->foo();  // A\n"
    "\n"
    "$a->foo2(); // BA\n",

    "string(1) \"B\"\n"
    "string(1) \"B\"\n"
    "string(1) \"B\"\n"
    "string(1) \"B\"\n"
    "string(1) \"B\"\n"
    "string(1) \"B\"\n"
    "string(1) \"A\"\n"
    "string(1) \"A\"\n"
    "string(1) \"B\"\n"
    "string(1) \"A\"\n"
  );

  MVCR("<?php "
       "class X {"
       "  static function foo() { return false; }"
       "  static function bar() { return 5.5; }"
       "  static function baz() { return time(); }"
       "  }"
       "var_dump(X::foo());"
       "var_dump(X::bar());"
       "var_dump(gettype(X::baz()));");

  MVCRO("<?php\n"
        "class X {\n"
        "  static function foo() { echo \"X::foo\\n\"; }\n"
        "  function bar() { static::foo(); }\n"
        "}\n"
        "class Y extends X {\n"
        "  static function foo() { echo \"Y::foo\\n\"; }\n"
        "  function baz() { X::bar(); }\n"
        "}\n"
        "$y = new Y;\n"
        "$y->baz();\n"
        "Y::baz();\n",

        "Y::foo\nX::foo\n");

  MVCRO("<?php\n"
        "function f() { return true; }\n"
        "if (f()) {\n"
        "  class A {\n"
        "    static $a = 'A';\n"
        "    static function f() { echo static::$a; }\n"
        "    function g() { $this->f(); }\n"
        "  }\n"
        "} else {\n"
        "  class A { }\n"
        "}\n"
        "class B extends A { static $a = 'B'; }\n"
        "$b = new B;\n"
        "$b->g();\n",

        "B");

  // instanceof static and new static
  MVCRO("<?php\n"
        "class A {\n"
        "  static function f() { return new static; }\n"
        "  static function g($o) { return $o instanceof static; }\n"
        "}\n"
        "class B extends A { }\n"
        "var_dump(A::g(A::f()));\n"
        "var_dump(A::g(B::f()));\n"
        "var_dump(B::g(A::f()));\n"
        "var_dump(B::g(B::f()));\n",

        "bool(true)\n"
        "bool(true)\n"
        "bool(false)\n"
        "bool(true)\n");

  MVCRO("<?php\n"
        "class TestA {\n"
        "protected static function doSomething() {\n"
        "echo \"TestA::doSomething\\n\";\n"
        "}\n"
        "protected static function test() {\n"
        "static::doSomething();\n"
        "}\n"
        "public static function nativeTest($obj) {\n"
        "$obj->bar();\n"
        "self::test();\n"
        "}\n"
        "}\n"
        "class Foo {\n"
        "public function bar() {}\n"
        "}\n"
        "$obj = new Foo();\n"
        "TestA::nativeTest($obj);\n",

        "TestA::doSomething\n");

  return true;
}

bool TestCodeRun::TestCallStatic() {
  MVCRO("<?php\n"
        "class c2 {\n"
        "  public static function __callStatic($func, $args) {\n"
        "    echo \"c2::__callStatic\n\";\n"
        "  }\n"
        "}\n"
        "class d2 extends c2 {\n"
        "  public function __call($func, $args) {\n"
        "    echo \"d2::__call\n\";\n"
        "  }\n"
        "  public function test1a() {\n"
        "    c2::foo();\n"
        "  }\n"
        "}\n"
        "$obj = new d2;\n"
        "$obj->test1a();\n",

        "d2::__call\n"
        // "c2::__callStatic\n"  <--- PHP 5.3 returns this
       );

  MVCRO("<?php\n"
        "class b2 { }\n"
        "class c2 extends b2 {\n"
        "  public static function __callStatic($func, $args) {\n"
        "    echo \"c2::__callStatic\n\"; }\n"
        "}\n"
        "class d2 extends c2 {\n"
        "  public function __call($func, $args) {\n"
        "    echo \"d2::__call\n\";\n"
        "  }\n"
        "  public function test1a() {\n"
        "    b2::foo();\n"
        "  }\n"
        "}\n"
        "set_error_handler('h');\n"
        "function h() { var_dump('errored');}"
        "$obj = new d2;\n"
        "$obj->test1a();\n"
        "var_dump('end');\n",

        "d2::__call\n"
        "string(3) \"end\"\n"

        // "string(7) \"errored\"\n"  <--- PHP 5.3 returns this
       );

  MVCRO("<?php\n"
        "class b2 {}\n"
        "class c2 extends b2 {\n"
        "  public function __call($func, $args) {\n"
        "    echo \"c2::__call\n\";\n"
        "  }\n"
        "  public function test1a() {\n"
        "    b2::foo();\n"
        "  }\n"
        "}\n"
        "set_error_handler('h');\n"
        "function h() { var_dump('errored');}"
        "$obj = new c2;\n"
        "$obj->test1a();\n"
        "var_dump('end');\n",

        "c2::__call\n"
        "string(3) \"end\"\n"

        // "string(7) \"errored\"\n"  <--- PHP 5.3 returns this
       );

  MVCRO("<?php\n"
        "class c3 {\n"
        "  public function __call($func, $args) {\n"
        "    echo \"c3::__call\n\";\n"
        "  }\n"
        "  public static function __callStatic($func, $args) {\n"
        "    echo \"c3::__callStatic\n\";\n"
        "  }\n"
        "  public function test1b() {\n"
        "    c3::foo(); // invokes c3::__callStatic\n"
        "  }\n"
        "}\n"
        "class d3 extends c3 {\n"
        "  public function test1b() {\n"
        "    c3::foo(); // invokes c3::__callStatic\n"
        "  }\n"
        "}\n"
        "\n"
        "c3::test1b();\n"
        "d3::test1b();\n",

        "c3::__callStatic\n"
        "c3::__callStatic\n"
       );

  MVCRO("<?php\n"
        "class a1 {\n"
        "  public function __call($func, $args) {\n"
        "    var_dump('a1::__call');\n"
        "  }\n"
        "  public function test() {\n"
        "    a1::foo();\n"
        "  }\n"
        "}\n"
        "$obj = new a1;\n"
        "$obj->test();\n",

        "string(10) \"a1::__call\"\n"
       );

  MVCRO("<?php\n"
        "class a1 {\n"
        "  public function __call($func, $args) {\n"
        "    var_dump('a1::__call');\n"
        "  }\n"
        "}\n"
        "class b1 {\n"
        "  public function test() {\n"
        "    a1::foo();\n"
        "  }\n"
        "}\n"
        "set_error_handler('h'); function h() { var_dump('errored');}"
        "$obj = new b1;\n"
        "$obj->test();\n"
        "var_dump('end');\n",

        "string(7) \"errored\"\n"
       );

  MVCRO("<?php\n"
        "class a2 {\n"
        "  public function __call($func, $args) {\n"
        "    var_dump('a2::__call');\n"
        "  }\n"
        "  public function test() {\n"
        "    a2::foo();\n"
        "  }\n"
        "}\n"
        "class b2 extends a2 {\n"
        "  public function test() {\n"
        "    a2::foo();\n"
        "    b2::foo();\n"
        "  }\n"
        "}\n"
        "$obj = new a2;\n"
        "$obj->test();\n"
        "$obj = new b2;\n"
        "$obj->test();\n",

        "string(10) \"a2::__call\"\n"
        "string(10) \"a2::__call\"\n"
        "string(10) \"a2::__call\"\n"
      );

  MVCRO("<?php\n"
        "class a1 {\n"
        "  public function __call($func, $args) {\n"
        "    var_dump('a1::__call');\n"
        "  }\n"
        "  public static function __callStatic($func, $args) {\n"
        "    var_dump('a1::__callStatic');\n"
        "  }\n"
        "  public function test() {\n"
        "    a1::foo();\n"
        "  }\n"
        "}\n"
        "class b1 {\n"
        "  public function test() {\n"
        "    a1::foo();\n"
        "  }\n"
        "}\n"
        "$obj = new a1;\n"
        "$obj->test();\n"
        "$obj = new b1;\n"
        "$obj->test();\n",

        "string(10) \"a1::__call\"\n"
        "string(16) \"a1::__callStatic\"\n"
       );

  MVCRO("<?php\n"
        "class a2 {\n"
        "  public function __call($func, $args) {\n"
        "    var_dump('a2::__call');\n"
        "  }\n"
        "  public static function __callStatic($func, $args) {\n"
        "    var_dump('a2::__callStatic');\n"
        "  }\n"
        "  public function test() {\n"
        "    a2::foo();\n"
        "  }\n"
        "}\n"
        "class b2 extends a2 {\n"
        "  public function test() {\n"
        "    a2::foo();\n"
        "    b2::foo();\n"
        "  }\n"
        "}\n"
        "$obj = new a2;\n"
        "$obj->test();\n"
        "$obj = new b2;\n"
        "$obj->test();\n",

        "string(10) \"a2::__call\"\n"
        "string(10) \"a2::__call\"\n"
        "string(10) \"a2::__call\"\n"
       );

  MVCRO("<?php\n"
        "class a1 {\n"
        "  public static function __callStatic($func, $args) {\n"
        "    var_dump('a1::__callStatic');\n"
        "  }\n"
        "  public function test() {\n"
        "    a1::foo();\n"
        "  }\n"
        "}\n"
        "class b1 {\n"
        "  public function test() {\n"
        "    a1::foo();\n"
        "  }\n"
        "}\n"
        "$obj = new a1;\n"
        "$obj->test();\n"
        "$obj = new b1;\n"
        "$obj->test();\n",

        "string(16) \"a1::__callStatic\"\n"
        "string(16) \"a1::__callStatic\"\n"
       );

  MVCRO("<?php\n"
        "class a2 {\n"
        "  public static function __callStatic($func, $args) {\n"
        "    var_dump('a2::__callStatic');\n"
        "  }\n"
        "  public function test() {\n"
        "    a2::foo();\n"
        "  }\n"
        "}\n"
        "class b2 extends a2 {\n"
        "  public function test() {\n"
        "    a2::foo();\n"
        "    b2::foo();\n"
        "  }\n"
        "}\n"
        "$obj = new a2;\n"
        "$obj->test();\n"
        "$obj = new b2;\n"
        "$obj->test();\n",

        "string(16) \"a2::__callStatic\"\n"
        "string(16) \"a2::__callStatic\"\n"
        "string(16) \"a2::__callStatic\"\n"
       );

  MVCRO("<?php\n"
        "class MethodTest {\n"
        "    public function __call($name, $arguments) {\n"
        "        var_dump($name, implode(', ', $arguments));\n"
        "    }\n"
        "    public static function __callStatic($name, $arguments) {\n"
        "        var_dump($name, implode(', ', $arguments));\n"
        "    }\n"
        "}\n"
        "$obj = new MethodTest;\n"
        "$obj->runTest('in object context');\n"
        "MethodTest::runTest('in static context');\n",

        "string(7) \"runTest\"\n"
        "string(17) \"in object context\"\n"
        "string(7) \"runTest\"\n"
        "string(17) \"in static context\"\n");

  return true;
}

bool TestCodeRun::TestNowDoc() {
  MVCRO("<?php\n"
        "$b = 'bad';\n"
        "$a = <<<'NOWDOC'\n"
        "$b\n"
        "NOWDOC;\n"
        "var_dump($a);\n"
        "$a = <<<\"NOWDOC\"\n"
        "$b\n"
        "NOWDOC;\n"
        "var_dump($a);\n"
        "$a = <<<NOWDOC\n"
        "$b\n"
        "NOWDOC;\n"
        "var_dump($a);\n",

        "string(2) \"$b\"\n"
        "string(3) \"bad\"\n"
        "string(3) \"bad\"\n"
       );

  MVCRO("<?php\n"
        "$a = <<<NOWDOC\n"
        "\"'\\t\n"
        "NOWDOC;\n"
        "var_dump($a);\n"
        "$a = <<<'NOWDOC'\n"
        "\"'\\t\n"
        "NOWDOC;\n"
        "var_dump($a);\n"
        "$a = <<<\"NOWDOC\"\n"
        "\"'\\t\n"
        "NOWDOC;\n"
        "var_dump($a);\n",

        "string(3) \"\"'\t\"\n"
        "string(4) \"\"'\\t\"\n"
        "string(3) \"\"'\t\"\n"
       );

  return true;
}

bool TestCodeRun::TestTernaryShortcut() {
  MVCRO("<?php\n"
        "function foo() { var_dump('hello'); return 789;}\n"
        "$a = 123 ?: 456;\n"
        "var_dump($a);\n"
        "$b[123] = 456;\n"
        "var_dump(isset($b[123]) ?: false);\n"
        "var_dump(foo()?:123);\n",

        "int(123)\n"
        "bool(true)\n"
        "string(5) \"hello\"\n"
        "int(789)\n"
       );

  return true;
}

bool TestCodeRun::TestGoto() {
  MVCRO("<?php goto a; echo 'Foo'; a: echo 'Bar';",
        "Bar");

  MVCRO("<?php function foo() { goto a; echo 'Foo'; a: echo 'Bar';} foo();",
        "Bar");

  MVCRO("<?php function foo() { "
        "goto a; b: echo 'Foo'; return;a: echo 'Bar'; goto b;} foo();",
        "BarFoo");

  MVCRO("<?php for($i=0,$j=50; $i<100; $i++) { "
        " while($j--) { if($j==17) goto end; }"
        "} "
        "echo 'no'; end: echo 'yes';",
        "yes");

  MVCRO("<?php goto a; if (false) { a: print 'here';} ",
        "here");

  return true;
}

bool TestCodeRun::TestClosure() {
  MVCRO("<?php $a = function ($v) { return $v > 2; }; echo $a(4).\"\n\";"
        " echo call_user_func_array($a, array(4));",
        "1\n1");

  MVCRO("<?php $a = function ($a) { return $a;}; var_dump($a(123));",
        "int(123)\n");

  MVCRO("<?php $abc = 123; $a = function () use ($abc) { var_dump($abc);}; "
        "$a();",
        "int(123)\n");

  MVCRO("<?php\n"
        "function foo() {\n"
        "  $var = 123;\n"
        "  $ref = 456;\n"
        "  $abc = 789;\n"
        "  $a = function () use ($var, &$ref) {\n"
        "    var_dump($abc, $var, $ref);\n"
        "    $abc = $var = $ref = 333;\n"
        "  };\n"
        "  var_dump($a());\n"
        "  var_dump($abc, $var, $ref);\n"
        "}\n"
        "foo();\n",

        "NULL\n"
        "int(123)\n"
        "int(456)\n"
        "NULL\n"
        "int(789)\n"
        "int(123)\n"
        "int(333)\n");

  MVCRO("<?php\n"
        "class Foo {\n"
        "  function bar() {\n"
        "    $abc = 123;\n"
        "    $a = function ($abc) use ($abc) {\n"
        "      var_dump($abc);\n"
        "    };\n"
        "    return $a;\n"
        "  }\n"
        "}\n"
        "$a = Foo::bar();\n"
        "$a(456);\n",

        "int(123)\n");

  MVCRO("<?php\n"
        "class Foo {\n"
        "  function bar() {\n"
        "    $abc = 123;\n"
        "    $a = function ($abc) use ($abc, $abc) {\n"
        "      var_dump($abc);\n"
        "    };\n"
        "    return $a;\n"
        "  }\n"
        "}\n"
        "$a = Foo::bar();\n"
        "$a(456);\n",

        "int(123)\n");

  MVCRO("<?php\n"
        "class Foo {\n"
        "  function bar() {\n"
        "    $abc = 123;\n"
        "    $a = function ($abc) use (&$abc, &$abc) {\n"
        "      var_dump($abc);\n"
        "    };\n"
        "    return $a;\n"
        "  }\n"
        "}\n"
        "$a = Foo::bar();\n"
        "$a(456);\n",

        "int(123)\n");

  MVCRO("<?php\n"
        "class Foo {\n"
        "  function bar() {\n"
        "    $abc = 123;\n"
        "    $a = function ($x) use ($abc) {\n"
        "      $n = func_num_args();\n"
        "      $args = func_get_args();\n"
        "      var_dump($n, $args);\n"
        "    };\n"
        "    return $a;\n"
        "  }\n"
        "\n"
        "  function baz($obj) {\n"
        "    $abc = 456;\n"
        "    $obj(789);\n"
        "  }\n"
        "}\n"
        "$a = Foo::bar();\n"
        "Foo::baz($a);\n",

        "int(1)\n"
        "array(1) {\n"
        "  [0]=>\n"
        "  int(789)\n"
        "}\n");

  MVCRO("<?php\n"
        "class Foo {\n"
        "  function bar() {\n"
        "    $a = function () { var_dump(__CLASS__, __FUNCTION__);};\n"
        "    $a();\n"
        "  }\n"
        "}\n"
        "Foo::bar();\n",

        "string(3) \"Foo\"\n"
        "string(9) \"{closure}\"\n");

  MVCRO("<?php\n"
        "function h() {\n"
        "  return array_filter(array(1, 2, 3),\n"
        "                      function($e) { return !($e & 1); });\n"
        "}\n"
        "h();\n"
        "var_dump(h());\n",

        "array(1) {\n"
        "  [1]=>\n"
        "  int(2)\n"
        "}\n");
  MVCRO("<?php\n"
        "$v=5;"
        "call_user_func("
        "  function() use($v) "
        "  { echo $v; }"
        ");"
        "$f = function() use($v) { echo $v; };"
        "call_user_func($f);"
        "call_user_func_array("
        "  function() use($v) "
        "  { echo $v; }, array()"
        ");"
        "call_user_func($f, array());", "5555");
  MVCRO("<?php\n"
        "$myfunc = function() {"
        "  echo \"hello, world!\\n\";"
        "};"
        "$myfunc();"
        "call_user_func($myfunc);"
        "call_user_func(\"$myfunc\");"
        "call_user_func_array($myfunc, array());"
        "call_user_func_array(\"$myfunc\", array());"
        "$isc = is_callable($myfunc, false, &$p);"
        "echo \"is_callable(\\$myfunc) = $isc\\n\";"
        "var_dump($p);",
        "hello, world!\n"
        "hello, world!\n"
        "hello, world!\n"
        "hello, world!\n"
        "hello, world!\n"
        "is_callable($myfunc) = 1\n"
        "string(17) \"Closure::__invoke\"\n");
  MVCRO("<?php\n"
        "function f() {"
        "$someVar = 456;"
        "$closure = function($param) use ($someVar) {"
        "echo $param . ' ' . $someVar . \"\\n\";"
        "};"
        "return $closure;"
        "}"
        "$x = f();"
        "$x(2);"
        "call_user_func($x, 2);",
        "2 456\n"
        "2 456\n");
  MVCRO("<?php\n"
        "$my_array = array(7, 1, 5, 6);"
        "$some_value = 'My print';"
        ""
        "usort($my_array,"
        "  function($a, $b) use ($some_value) { var_dump($some_value); }"
        ");",
        "string(8) \"My print\"\n"
        "string(8) \"My print\"\n"
        "string(8) \"My print\"\n"
        "string(8) \"My print\"\n");
  MVCRO("<?php\n"
        "function f() {"
        "  $someVar = 456;"
        "  $closure = function($param) use (&$someVar) {"
        "      echo $param . ' ' . $someVar . \"\\n\";"
        "      $param = 7;"
        "      $someVar = 11;"
        "    };"
        "  return $closure;"
        "}"
        "$x = f();"
        "$x(2);"
        "$x(2);",
        "2 456\n"
        "2 11\n");
  MVCRO("<?php\n"
        "function f() {"
        "  $someVar = 456;"
        "  $closure = function($param) use ($someVar) {"
        "      echo $param . ' ' . $someVar . \"\\n\";"
        "      $param = 7;"
        "      $someVar = 11;"
        "    };"
        "  return $closure;"
        "}"
        "$x = f();"
        "$x(2);"
        "$x(2);",
        "2 456\n"
        "2 456\n");
  MVCRO("<?php "
        "function test($x) {"
        "  $s_path = serialize($x);"
        "  $filter = function ($rel) use ($s_path) {"
        "    return $s_path;"
        "  };"
        "  var_dump($filter(0));"
        "}"
        "test('hello');"
        "test(array(1,2,'foo'=>'bar'));",
        "string(12) \"s:5:\"hello\";\"\n"
        "string(42) \"a:3:{i:0;i:1;i:1;i:2;s:3:\"foo\";s:3:\"bar\";}\"\n");
  MVCRO("<?php "
        "function test($a, $b) {"
        "  return array_map(function (array $x) use ($b) {"
        "      var_dump($x,$b);"
        "    }, $a);"
        "}"
        "test(array(array(1), array(2)), 5);",
        "array(1) {\n"
        "  [0]=>\n"
        "  int(1)\n"
        "}\n"
        "int(5)\n"
        "array(1) {\n"
        "  [0]=>\n"
        "  int(2)\n"
        "}\n"
        "int(5)\n");

  return true;
}

bool TestCodeRun::TestNamespace() {
  MVCRO("<?php\n"
        "namespace my\\name;\n"
        "class MyClass {}\n"
        "function myfunction() {}\n"
        "const MYCONST = 123;\n"
        "\n"
        "$a = new MyClass; var_dump(get_class($a));\n"
        "$c = new \\my\\name\\MyClass; var_dump(get_class($a));\n"
        "$a = strlen('hi'); var_dump($a);\n"
        "$d = namespace\\MYCONST; var_dump($d);\n"
        "$d = __NAMESPACE__ . '\\MYCONST'; var_dump(constant($d));\n"
        "var_dump(defined('MYCONST'));\n",

        "string(15) \"my\\name\\MyClass\"\n"
        "string(15) \"my\\name\\MyClass\"\n"
        "int(2)\n"
        "int(123)\n"
        "int(123)\n"
        "bool(false)\n"
       );

  MVCRO("<?php\n"
        "namespace foo\\baz {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace bar\\baz {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace {\n"
        "  use foo\\baz as baz;\n"
        "  baz\\foo();\n"
        "}\n",

        "string(7) \"foo\\baz\"\n");

  MVCRO("<?php\n"
        "namespace foo\\baz {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace bar\\baz {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace bar {\n"
        "  use foo\\baz as baz;\n"
        "  baz\\foo();\n"
        "}\n",

        "string(7) \"foo\\baz\"\n");

  MVCRO("<?php\n"
        "  namespace foo\\baz {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace bar\\baz {\n"
        "  function foo() { var_dump(__NAMESPACE__);}}\n"
        "namespace bar {\n"
        "  baz\\foo();\n"
        "}\n",

        "string(7) \"bar\\baz\"\n");

  MVCRO("<?php\n"
        "namespace {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "}\n"
        "namespace B {\n"
        "  foo();\n"
        "}\n",

        "string(0) \"\"\n");

  MVCRO("<?php\n"
        "namespace {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  foo();\n"
        "}\n",

        "string(1) \"B\"\n");

  MVCRO("<?php\n"
        "namespace {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  \\B\\foo();\n"
        "}\n",

        "string(1) \"B\"\n");

  MVCRO("<?php\n"
        "namespace {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  \\foo();\n"
        "}\n",

        "string(0) \"\"\n");

  MVCRO("<?php\n"
        "namespace {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  $a = 'foo';\n"
        "  $a();\n"
        "}\n",

        "string(0) \"\"\n");

  MVCRO("<?php\n"
        "namespace {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  function foo() { var_dump(__NAMESPACE__);}\n"
        "}\n"
        "namespace B {\n"
        "  call_user_func('foo');\n"
        "}\n",

        "string(0) \"\"\n");

  return true;
}

bool TestCodeRun::TestYield() {
  Option::EnableHipHopSyntax = true;

  MVCRO("<?php function fruit() { yield 'apple'; yield 'banana';} "
        "foreach (fruit() as $fruit) { var_dump($fruit);} ",

        "string(5) \"apple\"\n"
        "string(6) \"banana\"\n"
       );

  MVCRO("<?php class F { function fruit() { yield 'apple'; yield 'banana';} }"
        "foreach (F::fruit() as $fruit) { var_dump($fruit);} ",

        "string(5) \"apple\"\n"
        "string(6) \"banana\"\n"
       );

  MVCRO("<?php class F { function fruit() { yield 'apple'; yield 'banana';} }"
        "$f = new F; foreach ($f->fruit() as $fruit) { var_dump($fruit);} ",

        "string(5) \"apple\"\n"
        "string(6) \"banana\"\n"
       );

  MVCRO("<?php function fruit() { $a = 123; yield $a; yield ++$a;} "
        "foreach (fruit() as $fruit) { var_dump($fruit);} ",

        "int(123)\n"
        "int(124)\n"
       );

  MVCRO("<?php function fruit() { $a = 123; yield $a;yield break;yield ++$a;} "
        "foreach (fruit() as $fruit) { var_dump($fruit);} ",

        "int(123)\n"
       );

  MVCRO("<?php function nums() { for ($i = 0; $i < 3; $i++) yield $i;} "
        "foreach (nums() as $num) { var_dump($num);} ",

        "int(0)\n"
        "int(1)\n"
        "int(2)\n"
       );

  MVCRO("<?php function nums() { $i = 0; while ($i < 3) yield $i++;} "
        "foreach (nums() as $num) { var_dump($num);} ",

        "int(0)\n"
        "int(1)\n"
        "int(2)\n"
       );

  MVCRO("<?php function nums() { $i = 0; do yield $i++; while ($i < 3);} "
        "foreach (nums() as $num) { var_dump($num);} ",

        "int(0)\n"
        "int(1)\n"
        "int(2)\n"
       );

  MVCRO("<?php function nums() { $i = 0; foo: switch ($i) { "
        "case 0: yield $i; $i = 1; case 999: yield $i; break; $i = -1; "
        "case 1: $i = 2; yield $i; yield break;} goto foo;} "
        "foreach (nums() as $num) { var_dump($num);} ",

        "int(0)\n"
        "int(1)\n"
        "int(2)\n"
       );

  MVCRO("<?php\n"
        "function f() { yield func_num_args(); yield func_get_arg(1); }\n"
        "foreach (f(1, 2, 3) as $v) { var_dump($v); }\n",

        "int(3)\n"
        "int(2)\n"
       );

  MVCRO("<?php\n"
        "function f($a1, &$a2) {\n"
        "  foreach ($a1 as $k1 => $v1) {\n"
        "    foreach ($a2 as $k2 => &$v2) {\n"
        "      $v2 += $v1; yield $v2;\n"
        "    }\n"
        "  }\n"
        "}\n"
        "$a1 = array(1, 2);\n"
        "$a2 = array(1, 2);\n"
        "foreach (f($a1, $a2) as $v) { var_dump($v); }\n"
        "var_dump($a2[0], $a2[1]);\n",

        "int(2)\n"
        "int(3)\n"
        "int(4)\n"
        "int(5)\n"
        "int(4)\n"
        "int(5)\n");

  // yield within anonymous function
  MVCRO("<?php\n"
        "$a = function() { yield 1; yield 2; };\n"
        "foreach ($a() as $v) { var_dump($v); }\n",

        "int(1)\n"
        "int(2)\n");

  MVCRO("<?php\n"
        "function f() {\n"
        "  $a = function() { yield 1; yield 2; };\n"
        "  return $a;\n"
        "}\n"
        "$f = f();\n"
        "foreach ($f() as $v) { var_dump($v); }\n",

        "int(1)\n"
        "int(2)\n");

  MVCRO("<?php\n"
        "class A {\n"
        "  function f() {\n"
        "    $a = function() { yield 1; yield 2; };\n"
        "    return $a;\n"
        "  }\n"
        "}\n"
        "$a = new A;\n"
        "$f = $a->f();\n"
        "foreach ($f() as $v) { var_dump($v); }\n",

        "int(1)\n"
        "int(2)\n");

  // Continuatin::send()
  MVCRO("<?php\n"
        "function f() {\n"
        "  $a = yield 1; list($a, $b) = yield $a; yield $b;\n"
        "}\n"
        "$c = f();\n"
        "$c->next();\n"
        "var_dump($c->current());\n"
        "$c->send(2);\n"
        "var_dump($c->current());\n"
        "$c->send(array(3, 4));\n"
        "var_dump($c->current());\n",

        "int(1)\n"
        "int(2)\n"
        "int(4)\n");

  return true;
}

bool TestCodeRun::TestHint() {
  Option::EnableHipHopSyntax = true;

  MVCRO("<?php\n"
        "function f1(int $i = 1) { var_dump($i); }\n"
        "function f2(double $d = 5.5) { var_dump($d); }\n"
        "function f3(bool $b = true) { var_dump($b); }\n"
        "function f4(string $s = 'hello') { var_dump($s); }\n"
        "f1(); f2(); f3(); f4();\n",

        "int(1)\n"
        "float(5.5)\n"
        "bool(true)\n"
        "string(5) \"hello\"\n");

  MVCRO("<?php "
        "g(DATE_COOKIE);"
        "f(count(array()));"
        "function f(int $i) {"
        "  var_dump($i);"
        "}"
        "function g(string $s) {"
        "  var_dump($s);"
        "}",
        "string(16) \"l, d-M-y H:i:s T\"\n"
        "int(0)\n");

  return true;
}

bool TestCodeRun::TestParser() {
  MVCRO("<?php function foo() { return array(1, 2, 3);} var_dump(foo()[2]);",
        "int(3)\n");

  return true;
}

// please leave this unit test at last for debugging ad hoc code
bool TestCodeRun::TestAdHoc() {
  return true;
}
