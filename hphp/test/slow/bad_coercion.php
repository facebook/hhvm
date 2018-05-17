<?php

function test_is_nan($v) {
  var_dump(is_nan($v));
}

$values = array(
  0,
  '0',
  '0a',
  ' 0a',
  ' 0',
  '00:00:01',
);

foreach ($values as $v) {
  test_is_nan($v);
}
