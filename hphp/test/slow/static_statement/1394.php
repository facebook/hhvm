<?php

function test_unset_static() {
  static $static_var;
  $static_var ++;
  echo "value of static_var before unset: $static_var\n";
  var_dump( isset($static_var) );
  var_dump( empty($static_var) );
  unset($static_var);
  echo "value of static_var after unset: $static_var\n";
  var_dump( isset($static_var) );
  var_dump( empty($static_var) );
  $static_var = 20;
  echo "value of static_var after new assignment: $static_var\n";
}
test_unset_static();
test_unset_static();
test_unset_static();
