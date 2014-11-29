<?php

function assert_in_function() {
  assert_options(ASSERT_ACTIVE, true);
  $value = '2';
  var_dump(assert(is_string($value)));
  var_dump(assert('is_string($value)'));
}
assert_options(ASSERT_ACTIVE, true);
$value = '1';
var_dump(assert(is_string($value)));
var_dump(assert('is_string($value)'));
var_dump(assert('is_array($value)'));
assert_in_function();
