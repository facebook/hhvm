<?hh

class Foo {
  const BAR = 1;
}

function use_it(): void {
  $x = Foo::BAR;
  //        ^ hover-at-caret
}
