<?php

function main($a, $b, $c, $d) {
  $x = abs($a);
  $y = abs($b);
  $z = abs($c);
  $t = abs($d);

  var_dump($x);
  var_dump($y);
  var_dump($z);
  var_dump($t);
}

main(5, -5, 5.5, -5.5);
main(17293822569102704641, -17293822569102704641,
     4611686018427387904, -4611686018427387904);
main(0, 0.0, -0.0, false);
main(array(), array(1), new stdClass, true);

