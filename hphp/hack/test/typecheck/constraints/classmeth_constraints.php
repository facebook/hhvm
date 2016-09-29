<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Base {}
class MyClass {
  public static function element<T as Base>(?T $element): void {}
}

function my_array_map<Tm>((function(Tm): void) $f, array<Tm> $a): void {}
function render(): void {
  $f = class_meth(MyClass::class, 'element');
  $f(new Base());
  my_array_map($f, array());
}
