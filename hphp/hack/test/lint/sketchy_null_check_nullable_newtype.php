//// file1.php
<?hh

newtype Foo as ?int = ?int;

//// file2.php
<?hh

function test(Foo $x): void {
  if ($x) {
  }
}
