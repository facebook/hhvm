<?hh // partial
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function func(int &$x): void {
  $x = 5;
}

class Foo {
  public function method(int &$x): void {
    $x = 6;
  }

  public static function staticMethod(int &$x): void {
    $x = 6;
  }
}

function main(): void {
  $x = 1;

  // global function call
  func(&$x);

  // instance method calls
  $foo = new Foo();
  $foo->method(&$x);
  $foo = null;
  $foo?->method(&$x);

  // static method calls
  Foo::staticMethod(&$x);
}
