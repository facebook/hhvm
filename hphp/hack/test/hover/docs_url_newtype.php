<?hh

<<__Docs("http://example.com")>>
newtype Foo = string;

function takes_foo(Foo $f): void {}
//                   ^ hover-at-caret
