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

#include <test/test_performance.h>
#include <util/util.h>

using namespace std;

#define PERF_LOOP_COUNT "500"

#define PERF_START                                      \
  "<?php\n"                                             \
  "function timing_get_cpu_time() {\n"                  \
  "  $rusage = getrusage();\n"                          \
  "  return ($rusage['ru_utime.tv_sec']*1000*1000 +\n"  \
  "          $rusage['ru_utime.tv_usec'] +\n"           \
  "          $rusage['ru_stime.tv_sec']*1000*1000 +\n"  \
  "          $rusage['ru_stime.tv_usec']);\n"           \
  "}\n"                                                 \
  "$start = timing_get_cpu_time();\n"                   \
  "/* INPUT */"                                         \

#define PERF_END                                      \
  "/* INPUT */"                                       \
  "$end = timing_get_cpu_time();\n"                   \
  "print (($end - $start)/1000).\"ms\";\n"            \

///////////////////////////////////////////////////////////////////////////////

TestPerformance::TestPerformance() {
  m_perfMode = true;
  TestCodeRun::FastMode = false;
}

bool TestPerformance::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestBasicOperations);
  RUN_TEST(TestMemoryUsage);
  RUN_TEST(TestAdHocFile);
  RUN_TEST(TestAdHoc);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// performance testing

bool TestPerformance::TestBasicOperations() {
  VCR(PERF_START
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) {} $k = $i;"
      "\n\n/* Baseline - finding loop cost */"
      PERF_END);

  VCR(PERF_START
      "$j = 0;\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { $j += $i;} $k = $j;"
      "\n\n/* Inferrable integer arithmetic */"
      PERF_END);

  VCR(PERF_START
      "$j = 'test'; $j = 0; for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) "
      "{ $j += $i;}"
      "\n\n/* Non-inferrable integer arithmetic */"
      PERF_END);

  VCR(PERF_START
      "$a = 'test'; $b = 'test';\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { $c = $a . $b;} "
      "\n\n/* String concatenation */"
      PERF_END);

  VCR(PERF_START
      "function func() {}\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { func();}"
      "\n\n/* Calling a function without a parameter */"
      PERF_END);

  VCR(PERF_START
      "function func($a) { return $a;}\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { $i = func($i);}"
      "\n\n/* Calling a function with one parameter */"
      PERF_END);

  VCR(PERF_START
      "function func($a, $b) { static $k; $k = $a + $b;}\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { func($i, $i);}"
      "\n\n/* Calling a function with two parameters + static variables */"
      PERF_END);

  VCR(PERF_START
      "$global = 'test';\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) "
      "{ global $global; $b = $global;}"
      "\n\n/* Global variable resolution */"
      PERF_END);

  VCR(PERF_START
      "$a = array('test');\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { $b = $a[0];}"
      "\n\n/* Taking an array element with numeric index */"
      PERF_END);

  VCR(PERF_START
      "$a = array('test' => 'ok');\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { $b = $a['test'];}"
      "\n\n/* Taking an array element with string index */"
      PERF_END);

  VCR(PERF_START
      "$a = array();\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { $a[i] = 5;}"
      "\n\n/* Array element assignment */"
      PERF_END);

  VCR(PERF_START
      "$a = array();\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { $a[] = 5;}"
      "\n\n/* Array element appending */"
      PERF_END);

  VCR(PERF_START
      "class A { public $a = 'test'; }; $obj = new A();\n"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) { $b = $obj->a;}"
      "\n\n/* Taking an object's property */"
      PERF_END);

  return true;
}

bool TestPerformance::TestMemoryUsage() {
  VCR(PERF_START
      "$a = array();\n"
      "for ($i = 0; $i < 3000000; $i++) { $a[] = 'test';} sleep(5);"
      PERF_END);
  return true;
}

bool TestPerformance::TestAdHocFile() {
  string input;
  FILE *f = fopen("test/perf_ad_hoc.php", "r");
  if (f) {
    char buf[1024];
    for (int len = fread(buf, 1, sizeof(buf), f); len;
         len = fread(buf, 1, sizeof(buf), f)) {
      input.append(buf, len);
    }
    fclose(f);
  }

  Util::replaceAll(input, "PERF_START", "?>" PERF_START);
  Util::replaceAll(input, "PERF_END", PERF_END);

  VCR(input.c_str());

  return true;
}

bool TestPerformance::TestAdHoc() {
  VCR(PERF_START
      "for ($j = 0; $j < 100; $j++) {"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) {}} $k = $i;"
      "\n\n/* Baseline - finding loop cost */"
      PERF_END);

  VCR(PERF_START
      "for ($j = 0; $j < 1000; $j++) {"
      "$a = null; $a = array(1, 2, 3, 4, 5, array(1,2,3,4));"
      "for ($i = 0; $i < " PERF_LOOP_COUNT "; $i++) {"

      // add PHP code here
      "$a['test'.$i] = $i;"

      "}"
      "}"
      "\n\n/* my ad hoc test */"
      PERF_END);

  return true;
}
