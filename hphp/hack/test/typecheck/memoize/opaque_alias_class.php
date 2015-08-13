//// file1.php

<?hh

class BarImpl implements IMemoizeParam {
  public function getInstanceKey(): string {
    return "dummy";
  }
}

newtype Bar = BarImpl;

//// file2.php

<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(Bar $i): void {}
}
