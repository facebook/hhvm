<?php

function f5() {
  $i = 0;
  $foo = array('f'=>3, 'e'=>1, 'd'=>5, 'a'=>6, 'b'=>2, 'c'=>4);
  $a = 0;
  foreach ($foo as $key => &$val) {
    yield null;
    echo "key=$key val=$val\n";
    if ($key == 'e' && $a == 0) {
      $a = 1;
      unset($foo['e']);
      unset($foo['d']);
      $foo['d'] = 9;
      for ($j = 0;
 $j < 10000;
 ++$j)
        $foo[$j . 's' . $j] = $j;
    }
    ++$i;
    if ($i >= 20)
      break;
  }
}
foreach (f5() as $_) {
}
