//// file1.php
<?hh // partial

newtype fbid = int;

//// file2.php
<?hh // partial

class Foo {
  <<__Memoize>>
  public function someMethod(fbid $i): void {}
}

<<__Memoize>>
function some_function(fbid $i): void {}
