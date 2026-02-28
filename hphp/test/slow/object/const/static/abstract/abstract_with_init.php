<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  <<__Const>> abstract static public int $a = 5;
}

<<__EntryPoint>>
function f(): void{
  var_dump(A::$a);
}
