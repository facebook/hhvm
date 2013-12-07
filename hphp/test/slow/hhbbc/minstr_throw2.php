<?php

function err() {
  static $x = 0;
  if (++$x == 2) throw new Exception('asd');
}

set_error_handler('err');

function main() {
  $x = null;

  try {
    // The second DIM (intermediate op) will throw an exception from
    // the error handler here.  Test that the state with $x already
    // promoted to array is propagated to the catch block.
    $x[0]->foo->bar = 2;
  } catch (Exception $l) {
    var_dump($x);
  }
}

main();

