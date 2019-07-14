<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo<T super null>(null $x, @null $z): T {
  return null;
}

type Something = null;
newtype Foo = null;

class C {
  static vec<null> $z;
  const type T = null;
}

function test(): void {
  foo(null, null);
}
<<__EntryPoint>> function main(): void {
test();
var_dump("done");
}
