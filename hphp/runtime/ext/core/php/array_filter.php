<?hh

function array_filter(\HH\KeyedTraversable $arr, $func = null, $flag = null) {
  if ($func !== null && !is_callable($func)) {
    trigger_error(
      'array_filter() expects parameter 2 to be a valid callback',
      E_WARNING,
    );
    return null;
  }

  $res = dict[];
  if ($func === null) {
    foreach ($arr as $k => $v) {
      if ($v) {
        $res[$k] = $v;
      }
    }
  } else if ($flag === ARRAY_FILTER_USE_BOTH) {
    foreach ($arr as $k => $v) {
      if ($func($v, $k)) {
        $res[$k] = $v;
      }
    }
  } else if ($flag === ARRAY_FILTER_USE_KEY) {
    foreach ($arr as $k => $v) {
      if ($func($k)) {
        $res[$k] = $v;
      }
    }
  } else {
    foreach ($arr as $k => $v) {
      if ($func($v)) {
        $res[$k] = $v;
      }
    }
  }

  return $res;
}
