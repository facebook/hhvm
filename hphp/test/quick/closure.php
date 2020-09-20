<?hh

function funk($alice, $bob) {
  echo "Args: $alice $bob\n";
  debug_print_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS);
}

<<__EntryPoint>>
function main() {
  $use_by_val = 123;

  $c = function($arg, $dv = 500) use ($use_by_val) {
    $use_by_val *= 2;
    echo "Use: $use_by_val\n";
    funk($arg, $dv);
  };

  if (!($c is Closure)) {
    echo "Closure isn't instanceof closure!\n";
  }
  if (!is_callable($c)) {
    echo "Closure isn't callable!\n";
  }

  $c(777);
  call_user_func($c, 888);
  var_dump($c);
  var_dump($use_by_val);
  $debuginfo = $c->__debuginfo();
  var_dump(is_darray($debuginfo));
  var_dump(is_darray($debuginfo['static']));
  var_dump(is_darray($debuginfo['parameter']));
}
