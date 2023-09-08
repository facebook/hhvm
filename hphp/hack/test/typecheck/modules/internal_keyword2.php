//// modules.php
<?hh


new module foo {}

//// test.php
<?hh

module foo;
class Foo {
  public internal function bar(): void {}
}
