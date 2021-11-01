//// file1.php
<?hh

function g($x) {
  return $x;
}

//// file2.php
<?hh

function f() {
  return g(g(g(0)));
}
