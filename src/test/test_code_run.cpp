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
#include <lib/parser/parser.h>
#include <lib/system/builtin_symbols.h>
#include <lib/code_generator.h>
#include <lib/analysis/analysis_result.h>
#include <util/util.h>
#include <util/process.h>
#include <lib/option.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

TestCodeRun::TestCodeRun() : m_perfMode(false) {
  Option::GenerateCPPMain = true;
  Option::GenerateCPPMetaInfo = true;
  Option::GenerateCPPMacros = true;
  Option::GenerateCPPComments = true;
  Option::GenerateCPPNameSpace = true;
  Option::KeepStatementsWithNoEffect = false;
  Option::StaticMethodAutoFix = true;
}

bool TestCodeRun::CleanUp() {
  const char *argv[] = {"", NULL};
  string out, err;
  Process::Exec("cpp/tmp/cleanup.sh", argv, NULL, out, &err);
  if (!err.empty()) {
    printf("Failed to clean up cpp/tmp: %s\n", err.c_str());
    return false;
  }
  return true;
}

bool TestCodeRun::GenerateFiles(const char *input) {
  AnalysisResultPtr ar(new AnalysisResult());
  ar->setOutputPath("cpp/tmp");
  Parser::parseString(input, ar);
  BuiltinSymbols::load(ar);
  ar->loadBuiltins();
  ar->analyzeProgram();
  ar->preOptimize();
  ar->inferTypes();
  ar->postOptimize();
  ar->analyzeProgramFinal();
  ar->outputAllCPP(CodeGenerator::ClusterCPP, 0, NULL);
  return true;
}

bool TestCodeRun::CompileFiles(const char *input, const char *file, int line) {
  const char *argv[] = {"", NULL};
  string out, err;
  Process::Exec("cpp/makeall.sh", argv, NULL, out, &err);
  if (!err.empty()) {
    printf("%s:%d\nParsing: [%s]\nFailed to compile files: %s\n",
           file, line, input, err.c_str());
    return false;
  }
  return true;
}

bool TestCodeRun::VerifyCodeRun(const char *input, const char *output,
                                const char *file /* = "" */,
                                int line /* = 0 */,
                                bool nowarnings /* = false */) {
  ASSERT(input);
  if (!CleanUp()) return false;
  if (Option::EnableEval < Option::FullEval) {
    if (!GenerateFiles(input) ||
        !CompileFiles(input, file, line)) {
      return false;
    }
  }

  // generate main.php and get PHP's output
  string expected;
  if (output) {
    expected = output;
  } else {
    string fullPath = "cpp/tmp/main.php";
    ofstream f(fullPath.c_str());
    if (!f) {
      printf("Unable to open %s for write. Run this test from src/.\n",
             fullPath.c_str());
      return false;
    }

    f << input;
    f.close();

    const char *argv1[] = {"", "cpp/tmp/main.php", NULL};
    const char *argv2[] = {"", "-n", "cpp/tmp/main.php", NULL};
    string err;
    Process::Exec("php", nowarnings ? argv2 : argv1, NULL, expected, &err);
    if (!err.empty() && nowarnings) {
      printf("%s:%d\nParsing: [%s]\nFailed to run cpp/tmp/main.php: %s\n",
             file, line, input, err.c_str());
      return false;
    }
  }

  // run and verify output
  {
    string actual, err;
    if (Option::EnableEval < Option::FullEval) {
      const char *argv[] = {"", "--file=string", "--config=test/config.hdf",
                            NULL};
      Process::Exec("cpp/tmp/test", argv, NULL, actual, &err);
    } else {
      const char *argv[] = {"", "--file=cpp/tmp/main.php",
                            "--config=test/config.hdf", NULL};
      Process::Exec("hphpi/hphpi", argv, NULL, actual, &err);
    }

    if (m_perfMode) {
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

    if (actual != expected || !err.empty()) {
      printf("%s:%d\nParsing: [%s]\nBet %d:\n"
             "--------------------------------------\n"
             "%s"
             "--------------------------------------\n"
             "Got %d:\n"
             "--------------------------------------\n"
             "%s"
             "--------------------------------------\n"
             "Err: [%s]\n", file, line, input,
             (int)expected.length(), expected.c_str(),
             (int)actual.length(), actual.c_str(),
             err.c_str());
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestSanity);
  RUN_TEST(TestInnerFunction);
  RUN_TEST(TestInnerClass);
  RUN_TEST(TestVariableArgument);
  RUN_TEST(TestListAssignment);
  RUN_TEST(TestExceptions);
  RUN_TEST(TestPredefined);
  RUN_TEST(TestBoolean);
  RUN_TEST(TestInteger);
  RUN_TEST(TestDouble);
  RUN_TEST(TestString);
  RUN_TEST(TestLocale);
  RUN_TEST(TestArray);
  RUN_TEST(TestArrayEscalation);
  RUN_TEST(TestArrayOffset);
  RUN_TEST(TestArrayAccess);
  RUN_TEST(TestArrayIterator);
  RUN_TEST(TestArrayAssignment);
  RUN_TEST(TestArrayMerge);
  RUN_TEST(TestArrayUnique);
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
  RUN_TEST(TestDynamicMethods);
  RUN_TEST(TestVolatile);
  RUN_TEST(TestProgramFunctions);
  RUN_TEST(TestCompilation);
  RUN_TEST(TestReflection);
  RUN_TEST(TestReflectionClasses);
  RUN_TEST(TestErrorHandler);
  RUN_TEST(TestAssertOptions);
  RUN_TEST(TestExtMisc);
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
  RUN_TEST(TestLogicalOperators);
  RUN_TEST(TestGetClass);
  RUN_TEST(TestGetParentClass);
  RUN_TEST(TestRedeclaredFunctions);
  RUN_TEST(TestRedeclaredClasses);
  RUN_TEST(TestClone);
  RUN_TEST(TestEvalOrder);
  RUN_TEST(TestGetObjectVars);
  RUN_TEST(TestSerialization);
  //RUN_TEST(TestJson);
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
  RUN_TEST(TestFile);
  RUN_TEST(TestDirectory);
  RUN_TEST(TestAssignment);
  RUN_TEST(TestBadFunctionCalls);
  RUN_TEST(TestConstructor);
  RUN_TEST(TestTernary);
  RUN_TEST(TestUselessAssignment);
  RUN_TEST(TestTypes);
  RUN_TEST(TestSwitchStatement);
  RUN_TEST(TestExtImage);
  //RUN_TEST(TestEvaluationOrder);

  RUN_TEST(TestAdHoc);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// code generation

bool TestCodeRun::TestSanity() {
  VCR("<?php print 'Hello, World!';");
  VCR("Hello, World!");
  VCR("#!/usr/bin/env <?php\n"
      "#!/usr/bin/env php\n"
      "a /* show or not */ b\n"
      "Hello, World! # comments\n"
      "<?php\n"
      "print 'hello'; # comments");
  return true;
}

bool TestCodeRun::TestInnerFunction() {
  VCR("<?php function test() { print 'test';} test();");
  VCR("<?php function test() { function inner() { print 'test';} inner();} "
      "test();");
  return true;
}

bool TestCodeRun::TestInnerClass() {
  VCR("<?php class test { function p() { print 'test';} } "
      "$obj = new Test(); $obj->p();");
  VCR("<?php class test { function p() { function inner() { print 'test';} "
      "inner();} } $obj = new Test(); $obj->p();");
  VCR("<?php function test() { class test { function p() { print 'test';} }} "
      "test(); $obj = new Test(); $obj->p();");
  return true;
}

bool TestCodeRun::TestVariableArgument() {
  VCR("<?php function test() { "
      "  $n = func_num_args(); "
      "  var_dump($n);"
      "  $args = func_get_args();"
      "  var_dump($args);"
      "} "
      "test(); test(1); test(1, 2);");
  VCR("<?php function test() { "
      "  var_dump(func_get_arg(0));"
      "  var_dump(func_get_arg(1));"
      "} "
      "test(2, 'ok');");

  VCR("<?php function test($a) { "
      "  $n = func_num_args(); "
      "  var_dump($n);"
      "  $args = func_get_args();"
      "  var_dump($args);"
      "} "
      "test(1); test(1, 2); test(1, 2, 3);");
  VCR("<?php function test($a) { "
      "  var_dump(func_get_arg(0));"
      "  var_dump(func_get_arg(1));"
      "  var_dump(func_get_arg(2));"
      "} "
      "test(2, 'ok', array(1));");

  VCR("<?php function test($a, $b) { "
      "  $n = func_num_args(); "
      "  var_dump($n);"
      "  $args = func_get_args();"
      "  var_dump($args);"
      "} "
      "test(1, 2); test(1, 2, 3); test(1, 2, 3, 4);");
  VCR("<?php function test() { "
      "  var_dump(func_get_arg(0));"
      "  var_dump(func_get_arg(1));"
      "  var_dump(func_get_arg(2));"
      "  var_dump(func_get_arg(3));"
      "} "
      "test(2, 'ok', 0, 'test');");

  VCR("<?php function test($a) { "
      "  $n = func_num_args(); "
      "  var_dump($n);"
      "  $args = func_get_args();"
      "  var_dump($args);"
      "} "
      "test('test'); test(1, 2); test(1, 2, 3);");

  VCR("<?php class A { public function test($a) {"
      "  var_dump(func_num_args());"
      "  var_dump(func_get_args());"
      "}} $obj = new A(); $obj->test('test'); $obj->test(1, 2, 3);");

  VCR("<?php class A { public function __construct($a) {"
      "  var_dump(func_num_args());"
      "  var_dump(func_get_args());"
      "}} $obj = new A(1, 2, 3); $obj = new A('test');");

  VCR("<?php function test($a = 10) { "
      "  var_dump($a);"
      "  var_dump(func_get_args());"
      "} "
      "test(); test(1); test(1, 2);");

  VCR("<?php function test($a, $b = 10) { "
      "  var_dump($a);"
      "  var_dump($b);"
      "  var_dump(func_get_args());"
      "} "
      "test(1); test(1, 2); test(1, 2, 3);");

  // testing variable argument + reference parameter
  VCR("<?php $ar1 = array(10, 100, 100, 0); $ar2 = array(1, 3, 2, 4);"
      "array_multisort($ar1, $ar2); var_dump($ar1, $ar2);");

  return true;
}

bool TestCodeRun::TestListAssignment() {
  VCR("<?php $a = 'old'; var_dump(list($a) = false); var_dump($a);");
  VCR("<?php $a = 'old'; var_dump(list($a) = 'test'); var_dump($a);");
  VCR("<?php $a = 'old'; var_dump(list($a) = 123); var_dump($a);");
  VCR("<?php list() = array(1,2,3);");
  VCR("<?php list(,) = array(1,2,3);");
  VCR("<?php var_dump(list($a,) = array(1,2,3)); var_dump($a);");
  VCR("<?php var_dump(list(,$b) = array(1,2,3)); var_dump($b);");
  VCR("<?php var_dump(list($b) = array(1,2,3)); var_dump($b);");
  VCR("<?php var_dump(list($a,$b) = array(1,2,3)); "
      "var_dump($a); var_dump($b);");
  VCR("<?php var_dump(list($a,list($c),$b) = array(1,array(2),3));"
      "var_dump($a); var_dump($b); var_dump($c);");
  VCR("<?php $c = 'old'; var_dump(list($a,list($c),$b) = array(1,'test',3));"
      "var_dump($a); var_dump($b); var_dump($c);");
  VCR("<?php var_dump(list($a,list(),$b) = array(1,array(2),3));"
      "var_dump($a); var_dump($b);");
  VCR("<?php $info = array('coffee', 'brown', 'caffeine');"
      "list($a[0], $a[1], $a[2]) = $info;"
      "var_dump($a);");
  return true;
}

bool TestCodeRun::TestExceptions() {
  VCR("<?php try { throw new Exception('test');} "
      "catch (Exception $e) {}");
  VCR("<?php try { try { throw new Exception('test');} "
      "catch (InvalidArgumentException $e) {} } "
      "catch (Exception $e) { print 'ok';}");
  VCR("<?php class E extends Exception {} "
      "try { throw new E(); } catch (E $e) { print 'ok';}");
  VCR("<?php class E extends Exception {} class F extends E {}"
      "try { throw new F(); } catch (E $e) { print 'ok';}");
  VCR("<?php class E extends Exception { function __toString(){ return 'E';}} "
      "class F extends E { function __toString() { return 'F';}}"
      "try { throw new F(); } catch (E $e) { print $e;}");
  VCR("<?php "
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
  return true;
}

bool TestCodeRun::TestPredefined() {
  VCR("<?php \n\n\nvar_dump(/*__FILE__, */__LINE__);");
  VCR("<?php function Test() { var_dump(__FUNCTION__);} "
      "var_dump(__FUNCTION__); test();");
  VCR("<?php class A { "
      "function TestR() { var_dump(__CLASS__, __METHOD__);} "
      "static function Testm() { var_dump(__CLASS__, __METHOD__);}} "
      "function Testf() { var_dump(__CLASS__, __METHOD__);} "
      "testf(); A::testm(); $obj = new A(); $obj->testr();");
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// type system

bool TestCodeRun::TestBoolean() {
  VCR("<?php var_dump(true);");
  VCR("<?php var_dump(false);");
  VCR("<?php $a = 1; $b = ($a == 1); var_dump($b);");
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestInteger() {
  VCR("<?php var_dump(1);");
  VCR("<?php var_dump(0);");
  VCR("<?php var_dump(-1);");
  VCR("<?php var_dump(8589934592);");
  VCR("<?php var_dump(-8589934592);");
  VCR("<?php $a = 1;           var_dump($a);");
  VCR("<?php $a = 0;           var_dump($a);");
  VCR("<?php $a = -1;          var_dump($a);");
  VCR("<?php $a = 8589934592;  var_dump($a);");
  VCR("<?php $a = -8589934592; var_dump($a);");

  VCR("<?php $a = 10; var_dump(~$a);");
  VCR("<?php $a = 10; $b = 9; var_dump($a & $b);");
  VCR("<?php $a = 10; $b = 9; var_dump($a | $b);");
  VCR("<?php $a = 10; $b = 9; var_dump($a ^ $b);");
  VCR("<?php $a = 10; var_dump($a << 2);");
  VCR("<?php $a = 10; var_dump($a >> 2);");

  VCR("<?php $a = 10; $b = 9; $a &= $b; var_dump($a);");
  VCR("<?php $a = 10; $b = 9; $a |= $b; var_dump($a);");
  VCR("<?php $a = 10; $b = 9; $a ^= $b; var_dump($a);");
  VCR("<?php $a = 10; $b = 9; $a <<= 2; var_dump($a);");
  VCR("<?php $a = 10; $b = 9; $a >>= 2; var_dump($a);");

  VCR("<?php "
      "var_dump((integer)'10');"
      "var_dump((integer)'0x10');"
      "var_dump((integer)'010');"
      "var_dump(10 + 0x10);"
      "var_dump(10 + 010);");

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestDouble() {
  VCR("<?php var_dump(1.0);");
  VCR("<?php var_dump(0.0);");
  VCR("<?php var_dump(-1.0);");
  VCR("<?php var_dump(1/3);");
  VCR("<?php $a = 1/3; var_dump($a);");
  VCR("<?php $a = 1/3; $b = $a; var_dump($b);");

  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
      "var_dump((double)'10');"
      "var_dump((double)'0x10');"
      "var_dump((double)'010');"
      "var_dump(10.0 + 0x10);"
      "var_dump(10.0 + 010.0);");

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestString() {
  VCR("<?php print '\\'\\\\\"';");
  VCR("<?php print 'test\nok';");
  VCR("<?php print \"test\nok\";");
  VCR("<?php print \"test\\n\\r\\t\\v\\f\\\\\\$\\\"\";");
  VCR("<?php print \"\\1\\12\\123\\1234\\xA\\xAB\";");
  VCR("<?php print 'test\\n';");

  VCR("<?php $a = 'test'; $b = $a; print $b;");
  VCR("<?php $a = 'test'; $b = $a; $a = 'changed'; print $b;");
  VCR("<?php $a = 'test'; $b = $a; $a = 'changed'; print $a;");
  VCR("<?php $a = 'test'; $b = $a; $b = 'changed'; print $a;");

  VCR("<?php $a = 'a'; $b = 'b'; $c = 'a' . 'b'; print $c;");
  VCR("<?php $a = 'a'; $b = 'b'; $c = $a  . 'b'; print $c;");
  VCR("<?php $a = 'a'; $b = 'b'; $c = 'a' . $b ; print $c;");
  VCR("<?php $a = 'a'; $b = 'b'; $b .= $a;       print $b;");

  VCR("<?php $a = 'test'; print $a{0};");
  VCR("<?php $a = 'test'; print '['.$a{-1}.']';");
  VCR("<?php $a = 'test'; print '['.$a{100}.']';");

  VCR("<?php $a = 'test'; print $a[0];");
  VCR("<?php $a = 'test'; print $a['junk'];");
  VCR("<?php $a = 'test'; print '['.$a[-1].']';");
  VCR("<?php $a = 'test'; print '['.$a[100].']';");

  VCR("<?php $a = 'test'; $a[0] = 'ABC'; var_dump($a);")
  VCR("<?php $a = 'test'; $a[10] = 'ABC'; var_dump($a);")
  VCR("<?php $a = 'test'; $b = $a; $a[0] = 'ABC'; var_dump($a); var_dump($b);")
  VCR("<?php $a = 'test'; $b = $a; $a[10] = 'ABC'; var_dump($a);var_dump($b);")
  VCR("<?php $a = 'test'; $b = $a; $b[0] = 'ABC'; var_dump($a); var_dump($b);")
  VCR("<?php $a = 'test'; $b = $a; $b[10] = 'ABC'; var_dump($a);var_dump($b);")

  VCR("<?php $a = 'test'; var_dump(~$a);");
  VCR("<?php $a = 'test'; $b = 'zzz'; var_dump($a & $b);");
  VCR("<?php $a = 'test'; $b = 'zzz'; var_dump($a | $b);");
  VCR("<?php $a = 'test'; $b = 'zzz'; var_dump($a ^ $b);");
  VCR("<?php $a = 'test'; $b = 'zzz'; $a &= $b; var_dump($a);");
  VCR("<?php $a = 'test'; $b = 'zzz'; $a |= $b; var_dump($a);");
  VCR("<?php $a = 'test'; $b = 'zzz'; $a ^= $b; var_dump($a);");
  VCR("<?php $a = 'zzz'; $b = 'test'; var_dump($a & $b);");
  VCR("<?php $a = 'zzz'; $b = 'test'; var_dump($a | $b);");
  VCR("<?php $a = 'zzz'; $b = 'test'; var_dump($a ^ $b);");
  VCR("<?php $a = 'zzz'; $b = 'test'; $a &= $b; var_dump($a);");
  VCR("<?php $a = 'zzz'; $b = 'test'; $a |= $b; var_dump($a);");
  VCR("<?php $a = 'zzz'; $b = 'test'; $a ^= $b; var_dump($a);");
  VCR("<?php $a = 'zzz'; $a++; var_dump($a);");
  VCR("<?php $a = 'zzz'; ++$a; var_dump($a);");
  VCR("<?php $a = 'zzz'; $a--; var_dump($a);");
  VCR("<?php $a = 'zzz'; --$a; var_dump($a);");

  VCR("<?php $a = 'abc'; var_dump(isset($a[1], $a[2], $a[3]));");

  // serialization of binary string
  VCR("<?php var_dump(bin2hex(serialize(\"a\\x00b\")));");

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestArray() {
  VCR("<?php var_dump(array('b' => '2', 'a' => '1'));");
  VCR("<?php var_dump(array(1 => 'a', 0 => 'b'));");

  VCR("<?php $a = array();                       var_dump($a);");
  VCR("<?php $a = array(1);                      var_dump($a);");
  VCR("<?php $a = array(2, 1);                   var_dump($a);");
  VCR("<?php $a = array('1');                    var_dump($a);");
  VCR("<?php $a = array('2', '1');               var_dump($a);");
  VCR("<?php $a = array('a' => 1);               var_dump($a);");
  VCR("<?php $a = array('b' => 2, 'a' => 1);     var_dump($a);");
  VCR("<?php $a = array('a' => '1');             var_dump($a);");
  VCR("<?php $a = array('b' => '2', 'a' => '1'); var_dump($a);");

  VCR("<?php $a = array('a' => 1, 'a' => 2); var_dump($a);");
  VCR("<?php $a = array('a' => 1, 'b' => 2, 'a' => 3); var_dump($a);");

  VCR("<?php $a = array(1); $b = $a;                var_dump($b);");
  VCR("<?php $a = array(1); $b = $a; $a = array(2); var_dump($b);");
  VCR("<?php $a = array(1); $b = $a; $a = array(2); var_dump($a);");
  VCR("<?php $a = array(1); $b = $a; $b = array(2); var_dump($a);");

  VCR("<?php $a = array(); foreach ($a as $item) print '['.$item.']';");
  VCR("<?php $a = array(1); foreach ($a as $item) print '['.$item.']';");
  VCR("<?php $a = array(2,1); foreach ($a as $item) print '['.$item.']';");
  VCR("<?php $a = array('b' => 2, 'a' => 1); "
      "foreach ($a as $item) print '['.$item.']';");
  VCR("<?php $a = array('b' => 2, 'a' => 1); "
      "foreach ($a as $name => $item) print '['.$name.'=>'.$item.']';");

  VCR("<?php $a = array(2,1); var_dump($a[0]);");
  VCR("<?php $a = array(2,1); var_dump($a[-1]);");
  VCR("<?php $a = array(2,1); var_dump($a[3]);");
  VCR("<?php $a = array('b' => 2, 'a' => 1); var_dump($a['b']);");
  VCR("<?php $a = array('b' => 2, 'a' => 1); var_dump($a['bogus']);");
  VCR("<?php $a = array(2, 'test' => 1); var_dump($a);");
  VCR("<?php $a = array(1.2 => 'test'); var_dump($a[1]);");

  VCR("<?php $a = array(1, 'test'); var_dump($a);");
  VCR("<?php $a = array(); $a[] = 3; $a[] = 'test'; var_dump($a);");

  VCR("<?php $a = array();     $a['test'] = 3; var_dump($a);");
  VCR("<?php $a = array(1);    $a['test'] = 3; var_dump($a);");
  VCR("<?php $a = array(1, 2); $a[10] = 3;     var_dump($a);");
  VCR("<?php $a = array(1, 2); $a['10'] = 3;   var_dump($a);");
  VCR("<?php $a = array(1, 2); $b = $a; $a['10'] = 3; var_dump($b);");
  VCR("<?php $a = array(1, 2); $b = $a; $a['10'] = 3; var_dump($a);");

  VCR("<?php $a[] = 3; var_dump($a);");
  VCR("<?php $a = array(); $a[] = 3; var_dump($a);");
  VCR("<?php $a = array(1); $a[] = 3; var_dump($a);");
  VCR("<?php $a = array(1,2); $a[] = 3; var_dump($a);");
  VCR("<?php $a = ''; $a[] = 'test'; var_dump($a);");

  VCR("<?php $a = array(1, 2); "
      "foreach ($a as $item) { "
      "  print 'A['.$item.']'; "
      "  if ($item == 1) $a[] = 'new item'; "
      "} "
      "foreach ($a as $item) { "
      "  print 'B['.$item.']'; "
      "}"
      "var_dump($a);");

  VCR("<?php $a = array(1); $b = array(2); $c = $a + $b; var_dump($c);");
  VCR("<?php $a = array(1,2); $b = array(2,3); $c = $a + $b; var_dump($c);");
  VCR("<?php $a = array(1,2); $b = array(2); $c = $a + $b; var_dump($c);");
  VCR("<?php $a = array(1); $b = array(2,3); $c = $a + $b; var_dump($c);");

  VCR("<?php "
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

  VCR("$a = array('a' => 1, 'b' => 2);"
      "foreach ($a as $b => $c) {"
      "  var_dump($b);"
      "  unset($a['b']);"
      "}");

  VCR("$a = array('a' => 1, 'b' => 2);"
      "foreach ($a as $b => &$c) {"
      "  var_dump($b);"
      "  unset($a['b']);"
      "}");

  VCR("<?php "
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

  VCR("<?php "
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

  VCR("<?php "
      "$a = array('a' => 'apple', 'b' => 'banana', 'c' => 'citrus');"
      "foreach ($a as $k1 => &$v1) {"
      "  foreach ($a as $k2 => &$v2) {"
      "    if ($k2 == 'a') {"
      "      unset($a[$k2]);"
      "    }"
      "    var_dump($v1, $v2);"
      "  }"
      "}");
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestScalarArray() {
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestRange() {
  VCR("<?php "
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
  VCR("<?php $a = " exp "; $a[] = 1;              var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[] = 'test';         var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[] = array(0);       var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[0] = 1;             var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[0] = 'test';        var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[0] = array(0);      var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[1] = 1;             var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[1] = 'test';        var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[1] = array(0);      var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[2] = 1;             var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[2] = 'test';        var_dump($a);");      \
  VCR("<?php $a = " exp "; $a[2] = array(0);      var_dump($a);");      \
  VCR("<?php $a = " exp "; $a['test'] = 1;        var_dump($a);");      \
  VCR("<?php $a = " exp "; $a['test'] = 'test';   var_dump($a);");      \
  VCR("<?php $a = " exp "; $a['test'] = array(0); var_dump($a);");      \

#define TEST_ARRAY_PLUS(exp)                                            \
  VCR("<?php $a = " exp "; $a += array();                var_dump($a);"); \
  VCR("<?php $a = " exp "; $a += array(20);              var_dump($a);"); \
  VCR("<?php $a = " exp "; $a += array('b');             var_dump($a);"); \
  VCR("<?php $a = " exp "; $a += array(array(3));        var_dump($a);"); \
  VCR("<?php $a = " exp "; $a += array('c' => 20);       var_dump($a);"); \
  VCR("<?php $a = " exp "; $a += array('c' => 'b');      var_dump($a);"); \
  VCR("<?php $a = " exp "; $a += array('c' => array(3)); var_dump($a);"); \

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
  VCR("<?php $a['test_cache_2'] = 10; print $a['test_cache_26'];");
  VCR("<?php $a = array(10); $b = $a[0]; var_dump($b);");
  VCR("<?php $a = array(10); $b = $a[0] + 15; var_dump($b);");
  VCR("<?php $a = 1; $a = array($a); $a = array($a); var_dump($a);");
  VCR("<?php $a['A'] = array(1, 2); foreach ($a['A'] as $item) print $item;");
  VCR("<?php $a['A']['B'] = 1; var_dump($a);");
  VCR("<?php $a['A'] = 10; $a['A']++; var_dump($a);");
  VCR("<?php $a['A'] = 10; ++$a['A']; var_dump($a);");
  VCR("<?php $a['A'] = 10; $a['A'] .= 'test'; var_dump($a);");
  VCR("<?php $a['A'] = 10; $a['A'] += 25; var_dump($a);");
  VCR("<?php $a[null] = 10;"
      "var_dump($a[null]);"
      "var_dump($a[\"\"]);"
      "var_dump($a['']);");
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestArrayAccess() {
  VCR("<?php "
      "class A implements ArrayAccess {"
      "  public $a;"
      "  public function offsetExists($offset) {"
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
      "var_dump($obj['a']);"
      "unset($obj['a']);"
      "var_dump($obj['a']);"
      );

  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestArrayIterator() {
  VCR("<?php "
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
  return true;
}

bool TestCodeRun::TestArrayAssignment() {
  VCR("<?php "
      "$a = array(1, 2, 3);"
      "$b = $a;"
      "$b[4] = 4;"
      "var_dump($a);"
      "var_dump($b);"
      "$b = 3;"
      "var_dump($a);"
      "var_dump($b);");
  VCR("<?php "
      "$a = array('1', '2', '3');"
      "$b = $a;"
      "$b[4] = '4';"
      "var_dump($a);"
      "var_dump($b);"
      "$b = '3';"
      "var_dump($a);"
      "var_dump($b);");
  VCR("<?php "
      "$a = array(1.5, 2.5, 3.5);"
      "$b = $a;"
      "$b[4] = 4.5;"
      "var_dump($a);"
      "var_dump($b);"
      "$b = 3.5;"
      "var_dump($a);"
      "var_dump($b);");
  VCR("<?php "
      "$a = array(1, 'hello', 3.5);"
      "$b = $a;"
      "$b[4] = 'world';"
      "var_dump($a);"
      "var_dump($b);");
  VCR("<?php "
      "$a = array(1, 'hello', 3.5);"
      "$b = $a;"
      "$b[4] = 'world';"
      "var_dump($a);"
      "var_dump($b);"
      "$b = 3;"
      "var_dump($a);"
      "var_dump($b);");
  VCR("<?php "
      "$a = array('a' => '1', 2 => 2, 'c' => '3');"
      "var_dump($a);"
      "$a = array('a' => '1', 2 => 2, 'c' => '3',"
      "           'd' => array('a' => '1', 2 => 2, 'c' => '3'));"
      "var_dump($a);");
  VCR("<?php "
      "$a = array(1=>'main', 2=>'sub');"
      "$b = $a;"
      "var_dump(array_pop($b));"
      "print_r($a);"
      "var_dump(array_shift($b));"
      "print_r($a);");
  VCR("<?php "
      "$a = array(1, 2, 3);"
      "var_dump($a);"
      "array_pop($a);"
      "var_dump($a);"
      "array_shift($a);"
      "var_dump($a);");
  VCR("<?php "
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

  VCR("<?php $a = false; $a['a'] = 10;");
  VCR("<?php $a = false; $a['a']['b'] = 10;");

  return true;
}

bool TestCodeRun::TestArrayMerge() {
  VCR("<?php $a = array(1 => 1, 3 => 3); "
      "var_dump(array_merge($a, array(2)));");
  VCR("<?php $a = array(1 => 1, 3 => 3); "
      "var_dump(array_merge($a, array()));");
  VCR("<?php $a = array('a' => 1, 3 => 3); "
      "var_dump(array_merge($a, array(2)));");
  VCR("<?php $a = array('a' => 1, 'b' => 3); "
      "var_dump(array_merge($a, array(2)));");
  VCR("<?php $a = array('a' => 1, 3 => 3); "
      "var_dump(array_merge($a, array('a' => 2)));");
  VCR("<?php $a = array('a' => 1, 3 => 3); "
      "var_dump(array_merge($a, array('b' => 2)));");
  VCR("<?php $a = array('a' => 1, 'b' => 3); "
      "var_dump(array_merge($a, array('c' => 2)));");
  return true;
}

bool TestCodeRun::TestArrayUnique() {
  VCR("<?php "
      "var_dump(array_unique(array(array(1,2), array(1,2), array(3,4),)));");
  VCR("<?php "
      "$input = array(\"a\" => \"green\","
      "               \"red\", \"b\" => \"green\", \"blue\", \"red\");"
      "$result = array_unique($input);"
      "print_r($result);");
  VCR("<?php "
      "$input = array(4, \"4\", \"3\", 4, 3, \"3\");"
      "$result = array_unique($input);"
      "var_dump($result);");
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestCodeRun::TestVariant() {
  VCR("<?php $a = 1; $a = 'test'; print $a;");
  VCR("<?php $a = 1; $a = 'test'; $a .= 'b'; print $a;");

  VCR("<?php $a = array(); $a[] = 3; $a = 'test'; var_dump($a);");
  VCR("<?php $a = array(); $a['test'] = 3; var_dump($a);");
  VCR("<?php $a['test'] = 3; var_dump($a);");

  VCR("<?php $a = 1; $a = 'test'; print $a{0};");
  VCR("<?php $a=1;$a='t'; $a[0]  = 'AB'; var_dump($a);");
  VCR("<?php $a=1;$a='';  $a[0]  = 'AB'; var_dump($a);");
  VCR("<?php $a=1;$a='t'; $a[10] = 'AB'; var_dump($a);");
  VCR("<?php $a=1;$a='t'; $b = $a; $a[0] = 'AB'; var_dump($a); var_dump($b);");
  VCR("<?php $a=1;$a='t'; $b = $a; $a[10]= 'AB'; var_dump($a); var_dump($b);");
  VCR("<?php $a=1;$a='t'; $b = $a; $b[0] = 'AB'; var_dump($a); var_dump($b);");
  VCR("<?php $a=1;$a='t'; $b = $a; $b[10]= 'AB'; var_dump($a); var_dump($b);");

  VCR("<?php $a = 't'; $a = 1; print $a + 2;");
  VCR("<?php $a = 't'; $a = 1; print 2 + $a;");
  VCR("<?php $a = 't'; $a = 1; $b = 'a'; $b = 2; print $a + $b;");
  VCR("<?php $a = 't'; $a = 1; $a += 2; print $a;");
  VCR("<?php $a = 't'; $a = 1; $a += 'n'; print $a;");
  VCR("<?php $a = 't'; $a = 1; $a += '5'; print $a;");
  VCR("<?php $b = 'test'; $b = 1; $a += $b; print $a;");

  VCR("<?php $a = 't'; $a = 1; print -$a;");
  VCR("<?php $a = 't'; $a = -$a; print $a;");
  VCR("<?php $a = 't'; $a = 1; print $a - 2;");
  VCR("<?php $a = 't'; $a = 1; print 2 - $a;");
  VCR("<?php $a = 't'; $a = 1; $b = 'a'; $b = 2; print $a - $b;");
  VCR("<?php $a = 't'; $a = 1; $a -= 2; print $a;");
  VCR("<?php $a = 't'; $a = 1; $a -= 'n'; print $a;");
  VCR("<?php $a = 't'; $a = 1; $a -= '5'; print $a;");

  VCR("<?php $a = 't'; $a = 10; print $a * 2;");
  VCR("<?php $a = 't'; $a = 10; print 2 * $a;");
  VCR("<?php $a = 't'; $a = 10; $b = 'a'; $b = 2; print $a * $b;");
  VCR("<?php $a = 't'; $a = 10; $a *= 2; print $a;");
  VCR("<?php $a = 't'; $a = 10; $a *= 'n'; print $a;");
  VCR("<?php $a = 't'; $a = 10; $a *= '5'; print $a;");

  VCR("<?php $a = 't'; $a = 10; print $a / 2;");
  VCR("<?php $a = 't'; $a = 10; print 2 / $a;");
  VCR("<?php $a = 't'; $a = 10; $b = 'a'; $b = 2; print $a / $b;");
  VCR("<?php $a = 't'; $a = 10; $a /= 2; print $a;");
  VCR("<?php $a = 't'; $a = 10; $a /= '5'; print $a;");

  VCR("<?php $a = 't'; $a = 10; print $a % 2;");
  VCR("<?php $a = 't'; $a = 10; print 2 % $a;");
  VCR("<?php $a = 't'; $a = 10; $b = 'a'; $b = 2; print $a % $b;");
  VCR("<?php $a = 't'; $a = 10; $a %= 2; print $a;");
  VCR("<?php $a = 't'; $a = 10; $a %= '5'; print $a;");

  VCR("<?php $a = 't'; $a = 10; var_dump(~$a);");

  VCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a & $b);");
  VCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a | $b);");
  VCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a ^ $b);");
  VCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a << 2);");
  VCR("<?php $a = 't'; $a = 10; $b = 9; var_dump($a >> 2);");

  VCR("<?php $b = 't'; $a = 10; $b = 9; var_dump($a & $b);");
  VCR("<?php $b = 't'; $a = 10; $b = 9; var_dump($a | $b);");
  VCR("<?php $b = 't'; $a = 10; $b = 9; var_dump($a ^ $b);");
  VCR("<?php $b = 't'; $a = 10; $b = 2; var_dump($a << $b);");
  VCR("<?php $b = 't'; $a = 10; $b = 2; var_dump($a >> $b);");

  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a & $b);");
  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a | $b);");
  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a ^ $b);");
  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 2; var_dump($a << $b);");
  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 2; var_dump($a >> $b);");

  VCR("<?php $a = 't'; $a = 10; $b = 9; $a &= $b; var_dump($a);");
  VCR("<?php $a = 't'; $a = 10; $b = 9; $a |= $b; var_dump($a);");
  VCR("<?php $a = 't'; $a = 10; $b = 9; $a ^= $b; var_dump($a);");
  VCR("<?php $a = 't'; $a = 10; $b = 9; $a <<= 2; var_dump($a);");
  VCR("<?php $a = 't'; $a = 10; $b = 9; $a >>= 2; var_dump($a);");

  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a and $b);");
  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a or $b);");
  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a xor $b);");
  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump(!$a);");
  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a && $b);");
  VCR("<?php $a = 't'; $b = 't'; $a = 10; $b = 9; var_dump($a || $b);");

  VCR("<?php $a = 10; ++$a; var_dump($a);");
  VCR("<?php $a = 10; $a++; var_dump($a);");
  VCR("<?php $a = 10; --$a; var_dump($a);");
  VCR("<?php $a = 10; $a--; var_dump($a);");

  VCR("<?php $a = 't'; $a = 10; ++$a; var_dump($a);");
  VCR("<?php $a = 't'; $a = 10; $a++; var_dump($a);");
  VCR("<?php $a = 't'; $a = 10; --$a; var_dump($a);");
  VCR("<?php $a = 't'; $a = 10; $a--; var_dump($a);");

  VCR("<?php $a = 'test'; ++$a; var_dump($a);");
  VCR("<?php $a = 'test'; $a++; var_dump($a);");
  VCR("<?php $a = 'test'; --$a; var_dump($a);");
  VCR("<?php $a = 'test'; $a--; var_dump($a);");

  VCR("<?php $a = 1; $a = 'test'; var_dump(~$a);");
  VCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; var_dump($a & $b);");
  VCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; var_dump($a | $b);");
  VCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; var_dump($a ^ $b);");
  VCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; $a &= $b; var_dump($a);");
  VCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; $a |= $b; var_dump($a);");
  VCR("<?php $a = 1; $a = 'test'; $b = 'zzz'; $a ^= $b; var_dump($a);");
  VCR("<? class a { public $var2 = 1; public $var1; }"
      "class b extends a { public $var2; }"
      "function f() { $obj1 = new b(); var_dump($obj1); $obj1->var1 = 1; }"
      "f();"); //#147156
      return true;
}

bool TestCodeRun::TestObject() {
  VCR("<?php "
      "var_dump((object)NULL);"
      "var_dump((object)true);"
      "var_dump((object)10);"
      "var_dump((object)'test');"
      "var_dump((object)array(10, 20));"
      );

  VCR("<?php class A {} $obj = new A(); "
      "var_dump($obj);"
      "var_dump((bool)$obj);"
      "var_dump((int)$obj);"
      "var_dump((array)$obj);"
      "var_dump((object)$obj);"
      );

  VCR("<?php class A { public $test = 'ok';} $obj = new A(); "
      "var_dump($obj);"
      "var_dump((bool)$obj);"
      "var_dump((int)$obj);"
      "var_dump((array)$obj);"
      "var_dump((object)$obj);"
      );

  VCR("<?php "
      "var_dump((object)NULL);"
      "var_dump((object)true);"
      "var_dump((object)10);"
      "var_dump((object)'test');"
      "var_dump((object)array(10, 20));"
      );

  VCR("<?php class A { public $a = 0;} class B extends A {}"
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

  VCR("<?php "
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

  VCR("<?php "
      "class A {"
      "  public $b = 3;"
      "  public $a = 2;"
      "}"
      "$obj = new A(); var_dump($obj); var_dump($obj->c);"
      );

  VCR("<?php "
      "class A {"
      "  public $a = 2;"
      "}"
      "class B {"
      "  public $b = 3;"
      "}"
      "$obj = new A(); var_dump($obj); /*var_dump($obj->b);*/" // undeclared
      "$obj = new B(); var_dump($obj); var_dump($obj->b);"
      );

  VCR("<?php "
      "class A {"
      "  public $a = 2;"
      "}"
      "class B {"
      "  public $b = 3;"
      "}"
      "$obj = new A(); var_dump($obj); var_dump($obj->a);"
      "$obj = new B(); var_dump($obj); var_dump($obj->b);"
      );

  VCR("<?php "
      "class A {"
      "  public $a = 2;"
      "}"
      "class B extends A {"
      "  public $b = 3;"
      "}"
      "$obj = new A(); var_dump($obj); var_dump($obj->b);"
      "$obj = new B(); var_dump($obj); var_dump($obj->b);"
      );

  VCR("<?php "
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

  VCR("<?php "
      "interface I { public function test($a);}"
      "class A implements I { public function test($a) { print $a;}}"
      "$obj = new A(); var_dump($obj instanceof I); $obj->test('cool');"
      );

  VCR("<?php "
      "interface I { public function test($a);} "
      "class A { public function test($a) { print 'A';}} "
      "class B extends A implements I { "
      "  public function test($a) { print 'B';} "
      "}"
      "$obj = new A(); $obj->test(1);"
      "$obj = new B(); $obj->test(1);"
      );

  // circular references
  VCR("<?php class A { public $a;} "
      "$obj1 = new A(); $obj2 = new A(); $obj1->a = $obj2; $obj2->a = $obj1;"
      "var_dump($obj1);");

  VCR("<?php $a = 1; class A { public function t() { global $a; $b = 'a'; var_dump($$b);}} $obj = new A(); $obj->t();");

  VCR("<?php "
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
  VCR("<?php "
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


  return true;
}

bool TestCodeRun::TestObjectProperty() {

  VCR("<?php "
      "class A { public $a = 10; public function foo() { $this->a = 20;} } "
      "class B extends A { public $a = 'test';} "
      "$obj = new B(); $obj->foo(); var_dump($obj->a);");

  VCR("<?php "
      "class A { public $a = null; }"
      "class B extends A { public function foo() { var_dump($this->a);} } "
      "class C extends B { public $a = 'test';} "
      "$obj = new C(); $obj->foo();");
  VCR("<?php "
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
  VCR("<?php "
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

  VCR("<?php "
      "$one = array('cluster'=> 1, 'version'=>2);"
      "var_dump(isset($one->cluster));"
      "var_dump(empty($one->cluster));");

  return true;
}

bool TestCodeRun::TestObjectMethod() {
  VCR("<?php class A { function test() {}} "
      "$obj = new A(); $obj->test(); $obj = 1;");

  // calling a function that's implemented in a derived class
  VCR("<?php abstract class T { function foo() { $this->test();} }"
      "class R extends T { function test() { var_dump('test'); }} "
      "$obj = new R(); $obj->test(); $obj->foo();");

  VCR("<?php class A { function test() { print 'A';} "
      "function foo() { $this->test();}} "
      "class B extends A { function test() { print 'B';}} "
      "$obj = new A(); $obj = new B(); $obj->foo();");

  VCR("<?php "
      "class A {} "
      "class AA extends A { function test() { print 'AA ok';} }"
      "class B { function foo(A $obj) { $obj->test();}}"
      "$obj = new AA(); $b = new B(); $b->foo($obj);"
      );

  // calling a virtual function
  VCR("<?php abstract class T { abstract function test(); "
      "function foo() { $this->test();} }"
      "class R extends T { function test() { var_dump('test'); }} "
      "$obj = new R(); $obj->test(); $obj->foo();");

  // calling a virtual function
  VCR("<?php abstract class T { abstract function test(); } "
      "class R extends T { function test() { var_dump('test'); }} "
      "$obj = 1; $obj = new R(); $obj->test();");

  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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

  VCR("<?php "
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
  return true;
}

bool TestCodeRun::TestClassMethod() {
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestObjectMagicMethod() {
  VCR("<?php class A { "
      "  public $a = array(); "
      "  function __set($name, $value) { $this->a[$name] = $value.'set';} "
      "  function __get($name) { return $this->a[$name].'get';} "
      "} "
      "$obj = new A(); $obj->test = 'test'; var_dump($obj->test);");

  VCR("<?php "
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

  VCR("<?php class A { "
      "  public $a = array(); "
      "  function __set($name, $value) { $this->a[$name] = $value;} "
      "  function __get($name) { return $this->a[$name];} "
      "} "
      "$obj = new A(); $obj->test = 'test'; var_dump($obj->test);");

  VCR("<?php class A {} "
      "$obj = new A(); $obj->test = 'test'; var_dump($obj->test);");

  VCR("<?php class A { "
      "function __call($a, $b) { var_dump($a, $b[0], $b[1]);}} "
      "$obj = new A(); $a = 1; $obj->test($a, 'ss');");
  /*
  VCR("<?php class A { "
      "  function __set($name, $value) { $this->a[$name] = $value;} "
      "  function __get($name) { return $this->a[$name];} "
      "} "
      "$obj = new A(); $obj->test = 'test'; var_dump($obj->test);");
  */

  VCR("<?php "
      "class foo"
      "{"
      "  public $public = 'public';"
      "  public function __sleep()"
      "  { return array('public'); }"
      "}"
      "$foo = new foo();"
      "$data = serialize($foo);"
      "var_dump($data);");
  VCR("<?php "
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
  VCR("<?php "
      "class foo"
      "{"
      "  public $public = 'public';"
      "  public function __wakeup()"
      "  { echo \"__wakeup called\\n\"; return 1; }"
      "}"
      "$foo = unserialize(\"O:3:\\\"foo\\\":1:{s:6:\\\"public\\\";s:6:\\\"public\\\";}\");"
      "var_dump($foo);");

  return true;
}

bool TestCodeRun::TestObjectAssignment() {
  VCR("<?php "
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
  VCR("<?php "
      "class A { var $num; }"
      "function foo() { return new A(); }"
      "foreach(($a=(object)new A()) as $v);"
      "foreach(($a=(object)foo()) as $v);"
      "foreach(($a=new A()) as $v);"
      "$a->num = 1;"
      "print($a->num);");

  return true;
}

bool TestCodeRun::TestObjectPropertyExpression() {
  VCR("<?php "
      "class test {"
      "  function foo() {"
      "    $var = $this->blah->prop->foo->bar = \"string\";"
      "    var_dump($this->blah);"
      "  }"
      "}"
      "$t = new test;"
      "$t->foo();");
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
    COMPARE(a, op, null)                        \
    COMPARE(a, op, array())                     \
    COMPARE(a, op, array(1))                    \
    COMPARE(a, op, array(2))                    \
    COMPARE(a, op, array('1'))                  \
    COMPARE(a, op, array('0' => '1'))           \
    COMPARE(a, op, array('a'))                  \
    COMPARE(a, op, array('a' => 1))             \
    COMPARE(a, op, array('b' => 1))             \
    COMPARE(a, op, array('a' => 1, 'b' => 2))   \
    COMPARE(a, op, 'php')                       \
    COMPARE(a, op, '')                          \

#define COMPARE_OP(op)                                                  \
  VCR("<?php $i = 0; " COMPARE_ALL('1.2', op));                         \
  VCR("<?php $i = 0; " COMPARE_ALL(true, op));                          \
  VCR("<?php $i = 0; " COMPARE_ALL(false, op));                         \
  VCR("<?php $i = 0; " COMPARE_ALL(1, op));                             \
  VCR("<?php $i = 0; " COMPARE_ALL(0, op));                             \
  VCR("<?php $i = 0; " COMPARE_ALL(-1, op));                            \
  VCR("<?php $i = 0; " COMPARE_ALL('1', op));                           \
  VCR("<?php $i = 0; " COMPARE_ALL('0', op));                           \
  VCR("<?php $i = 0; " COMPARE_ALL('-1', op));                          \
  VCR("<?php $i = 0; " COMPARE_ALL(null, op));                          \
  VCR("<?php $i = 0; " COMPARE_ALL(array(), op));                       \
  VCR("<?php $i = 0; " COMPARE_ALL(array(1), op));                      \
  VCR("<?php $i = 0; " COMPARE_ALL(array(2), op));                      \
  VCR("<?php $i = 0; " COMPARE_ALL(array('1'), op));                    \
  VCR("<?php $i = 0; " COMPARE_ALL(array('0' => '1'), op));             \
  VCR("<?php $i = 0; " COMPARE_ALL(array('a'), op));                    \
  VCR("<?php $i = 0; " COMPARE_ALL(array('a' => 1), op));               \
  VCR("<?php $i = 0; " COMPARE_ALL(array('b' => 1), op));               \
  VCR("<?php $i = 0; " COMPARE_ALL(array('a' => 1, 'b' => 2), op));     \
  VCR("<?php $i = 0; " COMPARE_ALL('php', op));                         \
  VCR("<?php $i = 0; " COMPARE_ALL('', op));                            \

bool TestCodeRun::TestComparisons() {
  VCR("<?php var_dump(array(1 => 1, 2 => 1) ==  array(2 => 1, 1 => 1));");
  VCR("<?php var_dump(array(1 => 1, 2 => 1) === array(2 => 1, 1 => 1));");
  VCR("<?php var_dump(array('a'=>1,'b'=> 1) ==  array('b'=>1,'a'=> 1));");
  VCR("<?php var_dump(array('a'=>1,'b'=> 1) === array('b'=>1,'a'=> 1));");

  VCR("<?php $a = '05/17'; $b = '05/18'; var_dump($a == $b);");
  VCR("<?php var_dump('05/17' == '05/18');");

  COMPARE_OP(==);
  COMPARE_OP(===);
  COMPARE_OP(!=);
  COMPARE_OP(<>);
  COMPARE_OP(!==);
  COMPARE_OP(<);
  COMPARE_OP(>);
  COMPARE_OP(<=);
  COMPARE_OP(>=);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// semantics

bool TestCodeRun::TestUnset() {
  VCR("<?php $a = 10; unset($a); var_dump($a);");
  VCR("<?php $a = array(10); "
      "function test() { global $a; unset($a[0]); var_dump($a);}"
      "var_dump($a); test(); var_dump($a);");
  VCR("<?php $a = 10; unset($GLOBALS); var_dump($a);");
  VCR("<?php "
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
  VCR("<?php class A { public $arr;} $obj = new A; $obj->arr[] = 'test';"
      "var_dump($obj->arr); unset($obj->arr); var_dump($obj->arr);");
  return true;
}

bool TestCodeRun::TestReference() {
  VCR("<?php $idxa = array('a' => 1240430476);"
      "$idxa = &$idxa['a'];");

  VCR("<?php $a = array(1, 'a'); $b = $a; "
      "foreach ($b as $k => &$v) { $v = 'ok';} var_dump($a, $b);");

  VCR("<?php $a = array(1, 'test'); $b = $a; $c = &$b[0]; "
      "$c = 10; var_dump($a, $b);");

  // reference expressions
  VCR("<?php $a = &$b; $a = 10; var_dump($b);");
  VCR("<?php $a = 1; $b = $a;  $a = 2; var_dump($b);");
  VCR("<?php $a = 1; $b = &$a; $c = $b; $a = 2; var_dump($b); var_dump($c);");
  VCR("<?php $a = 1; $b = &$a; $b = 2; var_dump($a);");
  VCR("<?php $a = 1; $b = &$a; $c = $b; $b = 2; var_dump($a); var_dump($c);");
  VCR("<?php $a = 1; $c = $b = &$a; $b = 2; var_dump($a); var_dump($c);");
  VCR("<?php $a = 1; $b = &$a; $c = 2; $b = $c; $c = 5; "
      "var_dump($a); var_dump($b); var_dump($c);");
  VCR("<?php $a = 1; $b = &$a; $c = 2; $d = &$c; $b = $d; "
      "var_dump($a); var_dump($b); var_dump($c); var_dump($d);");
  VCR("<?php $a = 1; $b = &$a; $c = 2; $d = &$c; $b = &$d; "
      "var_dump($a); var_dump($b); var_dump($c); var_dump($d);");
  VCR("<?php $a = 10; $b = array(&$a); var_dump($b); "
      "$a = 20; var_dump($b);");
  VCR("<?php $a = array(); $b = 10; $a[] = &$b; $b = 20; var_dump($a);");
  VCR("<?php $a = 10; $b = array('test' => &$a); var_dump($b); "
      "$a = 20; var_dump($b);");
  VCR("<?php $a = array(); $b = 1; $a['t'] = &$b; $b = 2; var_dump($a);");
  VCR("<?php $a = array(1, 2); foreach ($a as $b) { $b++;} var_dump($a);");
  VCR("<?php $a = array(1, 2); foreach ($a as &$b) { $b++;} var_dump($a);");
  VCR("<?php $a = array(1, array(2,3)); "
      "foreach ($a[1] as &$b) { $b++;} var_dump($a);");

  // reference parameters
  VCR("<?php function f(&$a) { $a = 'ok';} "
      "$a = 10; f($a); var_dump($a);");
  VCR("<?php function f(&$a) { $a = 'ok';} "
      "$a = array(); $c = &$a['b']; $c = 'ok'; var_dump($a);");
  VCR("<?php function f(&$a) { $a = 'ok';} "
      "$a = array(); $c = &$a['b']; f($c); var_dump($a);");
  VCR("<?php function f(&$a) { $a = 'ok';} "
      "$a = array(); f($a['b']); var_dump($a);");
  VCR("<?php function f(&$a) { $a = 'ok';} class T { public $b = 10;} "
      "$a = new T(); $a->b = 10; f($a->b); var_dump($a);");
  VCR("<?php function f(&$a) { $a = 'ok';} class T {} "
      "$a = new T(); $a->b = 10; f($a->b); var_dump($a);");
  VCR("<?php function f(&$a) {} "
      "$a = array(); f($a['b']); var_dump($a);");
  VCR("<?php function f(&$a) {} class T {} "
      "$a = new T(); $a->b = 10; f($a->b); var_dump($a);");

  // reference returns
  VCR("<?php $a = 10; function &f() { global $a; return $a;} "
      "$b = &f(); $b = 20; var_dump($a);");
  VCR("<?php function &f() { $a = 10; return $a;} "
      "$b = &f(); $b = 20; var_dump($b);");
  VCR("<?php $a = array(); function &f() { global $a; return $a['b'];} "
      "$b = &f(); $b = 20; var_dump($a);");
  VCR("<?php function &f() { $a = array(); return $a['b'];} "
      "$b = &f(); $b = 20; var_dump($b);");

  // circular references
  //VCR("<?php $a = array('a' => &$a); var_dump($a);");
  //VCR("<?php $a = array('a' => &$a); $b = array($a); var_dump($b);");

  // shallow copy of members (either of arrays or objects)
  VCR("function test($a) { $a[1] = 10; $a['r'] = 20;} "
      "$b = 5; $a = array('r' => &$b); $a['r'] = 6; test($a); var_dump($a);");

  VCR("<?php "
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
  VCRNW("<?php "
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

  return true;
}

bool TestCodeRun::TestDynamicConstants() {
  VCR("<?php function foo($a) { return $a + 10;} define('TEST', foo(10)); "
      "var_dump(TEST);");
  VCR("<?php function foo() { return 15;} "
      "var_dump(TEST); define('TEST', foo()); var_dump(TEST);");
  VCR("<?php if (true) define('TEST', 1); else define('TEST', 2); "
      "var_dump(TEST);");
  VCR("<?php var_dump(TEST); define('TEST', 1); var_dump(TEST); "
      "define('TEST', 2); var_dump(TEST);");
  VCR("<?php if (false) define('TEST', 1); else define('TEST', 2); "
      "var_dump(TEST);");
  VCR("<?php var_dump(defined('TEST')); var_dump(TEST);"
      "define('TEST', 13);"
      "var_dump(defined('TEST')); var_dump(TEST);");
  VCR("<?php define('FOO', BAR); define('BAR', FOO); echo FOO; echo BAR;");
  VCR("<?php define('A', 10); class T { static $a = array(A); } "
      "define('A', 20); var_dump(T::$a);");
  return true;
}

bool TestCodeRun::TestDynamicVariables() {
  // r-value
  VCR("<?php $a = 1; function t() { global $a;$b = 'a'; var_dump($$b);} t();");
  VCR("<?php $a = 1; function t() { $b = 'a'; var_dump($$b);} t();");
  VCR("<?php function t() { $a = 'test'; $b = 'a'; var_dump($$b);} t();");
  VCR("<?php $a = 'test'; $b = 'a'; var_dump($$b);");
  VCR("<?php $a = 1; class A { public function t() { global $a; $b = 'a'; var_dump($$b);}} $obj = new A(); $obj->t();");

  // l-value
  VCR("<?php $a = 'test'; $b = 'a'; $$b = 'ok'; var_dump($a);");
  VCR("<?php $a = 'test'; $b = 'a'; $$b = 10; var_dump($a);");
  VCR("<?php $a = 'd'; var_dump($$a); $$a = 10; var_dump($$a); var_dump($d);");

  // ref-value
  VCR("<?php $a = 'test'; $b = 'a'; $c = &$$b; $c = 10; var_dump($a);");

  // extract
  VCR("<?php extract(array('a' => 'aval')); var_dump($a);");
  VCR("<?php extract(array('a' => 'ok')); $a = 1; var_dump($a);");
  VCR("<?php $a = 1; extract(array('a' => 'ok'), EXTR_SKIP); var_dump($a);");
  VCR("<?php $a = 1; extract(array('a' => 'ok'), EXTR_PREFIX_SAME, 'p');"
      " var_dump($p_a);");
  VCR("<?php extract(array('a' => 'ok'), EXTR_PREFIX_ALL, 'p');"
      " var_dump($p_a);");
  VCR("<?php extract(array('ok'), EXTR_PREFIX_INVALID, 'p'); var_dump($p_0);");
  VCR("<?php $a = null; extract(array('a' => 'ok'), EXTR_IF_EXISTS); var_dump($a);");
  VCR("<?php $a = null; extract(array('a' => 'ok', 'b' => 'no'), EXTR_PREFIX_IF_EXISTS, 'p'); var_dump($p_a); var_dump($b); var_dump($p_b);");
  VCR("<?php $a = 'ok'; extract(array('b' => &$a), EXTR_REFS); $b = 'no'; var_dump($a);");
  VCR("<?php $a = 'ok'; $arr = array('b' => &$a); extract($arr, EXTR_REFS); $b = 'no'; var_dump($a);");

  // compact
  VCR("<?php function test() { $a = 10; $b = 'test'; "
      "  var_dump(compact('ab')); "
      "  var_dump(compact('a', 'ab', 'b')); "
      "  var_dump(compact('a', array('ab', 'b')));"
      "} test(); ");

  // get_defined_vars
  VCR("<?php "
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
  return true;
}

bool TestCodeRun::TestDynamicProperties() {
  VCR("<?php class A { public $a = 1;} "
      "class B extends A { "
      "  public $m = 10;"
      "  public function test() { "
      "    $b = 'a';"
      "    $this->$b = 'test'; var_dump($this->$b); var_dump($this->a);"
      "    $c = &$this->$b; $c = array(1); var_dump($this->a);"
      "  }"
      "} $obj = new B(); $obj->test();");

  VCR("<?php class A { public $a = 1;} class B { public $a = 2;} "
      "$obj = 1; $obj = new A(); var_dump($obj->a);");
  return true;
}

bool TestCodeRun::TestDynamicFunctions() {
  VCR("<?php function test() { print 'ok';} $a = 'Test'; $a();");
  VCR("<?php function test($a) { print $a;} $a = 'Test'; $a('ok');");
  VCR("<?php function test($a, $b) { print $a.$b;} $a = 'Test'; $a('o','k');");

  VCR("<?php function t($a = 'k') { print $a;} "
      "$a = 'T'; $a(); $a('o');");
  VCR("<?php function t($a, $b = 'k') { print $a.$b;} "
      "$a = 'T'; $a('o'); $a('o', 'p');");
  VCR("<?php function t($a, $b = 'k', $c = 'm') { print $a.$b.$c;} "
      "$a = 'T'; $a('o'); $a('o', 'p'); $a('o', 'p', 'q');");

  VCR("<?php function test() { var_dump(func_get_args());} "
      "$a = 'Test'; $a();");
  VCR("<?php function test($a) { var_dump(func_get_args());} "
      "$a = 'Test'; $a(1); $a(1, 2);");
  VCR("<?php function test($a, $b) { var_dump(func_get_args());} "
      "$a = 'Test'; $a(1, 2); $a(1, 2, 3); $a(1, 2, 3, 4);");

  VCR("<?php function t($a = 'k') { var_dump(func_get_args());} "
      "$a = 'T'; $a(); $a('o'); $a('o', 'p'); $a('o', 'p', 'q');");
  VCR("<?php function t($a, $b = 'k') { var_dump(func_get_args());} "
      "$a = 'T'; $a('o'); $a('o', 'p'); $a('o', 'p', 'q');");
  VCR("<?php function t($a, $b = 'k', $c = 'q') { var_dump(func_get_args());} "
      "$a = 'T'; $a('o'); $a('o', 'p'); $a('o', 'p', 'q');");

  VCR("<?php function test(&$a, $b) { $a = 'ok';} $a = 'Test'; "
      "$a($a, 10); print $a;");

  VCR("<?php $a = 'test'; function &test() { global $a; return $a;} "
      " $b = $a(); $b = 'ok'; var_dump($a); "
      " $b = &$a(); $b = 'ok'; var_dump($a);");

  VCR("<?php function test($a, $b) { print $a.$b;} $a = 'Test'; $y = 'kqq'; "
      "$a('o',$y[0]);");
  VCR("<?php function test($a, $b) { print $a.$b;} $a = 'Test'; "
      "$y = array('k','q','q'); $a('o',$y[0]);");

  VCR("<?php "
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

  Option::DynamicInvokeFunctions.insert("test1");
  Option::DynamicInvokeFunctions.insert("test2");
  VCR("<?php "
      "function test1() { print __FUNCTION__;} "
      "function test2() { print __FUNCTION__;} "
      "fb_rename_function('test2', 'test3');"
      "fb_rename_function('test1', 'test2'); test2();"
      );
  Option::DynamicInvokeFunctions.clear();

  return true;
}

bool TestCodeRun::TestDynamicMethods() {
  Option::AllDynamic = true;
  VCR("<?php "
      "class A { public function test() { print 'in A';} } "
      "class B extends A { public function test() { print 'in B';} } "
      "$obj = new B(); "
      "call_user_func_array(array($obj, 'A::test'), array());");

  VCR("<?php $i = 'gi'; $s = 'gs'; class A { "
      "public function &dyn_test(&$a) { global $i; $a = $i; return $i;}} "
      "$obj = new A(); $f = 'dyn_test'; "
      "$c = &$obj->$f($b); var_dump($b); var_dump($c);");
  VCR("<?php $i = 'gi'; $s = 'gs'; class A { "
      "public static function &dyn_test(&$a) "
      "{ global $s; $a = $s; return $s;}} "
      "$f = 'dyn_test'; $e = A::$f($d); var_dump($d); var_dump($e);");
  VCR("<?php class dyn_A{} class B{} $cls = 'dyn_a'; $a = new $cls();");

  VCR("<?php class A { function _test() { print 'ok';} "
      "function __call($name, $args) { $name = '_'.$name; $this->$name();} } "
      "$obj = new A(); $obj->test();");

  VCR("<?php class A { function test($a, $b) { var_dump($a, $b);} } "
      "$m = 'test'; $o = new A();"
      "$ar = array(0,1); $st = 'abc';"
      "$o->$m($ar[0], $st[0]); A::$m($ar[1], $st[1]);");

  vector<string> backup = Option::DynamicMethodPrefixes;
  Option::DynamicMethodPrefixes.push_back("_");
  VCR("<?php class A { function _test() { print 'ok';} "
      "function __call($name, $args) { $name = '_'.$name; $this->$name();} } "
      "$obj = new A(); $obj->test();");
  Option::DynamicMethodPrefixes = backup;

  VCR("<?php "
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

  VCR("<?php "
      "class z {"
      "  function __construct() { echo 'construct'; }"
      "  function z() { echo 'method'; }"
      "}"
      "$z = new z;"
      "$z->z();");
  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestVolatile() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
#ifdef HPHP_NOTE
  VCR("<?php "
      "function foo($a) {"
      "  if (!interface_exists($a)) {"
      "    /*|Volatile|*/interface MyInterface{};"
      "    echo 'no';"
      "  } else {"
      "    echo 'yes';"
      "  }"
      "}"
      "foo('MyInterface');"
      "foo('MyInterface');");
#endif
  return true;
}

bool TestCodeRun::TestProgramFunctions() {
  //VCR("<?php var_dump($_SERVER);");
  VCR("<?php var_dump($argc, count($argv));");
  //VCR("<?php var_dump($_ENV);");

  VCR("<?php function p($a) { print $a;} "
      "register_shutdown_function('p', 'shutdown');");
  return true;
}

bool TestCodeRun::TestCompilation() {
  // overlapped interface
  VCR("<?php interface A {} class B implements A {} "
      "class C extends B implements A {} $obj = new C();");

  // trigraph
  VCR("<?php print '\?\?/';");

  // testing type inference on a re-declared constant
  VCR("<?php if (false) define('a', 'test'); define('a', 5); print $b % a;");

  // testing type inference on a special type casting: PlusOperand -> Double
  VCR("<?php function d() { return '2009';} $y = (d()) + 6;");

  VCR("<?php class A { public static $a = array('a', 'b'); public static function test() { self::$a[] = 'c'; var_dump(self::$a);} } A::test();");

  // \x65D is invalid in C++
  VCR("<?php var_dump(\"[\\x][\\xA][\\x65][\\x65D]\");");
  VCR("\\x65D");

  // float vs. double
  VCR("<?php $a = 1; $a = 'test'; var_dump($a + 2.5);");

  // +/- String
  VCR("<?php $a = -date('w');");

  // ObjectOffset.at()
  VCR("<?php class A { public $a = array('t' => 't');} class B { public $a;} "
      "$a = 1; $a = new A(); $a->a['t'] = true; var_dump($a->a['t']);");

  // Variant % operator
  VCR("<?php $a = date('d') % 10;");
  VCR("<?php $a = 'test'; $a = 1; print $a % 10;");

  // defining a constant after it's used
  VCR("<?php $a = MAX_LATITUDE + 5;"
      "if (12 > -MAX_LATITUDE) define('MAX_LATITUDE', 90); ");

  // toInt64() wrapper
  VCR("<?php print 1 << 32;");

  // !$a is closer in C++
  VCR("<?php if (!$a = true) {}");

  // integer as array element
  VCR("<?php function test($a = 0) { $b = $a; $c = $b[$a];}");

  // String/Array operators
  VCR("<?php function str() { return 'test';} "
      "function test() { var_dump(str() - $a);}");

  // unused variable warning
  VCR("<?php function test() {} function foo() { test($a = 1);}");

  // void return functions
  VCR("<?php function test() {} true ? test() : 1;");

  // uninitialized variables need to be Variant
  VCR("<?php function test() { $a = 0; $a += $b;} test();");

  // VariantOffset = VariantOffset
  VCR("<?php class A {} $a = new A(); $a->a = $a->b = 'test'; var_dump($a);");
  VCR("<?php $a = 1; $a = array(); $a['a'] = $a['b'] = 'test'; var_dump($a);");

  // lval on Variant
  VCR("<?php function test() { return array();} reset(test());");

  // variant.o_lval() needs lval() wrapper
  VCR("<?php class A { public $prop = 1;} class B { public $prop = 5;} "
      "$a = 1; $a = new A(); $a->prop++; var_dump($a->prop);");

  // obj->prop doesn't need lval() wrapper
  VCR("<?php class A { public $prop = 1;} "
      "$a = new A(); $a->prop++; var_dump($a->prop);");

  // ((p_obj)variant)->prop
  VCR("<?php class A { public $prop = 1;} "
      "$a = 1; $a = new A(); $a->prop++; var_dump($a->prop);");

  // unsigned int should never be seen
  VCR("<?php $a = 0xC0000000 & $b;");

  // redefine properties
  VCR("<?php class E extends exception { public $message; public $code;}");

  // redefine static members
  VCR("<?php class A { static $a = 1;} "
      "class B extends A { static $a = 2;} var_dump(B::$a);");

  // overriding function with assignment on parameters
  VCR("<?php class A { "
      "function __call($a, $b) { $b = 'a'; $b = 1; "
      "var_dump($a, $b[0], $b[1]);}} "
      "$obj = new A(); $a = 1; $b = 'a'; $b = 2; $obj->test($a, $b);");

  // method->method
  VCR("<?php class A { public function getA() { return $this;} "
      "public function test() { var_dump('test');}} "
      "class B { public function getA() {} public function test(){}}"
      "$obj = new A(); $obj->getA()->test();"
      );

  // constructor fallback
  VCR("<?php class A { function __construct($a) { var_dump($a);} } "
      "class B extends A {} "
      "$a = new B('test');");

  // prop->method
  VCR("<?php class A { function test() {}} class B { public $b;} "
      "class C { function test() {}} "
      "$a = 'test'; $a = new B(); $a->b = new A(); $a->b->test();");

  // testing code generation order
  VCR("<?php "
      "$global = B::CLASS_CONSTANT; "
      "$another = test2($global); "
      "define('CONSTANT', test2('defining')); "
      //"test();"
      //"function test($a = CONSTANT) { test2($a);} "
      "function test2($a) { var_dump($a); return 12345;} "
      "class A extends B {} "
      "class B { const CLASS_CONSTANT = 1;} ");

  // $_SERVER is already defined
  VCR("<?php $_SERVER = array('test' => 1); var_dump($_SERVER);");
  VCR("<?php $GLOBALS['_SERVER'] = array('test' => 1); var_dump($_SERVER);");

  // class constant as default
  VCR("<?php "
      "class A { const C = 123; static function t($a = B::C) {} } A::t();"
      "class B { const C = 456; static function t($a = A::C) {} } B::t();");

  // base virtual function prototype
  VCR("<?php class T { function __toString() { return 123;}} "
      "$obj = new T(); var_dump($obj);");

  // void wrapper
  VCR("<?php function test() {} var_dump(test()); $a = test();");

  // ambiguous overload
  VCR("<?php $a = 'test'; $a = 123; switch ($a) { case -1: var_dump($a);}");

  // [][]
  VCR("<?php $a['a']['b'] = 'test'; var_dump($a['a']['b']);");

  // testing Variant -> p_class conversion
  VCR("<?php class A { function test(A $a) { $a->foo();} "
      "function foo() { print 'foo';}}");

  VCR("<?php "
      "class A { function f($a) {} }"
      "$obj = new A;"
      "$obj->f(date('m/d/y H:i:s', 123456789));"
      "$v = date(\"m\",123456789)+1;");

  // no side effect optimization met if() short
  VCR("<?php if ($a) $a == 0;");

  // */ and // in default argument
  VCR("<?php "
      "function foo($p1=\"/.*/\", $p2=\"//\") {"
      "  var_dump($p1, $p2);"
      "}"
      "foo();");
  return true;
}

bool TestCodeRun::TestReflection() {
  VCR("<?php class A { public static function test() { print 'ok';}}"
      "var_dump(is_callable('A::test'));"
      "var_dump(function_exists('A::test'));");

  VCR("<?php function test($a) { return 'ok'.$a;}"
      "var_dump(function_exists('TEst')); "
      "var_dump(is_callable('teSt'));"
      "var_dump(call_user_func('teST', 'blah')); "
      "var_dump(call_user_func_array('teST', array('blah'))); "
      );

  VCR("<?php class B { public function f($a) { return 'ok'.$a;}} "
      "class A extends B { public $p = 'g';} "
      "$obj = new A(); "
      "var_dump(get_class($obj)); "
      "var_dump(strtolower(get_parent_class($obj))); " // glitch
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

  VCR("<?php class A { public static function f($a) { return 'ok'.$a;}} "
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

  VCR("<?php "
      "class A { function foo() {} }"
      "class B extends A { function bar() {}}"
      "var_dump(get_class_methods(new B()));");

  VCR("<?php "
      "interface A { function foo(); }"
      "abstract class B implements A { function bar() {}}"
      "var_dump(get_class_methods('B'));");

  VCR("<?php "
      "interface I1 { function ifoo2(); function ifoo1(); }"
      "interface I2 { function ifoo4(); function ifoo3(); }"
      "class A { function foo() {} function foo2() {} }"
      "abstract class B extends A implements I1, I2 { function bar() {}}"
      "abstract class C extends A implements I2, I1 { function bar() {}}"
      "var_dump(get_class_methods('B'));"
      "var_dump(get_class_methods('C'));"
      );

  VCR("<?php class A { static $a = 10; public $b = 20;}"
      "$obj = new A(); var_dump(get_object_vars($obj));");

  return true;
}

bool TestCodeRun::TestReflectionClasses() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
      "class z {"
      "  const foo = 10;"
      "}"
      "class c {"
      "  const bar = z::foo;"
      "}"
      "var_dump(c::bar);"
      "$r = new ReflectionClass('c');"
      "var_dump($r->getConstant(\"bar\"));");
  VCR("<?php "
      "class fOo {}"
      "interface ioO {}"
      "$c = new ReflectionClass('Foo');"
      "$i = new ReflectionClass('Ioo');"
      "var_dump($c->getFileName() !== '');"
      "var_dump($i->getFileName() !== '');");

  return true;
}

bool TestCodeRun::TestErrorHandler() {
  VCR("<?php function handler($code, $msg) { "
      "  var_dump(strpos($msg, 'system error') !== false); return true;"
      "} "
      "set_error_handler('handler');"
      "function a() {} set_error_handler('a');restore_error_handler();"
      "trigger_error('system error'); "
      );

  VCR("<?php function handler($code, $msg) { "
      "  var_dump(strpos($msg, 'system error') !== false); return true;"
      "} "
      "set_error_handler('handler');"
      "user_error('system error'); "
      );

  VCR("<?php function handler($e) { "
      "  var_dump(strpos((string)$e, 'bomb') !== false); return true;"
      "} "
      "set_exception_handler('handler');"
      "function a() {} set_exception_handler('a');restore_exception_handler();"
      "throw new Exception('bomb'); "
      );

  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestAssertOptions() {
  VCR("<?php "
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
  VCR("<?php var_dump(pack('nvc*', 0x1234, 0x5678, 65, 66));");
  VCR("<?php var_dump(unpack('nfrist/vsecond/c2chars', "
      "pack('nvc*', 0x1234, 0x5678, 65, 66)));");
  return true;
}

bool TestCodeRun::TestSuperGlobals() {
  VCR("<?php function foo() { "
      "file_get_contents('http://example.com');"
      "var_dump(empty($http_response_header));"
      "} foo();");
  return true;
}

bool TestCodeRun::TestGlobalStatement() {
  VCR("<?php "
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

  VCR("<?php "
      "$a = 0;"
      "function test() {"
      "  $a = 1;"
      "  if (true) global $a;"
      "  $a = 2;"
      "}"
      "test();"
      "print \"$a\\n\";"
      );

  VCR("<?php "
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

  VCR("<?php "
      "function test() {"
      "  if (true) {"
      "    global $a;"
      "    $a = 10;"
      "  }"
      "}"
      "test();"
      "var_dump($a);"
      );

  return true;
}

bool TestCodeRun::TestStaticStatement() {
  VCR("<?php "
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

  VCR("<?php "
      "function test() {"
      "  $static_var = 3;"
      "  echo $static_var;"
      "  static $static_var;"
      "  $static_var ++;"
      "  echo $static_var;"
      "}"
      "test();"
     )

  VCR("<?php "
      "class A { static function test() {"
      "  $static_var = 3;"
      "  echo $static_var;"
      "  static $static_var;"
      "  $static_var ++;"
      "  echo $static_var;"
      "} }"
      "A::test();"
     )

  VCR("<?php "
      "  $static_var = 1;"
      "  echo $static_var . \"\\n\";"
      "  static $static_var;"
      "  echo $static_var . \"\\n\";"
      "  $static_var ++;"
      "  echo $static_var . \"\\n\";"
     )

  VCR("<?php "
      "  $static_var = 1;"
      "  global $static_var;"
      "  echo $static_var . \"\\n\";"
      "  $static_var --;"
      "  echo $static_var . \"\\n\";"
     )

  VCR("<?php "
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

  VCR("<?php "
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

  VCR("<?php "
      "static $static_var;"
      "echo $static_var . \"\\n\";"
      "$static_var = 1;"
      "echo $static_var . \"\\n\";"
     )

  VCR("<?php "
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

  VCR("<?php "
      "if (false) {"
      "  static $static_var = +3;"
      "}"
      "echo $static_var . \"\\n\";"
      "$static_var = 4;"
      "echo $static_var . \"\\n\";"
     )

  VCR("<?php "
      "echo $static_var . \"\\n\";"
      "static $static_var = 4;"
      "echo $static_var . \"\\n\";"
      );

  VCR("<?php "
      "static $a = 5;"
      "echo $a . \"\\n\";"
      "global $a;"
      "echo $a . \"\\n\";"
      );

  VCR("<?php "
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

  VCR("<?php "
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

  VCR("<?php "
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

  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestIfStatement() {
  VCR("<?php "
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

  VCR("<?php "
      "if (true) {"
      "  function foo() { echo \"foo\\n\"; }"
      "} else if (false) {"
      "  function bar() { echo \"bar\\n\"; }"
      "}");

  VCR("<?php "
      "if (true) {"
      "  function foo() { echo \"foo\\n\"; }"
      "} elseif (false) {"
      "  function bar() { echo \"bar\\n\"; }"
      "}");

  return true;
}

bool TestCodeRun::TestBreakStatement() {
  VCR("<?php "
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
  VCR("<?php "
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

  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestAdd() {
  VCR("<?php "
      "printf(\"%s\\n\", 30 + 30);"
      "printf(\"%s\\n\", \"30\" + 30);"
      "printf(\"%s\\n\", 30 + \"30\");"
      "printf(\"%s\\n\", \"30\" + \"30\");"
      );

  VCR("<?php "
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

  VCR("<?php "
      "var_dump(1.7976931348623157e+308 + 1.7976931348623157e+308);"
      );
  return true;
}

bool TestCodeRun::TestMinus() {
  VCR("<?php "
      "printf(\"%s\\n\", 30 - 30);"
      "printf(\"%s\\n\", \"30\" - 30);"
      "printf(\"%s\\n\", 30 - \"30\");"
      "printf(\"%s\\n\", \"30\" - \"30\");"
      );
  VCR("<?php "
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
  VCR("<?php "
      "printf(\"%s\\n\", 30 * 30);"
      "printf(\"%s\\n\", \"30\" * 30);"
      "printf(\"%s\\n\", 30 * \"30\");"
      "printf(\"%s\\n\", \"30\" * \"30\");"
      );
  VCR("<?php "
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
  VCR("<?php "
      "printf(\"%s\\n\", 30 / 30);"
      "printf(\"%s\\n\", \"30\" / 30);"
      "printf(\"%s\\n\", 30 / \"30\");"
      "printf(\"%s\\n\", \"30\" / \"30\");"
      );
  VCR("<?php "
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
  VCR("<?php "
      "printf(\"%s\\n\", 30 % 30);"
      "printf(\"%s\\n\", \"30\" % 30);"
      "printf(\"%s\\n\", 30 % \"30\");"
      "printf(\"%s\\n\", \"30\" % \"30\");"
      );
  VCR("<?php "
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
  VCR("<?php "
      "$a = 1 % 9223372036854775807;"
      "var_dump($a);");

  return true;
}

bool TestCodeRun::TestOperationTypes() {
  VCR("<?php "
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
  VCR("<?php "
      "var_dump(1 / 1.7976931348623157e+308 || false);");
/*
  Fails under release build due to g++ optimization
  VCR("<?php "
      "var_dump((1.7976931348623157e+308 + 1.7976931348623157e+308) << 0);");
*/
  VCR("<?php "
      "function foo() {"
      "  return '1';"
      "}"
      "function bar() {"
      "  $a = 1;"
      "  $a += foo(1);"
      "  var_dump($a);"
      "  $b = 1;"
      "  $b -= foo(1);"
      "  var_dump($b);"
      "}"
      "bar();");

  return true;
}

#define UNARY_OP(op) \
  VCR("<?php " \
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
  #op"((string)\"1.7e+319\");" \
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
  #op"($a);")

bool TestCodeRun::TestUnaryOperators() {
  UNARY_OP(var_dump);
  return true;
}

bool TestCodeRun::TestSilenceOperator() {
  VCR("<?php "
      "@define( 'MARKDOWN_EMPTY_ELEMENT_SUFFIX',  \" />\");");
  return true;
}

bool TestCodeRun::TestPrint() {
  UNARY_OP(echo);
  UNARY_OP(print);
  UNARY_OP(print_r);
  UNARY_OP(var_export);
  UNARY_OP(serialize);
  return true;
}

bool TestCodeRun::TestLocale() {
  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestGetClass() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestRedeclaredClasses() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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

  VCR("<?php "
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
  VCR("<?php "
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

  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestClone() {
  VCR("<?php "
      "class A {"
      "  public $foo = 0;"
      "  public $fooref = 1;"
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestEvalOrder() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
      "class a {"
      "  function r(&$x) {"
      "    $x = 20;"
      "  }"
      "}"
      "function id($x) { return $x; }"
      "$a = new a();"
      "id($a)->r($x);"
      "var_dump($x);");

  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestGetObjectVars() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  return true;
}

bool TestCodeRun::TestSerialization() {
  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestJson() {
  VCR("<?php "
      "$a = array();"
      "$a[] = &$a;"
      "var_dump($a);"
      "var_dump(json_encode($a));");
  VCR("<?php "
      "$a = array(1.23456789e+34, 1E666, 1E666/1E666);"
      "$e = json_encode($a);"
      "var_dump($a);");
  VCR("<?php "
      "var_dump(json_decode(\"[\\\"a\\\",1,true,false,null]\", true));");
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestThrift() {
  VCR(
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
      "    return substr($this->buff, $pos, $n);"
      "    $pos += $n;"
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
      "                  )"
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
      "  var_dump($v1);"
      "  thrift_protocol_write_binary($p, 'foomethod', 1, $v1, 20, true);"
      "  var_dump(md5($p->getTransport()->buff));"
      "  var_dump(thrift_protocol_read_binary($p, 'TestStruct', true));"
      "}"
      "test();");
  return true;
}

bool TestCodeRun::TestExit() {
  VCR("<?php "
      "function foo() { return false; }"
      "foo() or die(\"foobar\");");
  VCR("<?php "
      "function foo() { return false; }"
      "foo() or exit(\"foobar\");");

  return true;
}

bool TestCodeRun::TestCreateFunction() {
  VCR("<?php var_dump(array_filter(array(1, 1003, 34, 5006), "
      "create_function('$x', 'return $x > 1000;')));");
  VCR("<?php var_dump(array_filter(array(1, 1003, 34, 5006), "
      "create_function('$x', 'return '.'$x > 1000;')));");
  return true;
}

bool TestCodeRun::TestConstructorDestructor() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
      "echo \"a\" . \"b\" . \"c\" . \"d\" . \"e\";"
      "echo 'a' . 'b' . 'c' . 'd' . 'e';"
      "echo 'a' . \"b\" . \"c\" . \"d\" . 'e';"
      "echo '\"a' . \"\\\"b\" . \"\\'c\" . \"\\'d\" . '\\\"e';"
      "echo 1 . 2 . 3 . 4 . 5;"
      "echo 1 . '2' . '3' . 4 . 5;"
      "echo 1 . \"2\" . \"3\" . 4 . 5;");
  VCR("<?php "
      "$v = \"c\";"
      "echo \"a\" . \"b\" . $v . \"d\" . \"e\";"
      "echo \"a\" . \"b\" . $v . \"d\" . \"e\" . $v . \"f\" . \"g\";");
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
      "$v = 1;"
      "echo $v . b'a' . b\"b\" . `ls \\055\\144 \\x2ftmp`;"
      "echo b'a' . b\"b\" . `ls \\055\\144 \\x2ftmp` . $v;");
  VCR("<?php "
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
  VCR("<?php "
      "function foo() { return \"hello\" . \"\\0\" . \"world\n\"; }"
      "function bar() {"
      "  $s = foo();"
      "  echo $s;"
      "}"
      "bar();");
  VCR("<?php "
      "define('FOO'.'BAR', 1);"
      "echo FOOBAR;");
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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

  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestConstant() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
      "define('FOO', \"\\n\");"
      "define('BAR', \"\\r\");"
      "var_dump(PHP_EOL);"
      "var_dump(FOO);"
      "var_dump(BAR);");

  return true;
}

bool TestCodeRun::TestClassConstant() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
      "define('FOO', 3);"
      "function foo($a = FOO) {"
      "  echo $a;"
      "}"
      "foo();");
  VCR("<?php "
      "class c {"
      "  function foo($x = self::BLAH) {}"
      "}");

  return true;
}

bool TestCodeRun::TestConstantFunction() {
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
      "if (defined('FOO')) echo 'defined'; else echo 'undefined';");
  VCR("<?php "
      "define('FOO', 1);"
      "if (defined('FOO')) echo 'defined'; else echo 'undefined';");
  VCR("<?php "
      "echo FOO;"
      "if (defined('FOO')) echo 'defined'; else echo 'undefined';");
#ifdef HPHP_NOTE
  VCR("<?php "
      "$a = 'M_PI';"
      "if (defined($a)) {"
      "  var_dump(M_PI);"
      "}"
      "$a = 'DEFINED';"
      "define('DEFINED', '12');"
      "if (defined($a)) {"
      "  var_dump(DEFINED);"
      "}"
      "$a = 'UNDEFINED';"
      "if (defined($a)) {"
      "  var_dump(UNDEFINED);"
      "}"
      "$a = 'NON_SCALAR';"
      "if (defined($a)) {"
      "  var_dump(NON_SCALAR);"
      "}"
      "define('NON_SCALAR', php_uname());"
      "if (defined($a)) {"
      "  var_dump(NON_SCALAR);"
      "}"
      "$a = 'REDECLARED';"
      "if (defined($a)) {"
      "  var_dump(REDECLARED);"
      "}"
      "$i = 1;"
      "for ($i = 0; $i < 2; $i++) {"
      "  if ($i > 0) {"
      "    define('REDECLARED', 1);"
      "  } else if ($i < 0) {"
      "    define('REDECLARED', -1);"
      "  }"
      "}"
      "if (defined($a)) {"
      "  var_dump(REDECLARED);"
      "}"
      "$a = 'DYNAMIC';"
      "if (defined($a)) {"
      "  var_dump(DYNAMIC);"
      "}"
      "define(/*|Dynamic|*/ 'DYNAMIC', 13);"
      "if (defined($a)) {"
      "  var_dump(DYNAMIC);"
      "}");
#endif
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestAssignment() {
  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestEvaluationOrder() {
  VCR("<?php var_dump($v++, $v++);");
  VCR("<?php var_dump($v, $v = 0);");
  return true;
}

bool TestCodeRun::TestSimpleXML() {
  VCR("<?php $doc = simplexml_load_string('<?xml version=\"1.0\"?><root xmlns:foo=\"http://example.com\"><foo:b1>c1</foo:b1><foo:b2>c2</foo:b2><foo:b2>c3</foo:b2></root>'); $foo_ns_bar = $doc->children('http://example.com');"
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

  VCR("<?php function printElement($el, $indent='') {"
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

  VCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node a=\"b\"><subnode attr1=\"value1\" attr2=\"value2\">test</subnode><subnode><subsubnode>test</subsubnode></subnode><test>v</test></node>');"
      "var_dump((array)$a->attributes());"
      "var_dump((string)$a->subnode[0]);"
      "var_dump((string)$a->subnode[0]['attr1']);"
      "var_dump((string)$a->subnode[1]['subsubnode']);"
      "var_dump((string)$a->subnode[1]->subsubnode);"
      "var_dump((string)$a->test);"
      "var_dump((array)$a->subnode[0]->attributes());"
      "var_dump((array)$a->subnode[1]->attributes());"
      "var_dump($a->asxml());"
      "var_dump((array)$a->addchild('newnode', 'newvalue'));"
      "$a->addattribute('newattr', 'newattrvalue');"
      "var_dump($a->asxml());"
      "var_dump((array)$a->attributes());"
      "var_dump((array)$a->newnode);"
      "var_dump($a->getname());"
      "var_dump((array)$a->children()->subnode[0]->subsubnode);"
      "$nodes = $a->xpath('//node/subnode');"
      "var_dump((string)$nodes[1]->subsubnode);"
      "$nodes = $nodes[1]->xpath('subsubnode');"
      "var_dump((string)$nodes[0]);"
      );

  VCR("<?php $a = new SimpleXMLElement('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');"
      "var_dump((array)($a->subnode->subsubnode));"
      "var_dump((string)($a->subnode->subsubnode));"
      );

  VCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');"
      "var_dump((array)($a->subnode->subsubnode));"
      "var_dump((string)($a->subnode->subsubnode));"
      );
  VCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode>test</subsubnode></subnode></node>');"
      "var_dump((string)($a->subnode->subsubnode['0']));"
      "var_dump((string)($a->subnode->subsubnode[0]));"
      );

  VCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode attr1=\"value1\">test</subnode></node>');"
      "var_dump((string)($a->subnode['attr1']));"
      );
  VCR("<?php $a = simplexml_load_string('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode attr1=\"value1\">test</subsubnode></subnode></node>');"
      "var_dump((string)($a->subnode->subsubnode['attr1']));"
      );

  VCR("<?php $a = new SimpleXMLElement('<?xml version=\"1.0\" encoding=\"utf-8\"?><node><subnode><subsubnode><sssnode>test</sssnode></subsubnode></subnode></node>');"
      "var_dump((string)($a->subnode->subsubnode->sssnode));"
      );
  VCR("<?php "
      "$post_xml = '<?xml version=\"1.0\" encoding=\"utf-16\"?><ScanResults version=\"1.0\"><scannedItem itemType=\"5\" itemSize=\"1079856\" "
      "itemName=\"C:\\\\Program Files\\\\VMware\\\\VMware Tools\\\\VMwareUser.exe\" "
      "IsScanned=\"1\" IsInfected=\"0\" ObjectSummary=\"0\" "
      "ScanError=\"0\"/></ScanResults>';"
      "$xml = new SimpleXMLElement($post_xml);"
      "foreach ($xml->scannedItem as $item) {"
      "  echo $item['itemName'] . \"\\n\";"
      "}");

  return true;
}

bool TestCodeRun::TestFile() {
  VCR("<?php "
      "$gif = imagecreatefromgif('http://www.php.net/images/php.gif');"
      "imagegif($gif);"
      "imagedestroy($gif);");
  VCR("<?php "
      "file_put_contents(\"/tmp/temp.txt\","
      "                  \"put this in the txt file\\n\");"
      "$txt = file_get_contents(\"/tmp/temp.txt\");"
      "echo $txt;"
      "file_put_contents(\"compress.zlib:///tmp/temp.zip\","
      "                  \"put this in the zip file\\n\");"
      "$zip = file_get_contents(\"compress.zlib:///tmp/temp.zip\");"
      "echo $zip;");
  VCR("<?php "
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
  VCR("<?php "
      "var_dump(filetype('test/test_ext_file2.tmp'));"
      "var_dump(is_link('test/test_ext_file2.tmp'));"
      "$a = lstat('test/test_ext_file2.tmp');"
      "var_dump($a['mtime']);");
  return true;
}

bool TestCodeRun::TestDirectory() {
  VCR("<?php "
      "$d = dir(\"/tmp/\");"
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
  VCR("<?php class A { function __construct() {}} $obj = new A(10);");

  // make sure foo() is still called
  VCR("<?php function foo($a) { print $a;} "
      "class A { function __construct() {}} $obj = new A(foo(10));");

  // make sure 1st parameter is corrected passed in
  VCR("<?php function foo($a) { print $a;} function bar($a) { return $a;}"
      " foo('ok', bar('bad'));");
  // Too many args
  VCR("<?php "
      "function foo($x) {}"
      "function z() {"
      "  $yay = 1;"
      "  $snarf = 2;"
      "  foo(1,foo(1), $yay,$snarf);"
      "}"
      "z();");

  return true;
}

bool TestCodeRun::TestConstructor() {
  // class-name constructors should not be renamed
  VCR("<?php class A { function a() { echo \"A\n\"; }}"
      "function test() { $obj = new A(); $obj->a(); }"
      "test();");
  // __construct takes priority
  VCR("<?php "
      "class A {"
      "  function a() { echo \"A\n\"; }"
      "  function __construct() { echo \"cons\n\"; }"
      "} "
      "function test() { $obj = new A(); $obj->a(); } "
      "test();");
  return true;
}

bool TestCodeRun::TestTernary() {
  VCR("<?php $t = true; $a = $t ? \"hello\" : \"world\"; var_dump($a);");
  VCR("<?php $f = false; $a = $f ? 5 : \"hello\"; var_dump($a);");
  VCR("<?php $t = true; $a = $t ? \"hello\" : null; var_dump($a);");
  VCR("<?php "
      "function memcache_init_split_vars() {"
      "  global $_SERVER;"
      "  global $MEMCACHED_SPLIT_HASH;"
      "  $MEMCACHED_SPLIT_HASH ="
      "    crc32(empty($_SERVER['SERVER_ADDR']) ? php_uname('n')"
      "                                         : $_SERVER['SERVER_ADDR']);"
      "}");
  VCR("<?php "
      "function f() {} function g() {} "
      "$t = true;"
      "$a = $t ? f() : g();"
      "var_dump($a);");
  VCR("<?php function test($a) { $b = $a + 1 == 5 ? 5 : 7; } test(4);");
  VCR("<?php $t = true; $f = false;"
      "$a = $t ? null : ($f ? \"hello\" : \"world\");");
  VCR("<?php $t = true; $a = $t ? \"\" : \"a\" . $t . \"b\";");
  VCR("<?php "
      "function add_cssclass($add, $class) {"
      "  $class = empty($class) ? $add : $class .= ' ' . $add;"
      "  return $class;"
      "}"
      "add_cssclass('test', $a);");
  VCR("<?php "
      "$a = 123;"
      "echo $a ? @mysql_data_seek(null, null) : false;");
  return true;
}

bool TestCodeRun::TestUselessAssignment() {
  VCR("<?php "
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
  VCR("<?php "
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

  return true;
}

bool TestCodeRun::TestTypes() {
  VCR("<?php "
      "function foo($m, $n) {"
      "  $offset_change = 10;"
      "  $offset_change -= strlen($m) - strlen($n);"
      "  var_dump($offset_change);"
      "}"
      "foo('abc', 'efg');");
  VCR("<?php "
      "function p(array $i = null) {"
      "  var_dump($i);"
      "  $i = array();"
      "}"
      "p();"
      "function q() {"
      "  p(null);"
      "}");

  return true;
}

bool TestCodeRun::TestSwitchStatement() {
  VCR("<?php class A {} $a = new A();"
      "switch ($a) { "
      "case 'foo': "
      "default:"
      "}");
  return true;
}

bool TestCodeRun::TestExtImage() {
  VCR("<?php "
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
  VCR("<?php "
      "header ('Content-type: image/png');"
      "$im = imagecreatetruecolor(120, 20);"
      "$text_color = imagecolorallocate($im, 233, 14, 91);"
      "imagestring($im, 1, 5, 5,  'A Simple Text String', $text_color);"
      "imagepng($im);"
      "imagedestroy($im);");
  VCR("<?php "
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
  VCR("<?php "
      "$image = imagecreatefromgif('test/images/php.gif');"
      ""
      "$emboss = array(array(2, 0, 0), array(0, -1, 0), array(0, 0, -1));"
      "imageconvolution($image, $emboss, 1, 127);"
      ""
      "header('Content-Type: image/png');"
      "imagepng($image, null, 9);");
  VCR("<?php "
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
  VCR("<?php "
      "// File and new size"
      "$filename = 'test/images/simpletext.jpg';"
      "$percent = 0.5;"
      "// Content type"
      "header('Content-type: image/jpeg');"
      "// Get new sizes"
      "list($width, $height) = getimagesize($filename);"
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
      "$png = imagecreatefrompng('test/images/smile.happy.png');"
      ""
      "// Save the image as a GIF"
      "imagegif($png, '/tmp/php.gif');"
      ""
      "// Free from memory"
      "imagedestroy($png);");
  VCR("<?php "
      "// Create an image instance"
      "$im = imagecreatefromgif('test/images/php.gif');"
      ""
      "// Enable interlancing"
      "imageinterlace($im, true);"
      ""
      "// Save the interfaced image"
      "imagegif($im, './php_interlaced.gif');"
      "imagedestroy($im);");
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
  VCR("<?php "
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
      "foo($text, $fsize, false);");
  VCR("<?php "
      "for ($i = 0; $i < 100000; $i++) {"
      "  $str =  exif_tagname($i);"
      "  if ($str) {"
      "    echo \"$i: $str\\n\";"
      "  }"
      "}");
  VCR("<?php "
      "$filename = 'test/images/test1pix.jpg';"
      "$image = exif_thumbnail($filename, $width, $height, $type);"
      "if ($image!==false) {"
      "  header('Content-type: ' .image_type_to_mime_type($type));"
      "  var_dump($width, $height, $type);"
      "} else {"
      "  echo 'No thumbnail available';"
      "}");
  return true;
}

// please leave this unit test at last for debugging ad hoc code
bool TestCodeRun::TestAdHoc() {
  return true;
}
