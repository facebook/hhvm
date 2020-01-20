//// file1.php
<?hh

function f1(): (int, string, bool) {
  throw new Exception();
}

//// file2.php
<?hh

function f2(): void {
  $c = f1();
  list($a, $b) = $c;
}
