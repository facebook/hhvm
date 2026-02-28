<?hh

<<__Docs("http://example.com")>>
enum class Foo: mixed {
  int X = 1;
}

function bar(): void {
  Foo#X;
// ^ hover-at-caret
}
