<?hh

<<file:__EnableUnstableFeatures("modules")>>

new module foo {}
module foo;

class Foo {
  internal function callFoo(): void {
  // ^ hover-at-caret
    echo "callFoo";
  }
}
