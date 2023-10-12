<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Base {}
class MyClass {
  public static function element<T as Base>(?T $element): void {}
}

function render(): void {
  $f = MyClass::element<>;
  $f(3);
}
