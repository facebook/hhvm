//// file1.php
<?hh // partial

class BarImpl {
}

newtype Bar = array<BarImpl>;

//// file2.php
<?hh // partial
class Foo {
  <<__Memoize>>
  public function someMethod(Bar $i): void {}
}
