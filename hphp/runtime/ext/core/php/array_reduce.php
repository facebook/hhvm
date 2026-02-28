<?hh

function array_reduce(\HH\KeyedTraversable $input, $func, $res = null) {
  if (!is_callable($func)) {
    trigger_error(
      'array_reduce() expects parameter 2 to be a valid callback',
      E_USER_WARNING,
    );
    return null;
  }

  foreach ($input as $v) {
    $res = $func($res, $v);
  }
  return $res;
}
