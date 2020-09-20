//// file1.php
<?hh // partial

class BarImpl {
}

newtype Bar = BarImpl;

//// file2.php
<?hh // partial

class Foo {
  <<__Memoize>>
  public function someMethod(Bar $i): void {}
}
