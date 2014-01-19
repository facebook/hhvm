<?php

function f1() {
  $i = 0;
  $foo = array(1,2,3,4);
  foreach ($foo as $key => &$val) {
    yield null;
    echo "key=$key val=$val\n";
    if($val == 2) {
      $foo[$key] = 0;
    }
 else if($val == 3) {
      unset($foo[$key]);
    }
 else {
      $val++;
    }
    ++$i;
    if ($i >= 20)
      break;
  }
  var_dump($foo);
}
foreach (f1() as $_) {
}
