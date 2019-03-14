<?php


// Github Issue #1436: Return type of htmlspecialchars() when
// wrong type passed.

<<__EntryPoint>>
function main_1436_htmlspecialchars() {
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
  try { var_dump(htmlspecialchars($input, ENT_QUOTES, 'UTF-8')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
}
