//// file1.php
<?hh

newtype myint as int = int;

//// file2.php
<?hh

function expect_bool(bool $x): bool {
  return $x;
}

function pass_my_int(myint $y) : bool {
  return expect_bool($y);
}
