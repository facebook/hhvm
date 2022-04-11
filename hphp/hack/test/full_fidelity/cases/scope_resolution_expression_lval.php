<?hh

function foo(): void {
  Foo::$bar = 1; // OK
  Foo::{$bar} = 2; // OK
  Foo::BAR = 3; // Not OK
}
