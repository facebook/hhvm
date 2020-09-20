<?hh

function test_is_nan($v) {
  try { var_dump(is_nan($v)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}


<<__EntryPoint>>
function main_bad_coercion() {
$values = varray[
  0,
  '0',
  '0a',
  ' 0a',
  ' 0',
  '00:00:01',
];

foreach ($values as $v) {
  test_is_nan($v);
}
}
