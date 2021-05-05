<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class MyBase {
  private final function foo(): void {}
}

class MyClass extends MyBase {
  private final function foo(): void {}
}

<<__EntryPoint>>
function my_main(): void {
  new MyClass();
}
