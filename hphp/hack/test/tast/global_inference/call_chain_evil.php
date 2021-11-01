//// file1.php
<?hh

function f1($x) {
  return $x;
}

//// file2.php
<?hh

function f2($x) {
  return f1($x);
}

//// file3.php
<?hh

function f3($x) {
  return f2(f1($x));
}

//// file4.php
<?hh

function f() {
  return f3(f2(f1("")));
}
