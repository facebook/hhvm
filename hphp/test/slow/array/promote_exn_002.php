<?php

function err($x, $y) { throw new Exception('heh'); }

class bar {}

function foo() {
  $bar = new bar;
  echo "----\n";
  $lol = new stdclass;
  try {
    $bar->x[$lol] = 2;
  } catch (Exception $y) {
    echo "after a throw:\n";
    set_error_handler(null);
    var_dump($bar);
    return;
  }
  var_dump($bar);
}

foo();
set_error_handler('err');
foo();
