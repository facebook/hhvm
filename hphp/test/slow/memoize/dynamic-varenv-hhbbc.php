<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Memoize, __Deprecated>> function foo(int $x) {
  echo "foo called\n";
  return $x;
}

<<__EntryPoint>>
function main_dynamic_varenv_hhbbc() {
var_dump(foo(123));
var_dump(foo(123));
}
