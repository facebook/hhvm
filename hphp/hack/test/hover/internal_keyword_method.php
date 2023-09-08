//// modules.php
<?hh

new module foo {}

//// test.php
<?hh
module foo;

class Foo {
  internal function callFoo(): void {
  // ^ hover-at-caret
    echo "callFoo";
  }
}
