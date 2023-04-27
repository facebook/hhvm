<?hh

<<__Docs("http://example.com")>>
type Foo = string;

function takes_foo(Foo $f): void {}
//                   ^ hover-at-caret
