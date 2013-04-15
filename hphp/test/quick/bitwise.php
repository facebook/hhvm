<?php
function f() {
  $x = 204; // 11001100 in binary
  $y = 170; // 10101010 in binary
  echo ($x ^ $y); // 01100110 in binary
  echo "\n";
  echo ($x & $y);
  echo "\n";
  echo ($x | $y);
  echo "\n";
  echo (~$x);
  echo "\n";
}
f();


// Pairwise probe.
function probe($l, $r) {
  echo "-------\n";
  echo "left: ";  var_dump($l);
  echo "right: "; var_dump($r);
  $v = ($l & $r); var_dump($v);
  $v = ($l | $r); var_dump($v);
  $v = ($l ^ $r); var_dump($v);
  $v = (~$l);     var_dump($v);
}

function main() {
  $i = 0x3;
  $data = array(15, "7", "not an int. at all.");
  foreach ($data as $left) {
    foreach ($data as $right) {
      probe($left, $right);
    }
  }
}

main();
