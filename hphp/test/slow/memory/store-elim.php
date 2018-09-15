<?php

function main($a, $x) {
  $s = $x;
  array_chunk($a, 0);
  $s = 0;
}

function handler() {
  var_dump(func_get_args()[4]);
  exit(0);
}


<<__EntryPoint>>
function main_store_elim() {
set_error_handler('handler');

main(0, 42);
}
