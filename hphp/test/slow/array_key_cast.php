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
function test5(float $v) { return HH\array_key_cast($v); }
function test6(string $v) { return HH\array_key_cast($v); }
function test7(string $v) { return HH\array_key_cast($v); }
function test8(resource $v) { return HH\array_key_cast($v); }
function test9(varray $v) { return HH\array_key_cast($v); }
function test10(vec $v) { return HH\array_key_cast($v); }
function test11(dict $v) { return HH\array_key_cast($v); }
function test12(keyset $v) { return HH\array_key_cast($v); }
function test13(stdClass $v) { return HH\array_key_cast($v); }

function func_maker1() { return 'HH\array_key_cast'; }

abstract final class FuncMaker2Statics {
  public static $x = 1;
}
function func_maker2() {
  return 'test' . FuncMaker2Statics::$x++;
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
    vec[$func(), fopen(__FILE__, 'r')],
    vec[$func(), varray[]],
    vec[$func(), vec[]],
    vec[$func(), dict[]],
    vec[$func(), keyset[]],
    vec[$func(), new stdClass]
  ];
  return __hhvm_intrinsics\launder_value($tests);
}


<<__EntryPoint>>
function main_array_key_cast() {
run_tests(make_tests(func_maker1<>));
run_tests(make_tests(func_maker2<>));
}
