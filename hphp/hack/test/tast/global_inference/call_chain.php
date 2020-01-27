//// file1.php
<?hh //partial

function f1($x) {
  return $x + 0;
}

//// file2.php
<?hh //partial

function f2($x) {
  return f1($x);
}

//// file3.php
<?hh //partial

function f3($x) {
  return f2($x);
}
