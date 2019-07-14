<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__LateInit>> public $x1 = 123;
}
<<__EntryPoint>> function main(): void {
new A();
}
