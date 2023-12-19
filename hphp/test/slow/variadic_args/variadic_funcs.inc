<?hh

function compare($args, $args_manual, $var_args, $var_arg_start = 0) {
  $success = true;
  foreach ($var_args as $i => $val) {
    $n = $i + $var_arg_start;
    $args_val = $args_manual[$n];
    if ($args_val !== $val) {
      echo "func_get_arg({$n}) is: {$args_val} expected: {$val}", "\n";
      $success = false;
    }
    $args_val = $args[$n];
    if ($args_val !== $val) {
      echo "func_get_args()[{$n}] is: {$args_val} expected: {$val}", "\n";
      $success = false;
    }
  }
  return $success;
}

<<__DynamicallyCallable>>
function variadic_only_no_vv(...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(is_array($v));
  var_dump($v);
}

<<__DynamicallyCallable>>
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

<<__DynamicallyCallable>>
function variadic_only_exc(...$v) {
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

<<__DynamicallyCallable>>
function variadic_only(...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(is_array($v));
  var_dump($v);
}

<<__DynamicallyCallable>>
function variadic_some($x, ...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(isset($x));
  var_dump(is_array($v));
  var_dump($v);
}

<<__DynamicallyCallable>>
function variadic_hack_only(...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(count($v));
  var_dump($v);
}

<<__DynamicallyCallable>>
function variadic_hack_some($x, ...$v) {
  echo "\n", '* ', __FUNCTION__, "\n";
  var_dump(1 + count($v));
  var_dump(array_merge(vec[$x], $v));
}

class C {
  <<__DynamicallyCallable>>
  public function variadic_only(...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump($v);
  }

  <<__DynamicallyCallable>>
  public function variadic_some($x, ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(isset($x));
    var_dump($v);
  }

  <<__DynamicallyCallable>>
  public function variadic_hack_only(...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump($v);
  }

  <<__DynamicallyCallable>>
  public function variadic_hack_some($x, ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(array_merge(vec[$x], $v));
  }

  <<__DynamicallyCallable>>
  public static function st_variadic_only(...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump($v);
  }

  <<__DynamicallyCallable>>
  public static function st_variadic_some($x, ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump($v);
  }

  <<__DynamicallyCallable>>
  public static function st_variadic_hack_only(...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump($v);
  }

  <<__DynamicallyCallable>>
  public static function st_variadic_hack_some($x, ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(array_merge(vec[$x], $v));
  }
}
