<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

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
  // instance method calls
  $foo = new Foo();
  $foo->method(&$x);
  $foo = null;
  $foo?->method(&$x);

  // static method calls
  Foo::staticMethod(&$x);
}

function dynamic1(): void {
  $x = 1;
  $dyn = new Foo();
  $dyn as dynamic;
  $str = 'method';
  $dyn->$str(&$x);
}

function dynamic2(): void {
  $x = 1;
  $dyn = new Foo();
  $dyn as dynamic;
  $str = 'method';
  $dyn?->$str(&$x);
}

function static_dynamic1(): void {
  $x = 1;
  $str = 'Foo';
  $str::staticMethod(&$x);
}

function func_dynamic(): void {
  $x = 1;
  $arr = vec[fun('func')];
  $arr[0](&$x);
}

function static_dynamic2(): void {
  $x = 1;
  $str = 'staticMethod';
  Foo::$str(&$x);
}
