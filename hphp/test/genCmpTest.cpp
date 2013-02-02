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
/*

This file generates a very large .php file.

The php file has a comparison for every possible combination of each of the
~70 different VARS (as either a constant-propagateable or indeterminate value)
and each of the 9 different OPS. This totals O(200k) different tests.

The php file, when run, generates O(1000000) lines of output.

Zend runs this file in a few seconds.
Now that each test is in its own function, hhvm with Jit can run in 8 minutes.

*/

#include <iostream>
#include <string>

using namespace std;

string OPS[] = {
  // http://www.php.net/manual/en/language.operators.comparison.php
  "==",
  "===",
  "!=",
  "<>",
  "!==",
  "<",
  ">",
  "<=",
  ">=",
};
int numOPS = sizeof(OPS) / sizeof(string);

string VARS[] = {
  // NULL
  "NULL",

  // bool
  "true",
  "false",

  // int
  "0",
  "1",
  "123",
  "-1",
  "-123",
  "7000",
  "123456789123456",
  "123456789123456789123456789", // overflow
  "456",
  
  // double
  "0.0",
  "0.00",
  "-0.0",
  "1.0",
  "-1.00000",
  "122.9",
  "123.1",
  "-122.9",
  "-123.1",
  "7e3",
  "7e+3",
  "123456789123456.0",
  "123456789123456789123456789.0",
  "741.258",

  // resource
  "curl_init()",

  // object
  "new O0()",
  "new O1(0)",
  "new O1(1)",
  "new O2(0)",
  "new O2(1)",
  "new O3(0, 0)",
  "new O3(0, 1)",
  "new O3(1, 0)",
  "new O3(1, 1)",
  "new S1('php')",
  "new S1('1')",
  "new S1('-1garbage')",

  // array
  "array()",
  "array(0 => '0')",
  "array(0 => '1')",
  "array('0' => '0')",
  "array('php' => '0')",
  "array(0 => 0, 1 => 1)",
  "array(0 => 1, 1 => 0)",
  "array(0 => 2, 1 => 2)",

  // string
  "''",
  "'0'",
  "'0.0'",
  "'0.00'",
  "'-0'",
  "'-0.0'",
  "'false'",
  "'true'",
  "'1'",
  "'1.0'",
  "'-1'",
  "'123'",
  "'123.1'",
  "'123q'",
  "'123456789123456789'",
  "'123456789123456789123456789'",
  "'null'",
  "'NULL'",
  "'7000'",
  "'7e3'",
  "'7.0E3'",
  "'7000.0'",
  "'php'",
  "'elephant'",
  "'electron'",
};
int numVARS = sizeof(VARS) / sizeof(string);

string prolog(

"<?php\n\n"

// helpers

"function id($x) { return $x; }\n\n"

// Pretty printers

"function prettyCC($x, $op, $y, $res) {\n"
"  print \"--------------------\\n\";\n"
"  print \"$x $op $y\\n\";\n"
"  if ($res) { print \"true\\n\"; } else { print \"false\\n\"; }\n"
"}\n\n"

"function prettyCN($x, $op, $y, $res) {\n"
"  print \"--------------------\\n\";\n"
"  print \"$x $op \\$y\\n\";\n"
"  print \"\\$y = \";\n"
"  var_dump($y);\n"
"  if ($res) { print \"true\\n\"; } else { print \"false\\n\"; }\n"
"}\n\n"

"function prettyNC($x, $op, $y, $res) {\n"
"  print \"--------------------\\n\";\n"
"  print \"\\$x $op $y\\n\";\n"
"  print \"\\$x = \";\n"
"  var_dump($x);\n"
"  if ($res) { print \"true\\n\"; } else { print \"false\\n\"; }\n"
"}\n\n"

"function prettyNN($x, $op, $y, $res) {\n"
"  print \"--------------------\\n\";\n"
"  print \"\\$x $op \\$y\\n\";\n"
"  print \"\\$x = \";\n"
"  var_dump($x);\n"
"  print \"\\$y = \";\n"
"  var_dump($y);\n"
"  if ($res) { print \"true\\n\"; } else { print \"false\\n\"; }\n"
"}\n\n"

// Classes

"class O0 {}\n\n"

"class O1 {\n"
"  public $x;\n"
"  function __construct($a) {\n"
"    $this->x = $a;\n"
"  }\n"
"}\n\n"

"class O2 {\n"
"  public $y;\n"
"  function __construct($a) {\n"
"    $this->y = $a;\n"
"  }\n"
"}\n\n"

"class O3 {\n"
"  public $x;\n"
"  private $y;\n"
"  function __construct($a, $b) {\n"
"    $this->x = $a;\n"
"    $this->y = $b;\n"
"  }\n"
"}\n\n"

"class S1 {\n"
"  private $x;\n"
"  function __construct($a) {\n"
"    $this->x = $a;\n"
"  }\n"
"  function __toString() {\n"
"    return $this->x;\n"
"  }\n"
"}\n\n"

);

int main() {
  cout << prolog;

  for (int j = 0; j < numOPS; ++j) {
    for (int i1 = 0; i1 < numVARS; ++i1) {
      for (int i2 = 0; i2 < numVARS; ++i2) {
        // each test is in its own function
        cout << "function foo_" << j << "_" << i1 << "_" << i2 << "() {\n";

        // generate the variables required for that test
        cout << "  # create variables\n";
        cout << "  $c" << i1 << " = " << VARS[i1] << ";\n";
        cout << "  $x" << i1 << " = id($c" << i1 << ");\n";
        if (i2 != i1) {
          cout << "  $c" << i2 << " = " << VARS[i2] << ";\n";
          cout << "  $x" << i2 << " = id($c" << i2 << ");\n";
        }
        cout << "\n";

        // pretty print the test results
        // pass strings into the C's, for proper printing
        cout << "  # " << VARS[i1] << " " << OPS[j] << " " << VARS[i2]
             << " --- " << i1 << ", " << j << ", " << i2 << "\n";
        cout << "  prettyCC("
             << "\"" << VARS[i1] << "\", "
             << "\"" << OPS[j] << "\", "
             << "\"" << VARS[i2] << "\", "
             << "$c" << i1 << " " << OPS[j] << " $c" << i2
             << ");\n";
        cout << "  prettyCN("
             << "\"" << VARS[i1] << "\", "
             << "\"" << OPS[j] << "\", "
             << "$x" << i2 << ", "
             << "$c" << i1 << " " << OPS[j] << " $x" << i2
             << ");\n";
        cout << "  prettyNC("
             << "$x" << i1 << ", "
             << "\"" << OPS[j] << "\", "
             << "\"" << VARS[i2] << "\", "
             << "$x" << i1 << " " << OPS[j] << " $c" << i2
             << ");\n";
        cout << "  prettyNN("
             << "$x" << i1 << ", "
             << "\"" << OPS[j] << "\", "
             << "$x" << i2 << ", "
             << "$x" << i1 << " " << OPS[j] << " $x" << i2
             << ");\n";

        // close and call the function
        cout << "}\n";
        cout << "foo_" << j << "_" << i1 << "_" << i2 << "();\n";
      }
    }
  }

  return 0;
}

