<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f($x) {
  if (($x & 15) != 0) {
    $x  = $x + 3;
    if ($x == 991919188238838) {
      goto harmful;
    }
  }
  $x++;
  harmful:
  return $x;
}

for ($i = 0; $i < 20; $i++) {
  $y = f($i);
  var_dump($y);
}
