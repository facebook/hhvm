//// modules.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>

new module foo {}

//// test.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;

class Foo {
  internal function callFoo(): void {
  // ^ hover-at-caret
    echo "callFoo";
  }
}
