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

#include <test/test_parser_stmt.h>
#include <compiler/parser/parser.h>
#include <compiler/code_generator.h>
#include <compiler/statement/statement_list.h>
#include <compiler/analysis/analysis_result.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

bool TestParserStmt::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestFunctionStatement);
  RUN_TEST(TestClassStatement);
  RUN_TEST(TestInterfaceStatement);
  RUN_TEST(TestClassVariable);
  RUN_TEST(TestClassConstant);
  RUN_TEST(TestMethodStatement);
  RUN_TEST(TestStatementList);
  RUN_TEST(TestBlockStatement);
  RUN_TEST(TestIfBranchStatement);
  RUN_TEST(TestIfStatement);
  RUN_TEST(TestWhileStatement);
  RUN_TEST(TestDoStatement);
  RUN_TEST(TestForStatement);
  RUN_TEST(TestSwitchStatement);
  RUN_TEST(TestCaseStatement);
  RUN_TEST(TestBreakStatement);
  RUN_TEST(TestContinueStatement);
  RUN_TEST(TestReturnStatement);
  RUN_TEST(TestGlobalStatement);
  RUN_TEST(TestStaticStatement);
  RUN_TEST(TestEchoStatement);
  RUN_TEST(TestUnsetStatement);
  RUN_TEST(TestExpStatement);
  RUN_TEST(TestForEachStatement);
  RUN_TEST(TestCatchStatement);
  RUN_TEST(TestTryStatement);
  RUN_TEST(TestThrowStatement);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestParserStmt::TestFunctionStatement() {
  V("<?php function test() {}",
    "function test() {\n}\n");

  V("<?php function test() {return 0;}",
    "function test() {\n  return 0;\n}\n");

  V("<?php function test() {return 0;return 1;}",
    "function test() {\n  return 0;\n  return 1;\n}\n");

  V("<?php function &test() {return 0;}",
    "function &test() {\n  return 0;\n}\n");

  V("<?php function test($param) {return 0;}",
    "function test($param) {\n  return 0;\n}\n");

  V("<?php function test ( $param1 ,  $param2 ) {return 0;}",
    "function test($param1, $param2) {\n  return 0;\n}\n");

#ifdef HPHP_NOTE
  {
    const char *input = "<?php /*|MasterCopy|*/ //note\nfunction test() {}";
    AnalysisResultPtr ar(new AnalysisResult());
    StatementListPtr tree = Parser::parseString(input, ar);
    if (!(*tree)[0]->hasHphpNote("MasterCopy")) {
      return false;
    }
  }
#endif

  return true;
}

bool TestParserStmt::TestClassStatement() {
  V("<?php class Test {}",
    "class test {\n}\n");

  V("<?php class Test { public $data;}",
    "class test {\n  public $data = null;\n}\n");

  V("<?php abstract class Test { public $data;}",
    "abstract class test {\n  public $data = null;\n}\n");

  V("<?php final class Test { public $data;}",
    "final class test {\n  public $data = null;\n}\n");

  V("<?php class Test extends Base { public $data;}",
    "class test extends base {\n  public $data = null;\n}\n");

  V("<?php class Test implements Base { public $data;}",
    "class test implements base {\n  public $data = null;\n}\n");

  V("<?php class Test implements Base,Base2 { public $data;}",
    "class test implements base, base2 {\n  public $data = null;\n}\n");

  V("<?php class Test { public $data; function test() {}}",
    "class test {\n  public $data = null;\n  public function test() {\n  }\n}\n");

  V("<?php class Test { private $data; function test() {}}",
    "class test {\n  private $data = null;\n  public function test() {\n  }\n}\n");

  V("<?php class Test { function test() {}}",
    "class test {\n  public function test() {\n  }\n}\n");

  return true;
}

bool TestParserStmt::TestInterfaceStatement() {
  V("<?php interface Test {}",
    "interface test {\n}\n");

  V("<?php interface Test extends Base {}",
    "interface test extends base {\n}\n");

  V("<?php interface Test extends Base,Base1 {}",
    "interface test extends base, base1 {\n}\n");

  V("<?php interface Test { function test();}",
    "interface test {\n  public function test();\n}\n");

  V("<?php interface Test { function test(); function test2();}",
    "interface test {\n  public function test();\n  public function test2();\n}\n");

  return true;
}

bool TestParserStmt::TestClassVariable() {
  V("<?php class Test { public $data;}",
    "class test {\n  public $data = null;\n}\n");

  V("<?php class Test { protected $data;}",
    "class test {\n  protected $data = null;\n}\n");

  V("<?php class Test { private $data;}",
    "class test {\n  private $data = null;\n}\n");

  V("<?php class Test { static $data;}",
    "class test {\n  static $data = null;\n}\n");

  V("<?php class Test { abstract $data;}",
    "class test {\n  abstract $data = null;\n}\n");

  V("<?php class Test { final $data;}",
    "class test {\n  final $data = null;\n}\n");

  V("<?php class Test { private static $data;}",
    "class test {\n  private static $data = null;\n}\n");

  V("<?php class Test { private static $data=2;}",
    "class test {\n  private static $data = 2;\n}\n");

  V("<?php class Test { var $data,$data2;}",
    "class test {\n  public $data = null, $data2 = null;\n}\n");

  V("<?php class Test { var $data,$data2=2;}",
    "class test {\n  public $data = null, $data2 = 2;\n}\n");

  V("<?php class Test { var $data=2,$data2;}",
    "class test {\n  public $data = 2, $data2 = null;\n}\n");

  V("<?php class Test { var $data=2,$data2=3;}",
    "class test {\n  public $data = 2, $data2 = 3;\n}\n");

  return true;
}

bool TestParserStmt::TestClassConstant() {
  V("<?php class Test { const DATA=1;}",
    "class test {\n  const DATA = 1;\n}\n");

  V("<?php class Test { const DATA=1,DATA2=2;}",
    "class test {\n  const DATA = 1, DATA2 = 2;\n}\n");

  return true;
}

bool TestParserStmt::TestMethodStatement() {
  V("<?php class A {function test();}",
    "class a {\n  public function test();\n}\n");
  V("<?php class A {function test() {}}",
    "class a {\n  public function test() {\n  }\n}\n");
  V("<?php class A {function test() {return 0;}}",
    "class a {\n  public function test() {\n    return 0;\n  }\n}\n");
  V("<?php class A {function test() {return 0;return 1;}}",
    "class a {\n  public function test() {\n    return 0;\n    return 1;\n  }\n}\n");
  V("<?php class A {function &test() {return 0;}}",
    "class a {\n  public function &test() {\n    return 0;\n  }\n}\n");
  V("<?php class A {function test($param) {return 0;}}",
    "class a {\n  public function test($param) {\n    return 0;\n  }\n}\n");
  V("<?php class A {function test ( $param1 ,  $param2 ) {return 0;}}",
    "class a {\n  public function test($param1, $param2) {\n    return 0;\n  }\n}\n");

  return true;
}

bool TestParserStmt::TestStatementList() {
  V("<?php ",
    "");

  V("<?php return;",
    "return;\n");

  V("<?php function test() {} ; class Test {}",
    "function test() {\n}\nclass test {\n}\n");

  V("<?php function test() {} ; class Test {} __halt_compiler();",
    "function test() {\n}\nclass test {\n}\n");

  V("<?php ; __halt_compiler(); function test() {} class Test {}",
    "function test() {\n}\nclass test {\n}\n");

  return true;
}

bool TestParserStmt::TestBlockStatement() {
  V("<?php { }",
    "{\n}\n");

  V("<?php { return;}",
    "{\n  return;\n}\n");

  V("<?php { $a = 1;return;}",
    "{\n  $a = 1;\n  return;\n}\n");

  V("<?php declare(A=1) return;",
    "{\n  return;\n}\n");

  V("<?php declare(A=1): return; enddeclare;",
    "{\n  return;\n}\n");

  V("<?php declare(A=1): $a = 1;return; enddeclare;",
    "{\n  $a = 1;\n  return;\n}\n");

  return true;
}

bool TestParserStmt::TestIfBranchStatement() {
  return TestIfStatement();
}

bool TestParserStmt::TestIfStatement() {
  V("<?php if (a>1);",
    "if (a > 1) {}\n");

  V("<?php if (a>1) {}",
    "if (a > 1) {\n}\n");

  V("<?php if (a>1) { return;}",
    "if (a > 1) {\n  return;\n}\n");

  V("<?php if (a>1) { return; return 2;}",
    "if (a > 1) {\n  return;\n  return 2;\n}\n");

  V("<?php if (a>1); else;",
    "if (a > 1) {}\n");

  V("<?php if (a>1); else {}",
    "if (a > 1) {}\nelse {\n}\n");

  V("<?php if (a>1); else { return;}",
    "if (a > 1) {}\nelse {\n  return;\n}\n");

  V("<?php if (a>1); else { return; return 2;}",
    "if (a > 1) {}\nelse {\n  return;\n  return 2;\n}\n");

  V("<?php if (a>1); elseif(b>2);",
    "if (a > 1) {}\nelseif (b > 2) {}\n");

  V("<?php if (a>1); elseif(b>2){}",
    "if (a > 1) {}\nelseif (b > 2) {\n}\n");

  V("<?php if (a>1); elseif(b>2){ return;}",
    "if (a > 1) {}\nelseif (b > 2) {\n  return;\n}\n");

  V("<?php if (a>1); elseif(b>2){ return; return 2;}",
    "if (a > 1) {}\nelseif (b > 2) {\n  return;\n  return 2;\n}\n");

  V("<?php if (a>1); elseif(b>2); else {$a=2;}",
    "if (a > 1) {}\nelseif (b > 2) {}\nelse {\n  $a = 2;\n}\n");

  V("<?php if (a>1); elseif(b>2){} else {$a=2;}",
    "if (a > 1) {}\nelseif (b > 2) {\n}\nelse {\n  $a = 2;\n}\n");

  V("<?php if (a>1); elseif(b>2){ return;} else{$a=2;}",
    "if (a > 1) {}\nelseif (b > 2) {\n  return;\n}\nelse {\n  $a = 2;\n}\n");

  V("<?php if (a>1); elseif(b>2){ return; return 2;} else{$a=2;}",
    "if (a > 1) {}\nelseif (b > 2) {\n  return;\n  return 2;\n}\nelse {\n  $a = 2;\n}\n");

  // if-endif format

  V("<?php if (a>1): endif;",
    "if (a > 1) {}\n");

  V("<?php if (a>1):;endif;",
    "if (a > 1) {\n}\n");

  V("<?php if (a>1): {} endif;",
    "if (a > 1) {\n  {\n  }\n}\n");

  V("<?php if (a>1): return; endif;",
    "if (a > 1) {\n  return;\n}\n");

  V("<?php if (a>1): return; return 2; endif;",
    "if (a > 1) {\n  return;\n  return 2;\n}\n");

  V("<?php if (a>1):; else: endif;",
    "if (a > 1) {\n}\n");

  V("<?php if (a>1):; else:; endif;",
    "if (a > 1) {\n}\nelse {\n}\n");

  V("<?php if (a>1):; else: {} endif;",
    "if (a > 1) {\n}\nelse {\n  {\n  }\n}\n");

  V("<?php if (a>1):; else: return; endif;",
    "if (a > 1) {\n}\nelse {\n  return;\n}\n");

  V("<?php if (a>1):; else: return; return 2; endif;",
    "if (a > 1) {\n}\nelse {\n  return;\n  return 2;\n}\n");

  V("<?php if (a>1):; elseif(b>2): endif;",
    "if (a > 1) {\n}\nelseif (b > 2) {}\n");

  V("<?php if (a>1):; elseif(b>2):; endif;",
    "if (a > 1) {\n}\nelseif (b > 2) {\n}\n");

  V("<?php if (a>1):; elseif(b>2):{} endif;",
    "if (a > 1) {\n}\nelseif (b > 2) {\n  {\n  }\n}\n");

  V("<?php if (a>1):; elseif(b>2): return; endif;",
    "if (a > 1) {\n}\nelseif (b > 2) {\n  return;\n}\n");

  V("<?php if (a>1):; elseif(b>2): return; return 2; endif;",
    "if (a > 1) {\n}\nelseif (b > 2) {\n  return;\n  return 2;\n}\n");

  V("<?php if (a>1):; elseif(b>2):; else: $a=2; endif;",
    "if (a > 1) {\n}\nelseif (b > 2) {\n}\nelse {\n  $a = 2;\n}\n");

  V("<?php if (a>1):; elseif(b>2):{} else: $a=2; endif;",
    "if (a > 1) {\n}\nelseif (b > 2) {\n  {\n  }\n}\nelse {\n  $a = 2;\n}\n");

  V("<?php if (a>1):; elseif(b>2): return; else:$a=2; endif;",
    "if (a > 1) {\n}\nelseif (b > 2) {\n  return;\n}\nelse {\n  $a = 2;\n}\n");

  V("<?php if (a>1):; elseif(b>2): return; return 2; else:$a=2; endif;",
    "if (a > 1) {\n}\nelseif (b > 2) {\n  return;\n  return 2;\n}\nelse {\n  $a = 2;\n}\n");

  return true;
}

bool TestParserStmt::TestWhileStatement() {
  V("<?php while (true);",
    "while (true) {}\n");

  V("<?php while (true) {}",
    "while (true) {\n}\n");

  V("<?php while (true) { return;}",
    "while (true) {\n  return;\n}\n");

  V("<?php while (true): endwhile;",
    "while (true) {}\n");

  V("<?php while (true):; endwhile;",
    "while (true) {\n}\n");

  V("<?php while (true): {} endwhile;",
    "while (true) {\n  {\n  }\n}\n");

  V("<?php while (true): return; endwhile;",
    "while (true) {\n  return;\n}\n");

  return true;
}

bool TestParserStmt::TestDoStatement() {
  V("<?php do ; while (true);",
    "do {}\nwhile (true);\n");

  V("<?php do {} while (true);",
    "do {\n}\nwhile (true);\n");

  V("<?php do {$a=1;} while (true);",
    "do {\n  $a = 1;\n}\nwhile (true);\n");

  return true;
}

bool TestParserStmt::TestForStatement() {
  V("<?php for ($i = 1; $i < 10; $i++);",
    "for ($i = 1; $i < 10; $i++) {}\n");

  V("<?php for ($i = 1; $i < 10; $i++) {}",
    "for ($i = 1; $i < 10; $i++) {\n}\n");

  V("<?php for ($i = 1; $i < 10; $i++) { return;}",
    "for ($i = 1; $i < 10; $i++) {\n  return;\n}\n");

  V("<?php for ($i = 1; $i < 10; $i++): endfor;",
    "for ($i = 1; $i < 10; $i++) {}\n");

  V("<?php for ($i = 1; $i < 10; $i++):; endfor;",
    "for ($i = 1; $i < 10; $i++) {\n}\n");

  V("<?php for ($i = 1; $i < 10; $i++): {} endfor;",
    "for ($i = 1; $i < 10; $i++) {\n  {\n  }\n}\n");

  V("<?php for ($i = 1; $i < 10; $i++): return; endfor;",
    "for ($i = 1; $i < 10; $i++) {\n  return;\n}\n");

  V("<?php for ($i = 1,$j = 2; $i < 10; $i++,$j++): return; endfor;",
    "for ($i = 1, $j = 2; $i < 10; $i++, $j++) {\n  return;\n}\n");

  return true;
}

bool TestParserStmt::TestSwitchStatement() {
  V("<?php switch($a) {}",
    "switch ($a) {\n}\n");

  V("<?php switch($a) { case 1: }",
    "switch ($a) {\ncase 1:\n}\n");

  V("<?php switch($a) { case 1: ; }",
    "switch ($a) {\ncase 1:\n}\n");

  V("<?php switch($a) { case 1: $a = 1; break; }",
    "switch ($a) {\ncase 1:\n  $a = 1;\n  break;\n}\n");

  V("<?php switch($a) { default: }",
    "switch ($a) {\ndefault:\n}\n");

  V("<?php switch($a) { default:; }",
    "switch ($a) {\ndefault:\n}\n");

  V("<?php switch($a) { default: $a = 1; break; }",
    "switch ($a) {\ndefault:\n  $a = 1;\n  break;\n}\n");

  V("<?php switch($a) { case 1: $a = 0; case 2: $a = 1; break; }",
    "switch ($a) {\ncase 1:\n  $a = 0;\ncase 2:\n  $a = 1;\n  break;\n}\n");

  V("<?php switch($a) { case 1: $a = 0; default: $a = 1; break; }",
    "switch ($a) {\ncase 1:\n  $a = 0;\ndefault:\n  $a = 1;\n  break;\n}\n");

  V("<?php switch($a) { default;; }",
    "switch ($a) {\ndefault:\n}\n");

  V("<?php switch($a) { ;default;; }",
    "switch ($a) {\ndefault:\n}\n");

  V("<?php switch($a) :default;; endswitch;",
    "switch ($a) {\ndefault:\n}\n");

  V("<?php switch($a) :;default;; endswitch;",
    "switch ($a) {\ndefault:\n}\n");

  return true;
}

bool TestParserStmt::TestCaseStatement() {
  return TestSwitchStatement();
}

bool TestParserStmt::TestBreakStatement() {
  V("<?php break; break $a;",
    "break;\nbreak $a;\n");

  return true;
}

bool TestParserStmt::TestContinueStatement() {
  V("<?php continue; continue $a;",
    "continue;\ncontinue $a;\n");

  return true;
}

bool TestParserStmt::TestReturnStatement() {
  V("<?php return; return $a; return 1;",
    "return;\nreturn $a;\nreturn 1;\n");

  return true;
}

bool TestParserStmt::TestGlobalStatement() {
  V("<?php global $a;",
    "global $a;\n");

  V("<?php global $$b;",
    "global ${$b};\n");

  V("<?php global ${$a};",
    "global ${$a};\n");

  V("<?php global $a, $$b, ${$a};",
    "global $a, ${$b}, ${$a};\n");

  return true;
}

bool TestParserStmt::TestStaticStatement() {
  V("<?php static $a;",
    "static $a = null;\n");

  V("<?php static $a = 1;",
    "static $a = 1;\n");

  V("<?php static $a, $b;",
    "static $a = null, $b = null;\n");

  V("<?php static $a = 1, $b;",
    "static $a = 1, $b = null;\n");

  V("<?php static $a, $b = 1;",
    "static $a = null, $b = 1;\n");

  V("<?php static $a = \"$-\";",
    "static $a = '$-';\n");

  return true;
}

bool TestParserStmt::TestEchoStatement() {
  V("<?php echo $a;",
    "echo $a;\n");

  V("<?php echo $a, $b;",
    "echo $a, $b;\n");

  V("$a;",
    "echo '$a;';\n");

  V("'\r\n\t \\$a\"",
    "echo \"\'\\r\\n\\t\".' '.\"\\\\\".'$a\"';\n");

  return true;
}

bool TestParserStmt::TestUnsetStatement() {
  V("<?php unset($a);",
    "unset($a);\n");

  V("<?php unset($a,$b);",
    "unset($a, $b);\n");

  return true;
}

bool TestParserStmt::TestExpStatement() {
  V("<?php $a;",
    "$a;\n");

  return true;
}

bool TestParserStmt::TestForEachStatement() {
  V("<?php foreach ($a as $b) ;",
    "foreach ($a as $b) {}\n");

  V("<?php foreach ($a + $b as $b) ;",
    "foreach ($a + $b as $b) {}\n");

  V("<?php foreach ($a as $b) {}",
    "foreach ($a as $b) {\n}\n");

  V("<?php foreach ($a + $b as $b) {}",
    "foreach ($a + $b as $b) {\n}\n");

  V("<?php foreach ($a as &$b) ;",
    "foreach ($a as &$b) {}\n");

  V("<?php foreach ($a + $b as $b => $c) ;",
    "foreach ($a + $b as $b => $c) {}\n");

  V("<?php foreach ($a + $b as $b => &$c) ;",
    "foreach ($a + $b as $b => &$c) {}\n");

  V("<?php foreach ($a + $b as $b => &$c) : $a = 1; $b = 2; endforeach;",
    "foreach ($a + $b as $b => &$c) {\n  $a = 1;\n  $b = 2;\n}\n");

  V("<?php foreach ($a as &$name => &$b) ;",
    "foreach ($a as $name => &$b) {}\n");

  V("<?php foreach ($a as &$name => $b) ;",
    "foreach ($a as $name => $b) {}\n");

  return true;
}

bool TestParserStmt::TestCatchStatement() {
  V("<?php try { } catch (Exception $e) {}",
    "try {\n} catch (exception $e) {\n}\n");

  V("<?php try { $a = 1;} catch (Exception $e) {$b=2;}",
    "try {\n  $a = 1;\n} catch (exception $e) {\n  $b = 2;\n}\n");

  V("<?php try { $a = 1;} catch (Exception $e) {$b=2;} catch (Exception2 $e2) { $c =3;}",
    "try {\n  $a = 1;\n} catch (exception $e) {\n  $b = 2;\n} catch (exception2 $e2) {\n  $c = 3;\n}\n");

  return true;
}

bool TestParserStmt::TestTryStatement() {
  return TestCatchStatement();
}

bool TestParserStmt::TestThrowStatement() {
  V("<?php throw $a;",
    "throw $a;\n");

  return true;
}
