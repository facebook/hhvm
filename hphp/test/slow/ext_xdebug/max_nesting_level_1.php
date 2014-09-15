<?php

function foo($i) {
  echo "$i\n";
  if ($i === 10) {
    return;
  } else if ($i === 3) {
    ini_set("hhvm.xdebug-not-done.max_nesting_level", 0);
  }
  foo($i + 1);
}

$i = xdebug_get_stack_depth();
echo "$i\n";
ini_set("hhvm.xdebug-not-done.max_nesting_level", $i + 4);
foo($i + 1);
