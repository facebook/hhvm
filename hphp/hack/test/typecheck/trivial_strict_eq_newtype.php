//// file1.php
<?hh
newtype B = int;

newtype A = B;

newtype Foo = A;

//// file2.php
<?hh

newtype Bar = string;

//// file3.php
<?hh

function test(Foo $foo, Bar $bar): void {
  if ($foo === $bar) {
  }
}
