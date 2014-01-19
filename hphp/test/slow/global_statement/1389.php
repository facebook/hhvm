<?php

$global_var = 10;
function test_unset() {
  global $global_var;
  var_dump( isset($global_var) );
  var_dump( empty($global_var) );
  unset($global_var);
  var_dump( isset($global_var) );
  var_dump( empty($global_var) );
 }
var_dump($global_var);
test_unset();
var_dump($global_var);
