<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__Const>> public static int $p;
}

<<__EntryPoint>>
function foo(): void{
    var_dump(A::$p);
}
