<?hh

/**
 * I am the doc comment for MyAttribute.
 */
class MyAttribute implements HH\MethodAttribute {
}

class Foo {
  <<__Memoize, MyAttribute>>
  //               ^ hover-at-caret
  public function bar(): int { return 0; }
}
