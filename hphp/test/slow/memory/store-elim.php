<?php

set_error_handler('handler');

function main($a, $x) {
  $s = $x;
  array_chunk($a, 0);
  $s = 0;
}

function handler() {
  var_dump(func_get_args()[4]);
  exit(0);
}

main(0, 42);
