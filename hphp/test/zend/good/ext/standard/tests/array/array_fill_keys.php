<?hh

function _try($fn) :mixed{
  try {
    return $fn();
  } catch (Exception $e) {
    echo get_class($e).': '.$e->getMessage().PHP_EOL;
    return null;
  }
}

<<__EntryPoint>>
function main(): void {
  var_dump(_try(() ==> array_fill_keys('test', 1)));
  var_dump(array_fill_keys(vec[], 1));
  var_dump(array_fill_keys(vec['foo', 'bar'], NULL));
  var_dump(array_fill_keys(vec['5', 'foo', 10, '1.23'], 123));
  var_dump(array_fill_keys(vec['test', 10, 100], ''));
}
