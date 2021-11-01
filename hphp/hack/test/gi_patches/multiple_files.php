//// file1.php
<?hh

enum E : int {
  X = 1;
  Y = 2;
}

//// file2.php
<?hh

/* HH_FIXME[4030] */
function getX() {
  return E::X;
}
