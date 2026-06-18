<?hh

abstract class Foo {
  abstract const int BAR;
}

function use_it(Foo $f): void {
  $x = $f::BAR;
  //       ^ hover-at-caret
}
