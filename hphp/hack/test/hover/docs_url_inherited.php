<?hh

<<__Docs("http://example.com")>>
class Foo {}

class FooChild extends Foo {}

function takes_foochild(FooChild $f): void {}
//                      ^ hover-at-caret
