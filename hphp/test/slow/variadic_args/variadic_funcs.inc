<?hh

function compare($args, $args_manual, $var_args, $var_arg_start = 0) {
  $success = true;
  foreach ($var_args as $i => $val) {
    $n = $i + $var_arg_start;
    $args_val = $args_manual[$n];
    if ($args_val !== $val) {
      echo "func_get_arg(${n}) is: ${args_val} expected: ${val}", "\n";
      $success = false;
    }
    $args_val = $args[$n];
    if ($args_val !== $val) {
      echo "func_get_args()[${n}] is: ${args_val} expected: ${val}", "\n";
      $success = false;
    }
  }
  return $success;
}

function variadic_only_no_vv(...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(is_array($v));
  var_dump($v);
}

function variadic_only_no_vv_exc(...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(is_array($v));
  var_dump($v);
  $trace = debug_backtrace();
  foreach ($trace as $row) {
    echo $row['function'], ': ';
    print_r($row['args']);
  }
  throw new Exception(__FUNCTION__.': '.count($v));
}

function variadic_only_exc(...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  $args = func_get_args();
  var_dump(is_array($v));
  var_dump($v);
  $trace = debug_backtrace();
  foreach ($trace as $row) {
    echo $row['function'], ': ';
    print_r($row['args']);
  }
  throw new Exception(__FUNCTION__.': '.count($v));
}

function variadic_only(...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  $var_arg_offset = 0;
  var_dump(is_array($v));
  var_dump($v);
  var_dump($v === array_slice(func_get_args(), $var_arg_offset));
  $manual = array();
  for ($i = 0; $i < func_num_args(); $i++) {
    $manual[$i] = func_get_arg($i);
  }
  var_dump(compare(func_get_args(), $manual, $v, $var_arg_offset));
}

function variadic_some($x, ...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  $var_arg_offset = 1;
  var_dump(isset($x));
  var_dump(is_array($v));
  var_dump($v);
  var_dump($v === array_slice(func_get_args(), $var_arg_offset));

  $manual = array();
  for ($i = 0; $i < func_num_args(); $i++) {
    $manual[$i] = func_get_arg($i);
  }
  var_dump(compare(func_get_args(), $manual, $v, $var_arg_offset));
}

function variadic_hack_only(...) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(func_num_args());
  var_dump(func_get_args());
}

function variadic_hack_some($x, ...) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(func_num_args());
  var_dump(func_get_args());
}

class C {
  public function variadic_only(...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    $var_arg_offset = 0;
    var_dump($v);
    var_dump($v === array_slice(func_get_args(), $var_arg_offset));
    $manual = array();
    for ($i = 0; $i < func_num_args(); $i++) {
      $manual[$i] = func_get_arg($i);
    }
    var_dump(compare(func_get_args(), $manual, $v, $var_arg_offset));
  }

  public function variadic_some($x, ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    $var_arg_offset = 1;
    var_dump(isset($x));
    var_dump($v);
    var_dump($v === array_slice(func_get_args(), $var_arg_offset));
    $manual = array();
    for ($i = 0; $i < func_num_args(); $i++) {
      $manual[$i] = func_get_arg($i);
    }
    var_dump(compare(func_get_args(), $manual, $v, $var_arg_offset));
  }

  public function variadic_hack_only(...) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
  }

  public function variadic_hack_some($x, ...) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
  }

  public static function st_variadic_only(...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    $var_arg_offset = 0;
    var_dump($v);
    var_dump($v === array_slice(func_get_args(), $var_arg_offset));
    $manual = array();
    for ($i = 0; $i < func_num_args(); $i++) {
      $manual[$i] = func_get_arg($i);
    }
    var_dump(compare(func_get_args(), $manual, $v, $var_arg_offset));
  }

  public static function st_variadic_some($x, ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    $var_arg_offset = 1;
    var_dump($v);
    var_dump($v === array_slice(func_get_args(), $var_arg_offset));
    $manual = array();
    for ($i = 0; $i < func_num_args(); $i++) {
      $manual[$i] = func_get_arg($i);
    }
    var_dump(compare(func_get_args(), $manual, $v, $var_arg_offset));
  }

  public static function st_variadic_hack_only(...) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
  }

  public static function st_variadic_hack_some($x, ...) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_get_args());
  }
}
