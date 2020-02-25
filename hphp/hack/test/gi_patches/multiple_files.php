//// file1.php
<?hh

enum E : int {
  X = 1;
  Y = 2;
}

//// file2.php
<?hh // partial

function getX() {
  return E::X;
}
