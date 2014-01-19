<?php

function test($a, $b) {
   $n = func_num_args();
   var_dump($n);
  $args = func_get_args();
  var_dump($args);
}
 test(1, 2);
 test(1, 2, 3);
 test(1, 2, 3, 4);
