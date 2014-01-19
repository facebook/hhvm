<?php

function funk($alice, $bob) {
  echo "Args: $alice $bob\n";
  debug_print_backtrace();
}


function main() {
  $use_by_val = 123;
  $use_by_ref = 1000;

  $c = function($arg, $dv = 500) use ($use_by_val, &$use_by_ref) {
    $use_by_val *= 2;
    $use_by_ref *= 2;
    echo "Use: $use_by_val $use_by_ref\n";
    funk($arg, $dv);
  };

  if (!($c instanceof Closure)) {
    echo "Closure isn't instanceof closure!\n";
  }
  if (!is_callable($c)) {
    echo "Closure isn't callable!\n";
  }

  $c(777);
  call_user_func($c, 888);
  var_dump($c);
  var_dump($use_by_val, $use_by_ref);
}

main();
