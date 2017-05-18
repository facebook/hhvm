<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function run_tests($tests) {
  foreach ($tests as $test) {
    list($func, $val) = $test;
    try {
      var_dump($func($val));
    } catch (Exception $e) {
      echo "Exception: " . $e->getMessage() . "\n";
    }
  }
}

// Use type-hints to test HHBBC optimizations

function test1($v) { return HH\array_key_cast($v); }
function test2(bool $v) { return HH\array_key_cast($v); }
function test3(bool $v) { return HH\array_key_cast($v); }
function test4(int $v) { return HH\array_key_cast($v); }
function test5(double $v) { return HH\array_key_cast($v); }
function test6(string $v) { return HH\array_key_cast($v); }
function test7(string $v) { return HH\array_key_cast($v); }
function test8(resource $v) { return HH\array_key_cast($v); }
function test9(array $v) { return HH\array_key_cast($v); }
function test10(vec $v) { return HH\array_key_cast($v); }
function test11(dict $v) { return HH\array_key_cast($v); }
function test12(keyset $v) { return HH\array_key_cast($v); }
function test13(stdclass $v) { return HH\array_key_cast($v); }

function func_maker1() { return 'HH\array_key_cast'; }
function func_maker2() {
  static $x = 1;
  return 'test' . $x++;
}

function make_tests($func) {
  $tests = vec[
    vec[$func(), null],
    vec[$func(), false],
    vec[$func(), true],
    vec[$func(), 123],
    vec[$func(), 789.123],
    vec[$func(), 'abc'],
    vec[$func(), '456'],
    vec[$func(), STDIN],
    vec[$func(), []],
    vec[$func(), vec[]],
    vec[$func(), dict[]],
    vec[$func(), keyset[]],
    vec[$func(), new stdclass]
  ];
  return __hhvm_intrinsics\launder_value($tests);
}

run_tests(make_tests('func_maker1'));
run_tests(make_tests('func_maker2'));
