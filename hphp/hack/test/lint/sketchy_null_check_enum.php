//// file1.php
<?hh // strict

enum Foo : int {
  ONE = 1;
  TWO = 2;
}

//// file2.php
<?hh // strict

function test(?Foo $x): void {
  if ($x) {
  }
}
