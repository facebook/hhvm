//// file1.php
<?hh // partial

newtype Bar = array<int>;

//// file2.php
<?hh // partial

class Foo {
  <<__Memoize>>
  public function someMethod(Bar $i): void {}
}

<<__Memoize>>
function some_function(Bar $i): void {}
