<?php

function decieve_static_analysis() {
  return mt_rand() ? 'a' : 'b';
}

function main() {
  var_dump(memory_get_usage());
  $slab_size = memory_get_usage();
  $a = array();
  for ($i = 0; $i < $slab_size / 1000; $i++) {
    $a[] = str_repeat(decieve_static_analysis(), 1000);
  }
  // One day we can compare this to the previous version and get a bigger number
  var_dump(memory_get_usage());
}
main();
