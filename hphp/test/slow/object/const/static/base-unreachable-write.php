<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static vec<int> $a = vec[10];
  <<__Const>>
  public static vec<int> $ca = vec[1,2,3];
}

<<__EntryPoint>>
function test() :mixed{
  A::$a[] = 20;
  var_dump(A::$a);
  var_dump(A::$ca);
}

function dead(): void{
  A::$ca[] = 4;
}
