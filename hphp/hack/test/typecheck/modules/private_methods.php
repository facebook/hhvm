//// modules.php
<?hh


new module foo {}

//// test.php
<?hh

module foo;

internal type Bar = int;
internal class Bar2 {}
class Foo {
  private function foo(): Bar {
    return 5;
  }
  private function foo2(Bar $_, Bar2 $_2): void {}

}
