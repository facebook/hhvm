<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static $foo;
  public static function set($x) { self::$foo = $x; }
  public static function get() { return [self::$foo => 'abc']; }
}

function test() {
  A::set(__hhvm_intrinsics\launder_value(new stdclass));
  var_dump((bool)A::get());
}

<<__EntryPoint>>
function main_array_obj_key() {
test();
}
