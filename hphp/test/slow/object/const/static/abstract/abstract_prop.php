<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  abstract static int $p;
  abstract int $l;
}

<<__EntryPoint>>
function dump_props(): void{
  var_dump(A::$p);
  var_dump(A::$l);
}
