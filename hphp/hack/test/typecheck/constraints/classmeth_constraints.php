<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Base {}
class MyClass {
  public static function element<T as Base>(?T $element): void {}
}

function my_array_map<Tm>((function(Tm): void) $f, varray<Tm> $a): void {}
function render(): void {
  $f = MyClass::element<>;
  $f(new Base());
  my_array_map($f, varray[]);
}
