<?hh

class Foo {
  /** A doc comment. */
  const BAR = 1;
}

function use_it(): void {
  $x = Foo::BAR;
  //        ^ hover-at-caret
}
