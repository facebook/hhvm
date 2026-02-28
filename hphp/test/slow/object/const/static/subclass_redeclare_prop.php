<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__Const>>
  public static int $x = 5;
  public static function write_x() :mixed{
    static::$x = 10;
  }
}

class B extends A {
  public static int $x = 6;
}

<<__EntryPoint>>
function main() :mixed{
  try {
    A::write_x();
    echo "FAIL: wrote to static const property\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  B::write_x();
  var_dump(A::$x);
  var_dump(B::$x);
}
