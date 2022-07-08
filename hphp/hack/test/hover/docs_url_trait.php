<?hh

<<__Docs("http://example.com")>>
class TraitFoo {}

class Foo {
  use TraitFoo;
}

function takes_foo(Foo $f): void {}
//                 ^ hover-at-caret
