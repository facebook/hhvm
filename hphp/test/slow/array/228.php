<?php

$foo = array(1,2,3,4);
foreach ($foo as $key => $val) {
  if($val == 2) {
    $foo[$key] = 0;
  }
 else if($val == 3) {
    unset($foo[$key]);
  }
 else {
    $foo[$key] = $val + 1;
  }
}
var_dump($foo);
