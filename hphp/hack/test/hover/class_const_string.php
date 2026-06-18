<?hh

class Foo {
  const string GK = 'my_gk_name';
}

function use_it(): void {
  $x = Foo::GK;
  //        ^ hover-at-caret
}
