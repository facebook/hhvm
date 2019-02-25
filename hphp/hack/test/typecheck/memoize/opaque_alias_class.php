//// file1.php
<?hh // partial

class BarImpl implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

newtype Bar = BarImpl;

//// file2.php
<?hh // partial

class Foo {
  <<__Memoize>>
  public function someMethod(Bar $i): void {}
}

<<__Memoize>>
function some_function(Bar $i): void {}
