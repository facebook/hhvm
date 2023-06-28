<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  public $p;
}

class B {
  private static $stats1 = dict[];
  public static $stats2;

  public static function reset1($key) :mixed{
    return self::$stats1[$key] ??= 0;
  }
  public static function reset2($key) :mixed{
    return self::$stats2->$key ??= 0;
  }
}

<<__EntryPoint>>
function main() :mixed{
  B::$stats2 = new A();
  var_dump(B::reset1(__hhvm_intrinsics\launder_value(123)));
  var_dump(B::reset2(__hhvm_intrinsics\launder_value('p')));
}
