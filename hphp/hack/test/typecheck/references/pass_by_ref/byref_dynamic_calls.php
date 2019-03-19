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
  $str = 'method';
  $foo->$str(&$x);
  $foo = null;
  $foo?->method(&$x);
  $foo?->$str(&$x);

  // static method calls
  Foo::staticMethod(&$x);
  $str = 'Foo';
  $str::staticMethod(&$x);
  $str = 'staticMethod';
  Foo::$str(&$x);

  $arr = vec[fun('func')];
  $arr[0](&$x);
}
