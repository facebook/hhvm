//// file1.php
<?hh // partial

function g($x) {
  return $x;
}

//// file2.php
<?hh // partial

function f() {
  return g(g(g(0)));
}
