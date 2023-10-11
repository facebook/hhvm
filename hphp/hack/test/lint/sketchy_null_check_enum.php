//// file1.php
<?hh

enum Foo : int {
  ONE = 1;
  TWO = 2;
}

//// file2.php
<?hh

function test(?Foo $x): void {
  if ($x) {
  }
}
