<?php

// Github Issue #1436: Return type of htmlspecialchars() when
// wrong type passed.

$inputs = array(
  'foo<>bar',
  '',
  null,
  array('str' => 'aaa'),
  42,
  1.234
);

foreach ($inputs as $input) {
  var_dump($input);
  var_dump(htmlspecialchars($input, ENT_QUOTES, 'UTF-8'));
}
