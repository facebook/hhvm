<?hh

<<__Docs("http://example.com")>>
class Foo {}

<<__Docs("http://example.com/foochild")>>
class FooChild extends Foo {}

function takes_foochild(FooChild $f): void {}
//                      ^ hover-at-caret
