<?hh

// I am a comment on MyAttribute.
class MyAttribute implements HH\ClassAttribute {
}

<<MyAttribute>>
//  ^ hover-at-caret
class Foo {
  public function bar(): int { return 0; }
}
