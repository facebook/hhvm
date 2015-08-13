//// file1.php

<?hh

newtype fbid = int;

//// file2.php

<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(fbid $i): void {}
}
