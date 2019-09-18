<?hh

function error_handler($errno, $errstr) {
  echo $errstr . "\n";
}

class A {
  <<__SoftLateInit>> public static $a;
  <<__SoftLateInit>> private static $b;

  public static function get() {
    return A::$b;
  }
  public static function set($x) {
    A::$b = $x;
  }
}

const TESTS = vec[
  'test1',
  'test2',
  'test3',
  'test4',
  'test5'
];

function test_common() {
  var_dump(A::$a);
  var_dump(A::get());

  var_dump(A::$a);
  var_dump(A::get());
}

function test1() {
  test_common();
}

function test2() {
  HH\set_soft_late_init_default(123);
  test_common();
}

function test3() {
  HH\set_soft_late_init_default('abc');
  test_common();
}

function test4() {
  HH\set_soft_late_init_default('abc');
  test_common();
  HH\set_soft_late_init_default('def');
  test_common();
}

function test5() {
  A::$a = 789;
  A::set(789);
  HH\set_soft_late_init_default('abc');
  test_common();
}

function main() {
  set_error_handler(fun('error_handler'));

  $count = __hhvm_intrinsics\apc_fetch_no_check('test-count');
  if ($count === false) $count = 0;
  if ($count >= count(TESTS)) return;
  TESTS[$count]();
  $count++;
  apc_store('test-count', $count);
}
<<__EntryPoint>>
function main_entry(): void {

  main();
}
