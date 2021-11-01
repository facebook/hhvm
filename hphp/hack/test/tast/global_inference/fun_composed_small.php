//// file1.php
<?hh

function g1($x) {
  return $x;
}

//// file2.php
<?hh

function g2($x) {
  return $x;
}

//// file4.php
<?hh

function f() {
  return g2(g1(0));
}
