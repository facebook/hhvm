<?hh

<<__Docs("http://example.com")>>
enum Foo: string as string {}

function takes_foo(Foo $f): void {}
//                   ^ hover-at-caret
