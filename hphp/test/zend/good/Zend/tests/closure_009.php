<?php
$a = 1;
$x = function ($x) use ($a) {
  static $n = 0;
  $n++;
  $a = $n.':'.$a;
  echo $x.':'.$a."\n";
};
$x(1);
$x(2);
$x(3);
?>
