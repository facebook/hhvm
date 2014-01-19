<?php

function test() {
   var_dump(func_get_arg(0));
  var_dump(func_get_arg(1));
  var_dump(func_get_arg(2));
  var_dump(func_get_arg(3));
}
 test(2, 'ok', 0, 'test');
