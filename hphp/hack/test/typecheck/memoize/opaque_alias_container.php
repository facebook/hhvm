//// file1.php

<?hh

newtype Bar = array<int>;

//// file2.php

<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(Bar $i): void {}
}
