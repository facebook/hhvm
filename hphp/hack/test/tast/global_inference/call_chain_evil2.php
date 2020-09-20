//// file1.php
<?hh //partial

function f1($x) {
  return $x;
}

//// file2.php
<?hh //partial

function f2($x) {
  return f1($x);
}

//// file3.php
<?hh //partial

function f3($x) {
  return f1(f2($x));
}

//// file4.php
<?hh // partial

function f() {
  return f1(f2(f3("")));
}
