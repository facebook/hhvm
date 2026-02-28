<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static int $a = 55;
  <<__Const>>
  public static int $ca = 5;
}

<<__EntryPoint>>
function test() :mixed{
  A::$a += 60;
  var_dump(A::$a);
  if (A::$a > 150) {
    A::$ca += 10;
  }
  var_dump(A::$ca);
}
