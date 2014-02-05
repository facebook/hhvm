<?php

function err($x, $y) { throw new Exception('heh'); }

function foo() {
  echo "----\n";
  $lol = new stdclass;
  try {
    $x[$lol] = 2;
  } catch (Exception $y) {
    echo "after a throw:\n";
    set_error_handler(null);
    var_dump($x);
    return;
  }
  var_dump($x);
}

foo();
set_error_handler('err');
foo();
