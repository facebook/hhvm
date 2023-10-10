<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__Const>> abstract public static int $p;
  <<__Const>> abstract public int $l;
}

<<__EntryPoint>>
function dump_props(): void{
  var_dump(A::$p);
  var_dump(A::$l);
}
