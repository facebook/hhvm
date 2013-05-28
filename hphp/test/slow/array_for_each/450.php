<?php

function f2() {
  $i = 0;
  $foo = array(1,2,3,4);
  foreach ($foo as $key => &$val) {
    yield null;
    echo "key=$key val=$val\n";
    if($val == 2) {
      $foo[$key] = 0;
    }
 else if($val == 3) {
      $foo['a'] = 7;
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
foreach (f2() as $_) {
}
