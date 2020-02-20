//// file1.php
<?hh

function f1(mixed $x): dynamic {
  return $x;
}

//// file2.php
<?hh

function f2(dynamic $x): mixed {
  return f1($x);
}

//// file3.php
<?hh

function f3(mixed $x): mixed {
  return f2(f1($x));
}

//// file4.php
<?hh // partial

function f(): dynamic {
  return f3(f2(f1("")));
}
