//// file1.php
<?hh // partial

function g1($x) {
  return $x;
}

//// file2.php
<?hh // partial

function g2($x) {
  return $x;
}

//// file3.php
<?hh // partial

function g3($x) {
  return $x;
}

//// file4.php
<?hh // partial

function f() {
  return g3(g2(g1(0)));
}
