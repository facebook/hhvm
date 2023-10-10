<?hh

function array_map($func, $arr, ...$extra) {
  if ($extra) {
    return \__SystemLib\array_map($func, $arr, ...$extra);
  }

  if (!($arr is \HH\KeyedTraversable)) {
    return \__SystemLib\array_map($func, $arr);
  }

  if (is_object($arr)) {
    $arr = $arr->toDArray();
  }

  if ($func === null) {
    return $arr;
  }

  if (!is_callable($func)) {
    trigger_error(
      'array_map() expects parameter 1 to be a valid callback',
      E_USER_WARNING,
    );
    return null;
  }

  $res = darray[];
  foreach ($arr as $k => $v) {
    $res[$k] = $func($v);
  }

  return $res;
}
