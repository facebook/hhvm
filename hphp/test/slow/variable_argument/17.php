<?php

function test($a) {
   $n = func_num_args();
   var_dump($n);
  $args = func_get_args();
  var_dump($args);
}
 test('test');
 test(1, 2);
 test(1, 2, 3);
