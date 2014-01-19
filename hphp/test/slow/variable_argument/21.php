<?php

function test($a, $b = 10) {
   var_dump($a);
  var_dump($b);
  var_dump(func_get_args());
}
 test(1);
 test(1, 2);
 test(1, 2, 3);
