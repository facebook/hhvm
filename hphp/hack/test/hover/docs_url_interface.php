<?hh

<<__Docs("http://example.com")>>
class IFoo {}

class FooChild implements IFoo {}

function takes_foochild(FooChild $f): void {}
//                      ^ hover-at-caret
