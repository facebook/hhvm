<?hh

/**
 * The Foo class does stuff.
 */
<<__Docs("http://example.com")>>
class Foo {}

function takes_foo(Foo $f): void {}
//                 ^ hover-at-caret
