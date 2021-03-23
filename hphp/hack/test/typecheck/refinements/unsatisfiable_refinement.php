<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I { public function bar():void; }
class C<T> { }
class D extends C<string> { }
function foo<T as I>(T $x):void {
  $x->bar();
}
function testit<T as I>(C<T> $x):void {
  if ($x is D) {
    // just drop through
  }
  foo("A");
}

<<__EntryPoint>>
function main():void {
  testit(new C<I>());
}
