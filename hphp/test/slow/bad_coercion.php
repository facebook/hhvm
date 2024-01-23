<?hh

function test_is_nan($v) :mixed{
  try { var_dump(is_nan($v)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}


<<__EntryPoint>>
function main_bad_coercion() :mixed{
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
});

  $values = vec[
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
