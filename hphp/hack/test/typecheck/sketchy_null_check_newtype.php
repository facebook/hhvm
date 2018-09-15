//// file1.php
<?hh // strict

newtype Foo as int = int;

//// file2.php
<?hh // strict

function test(?Foo $x): void {
  if ($x) {
  }
}
