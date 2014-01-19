<?php

function gen() {
  $foo = array(1,2,3,4);
  foreach ($foo as $key => &$val) {
    if($val == 2) {
      $foo[$key] = 0;
    }
 else if($val == 3) {
      unset($foo[$key]);
    }
 else {
      $val++;
    }
  }
  var_dump($foo);
  yield null;
}
foreach (gen() as $_) {
}
