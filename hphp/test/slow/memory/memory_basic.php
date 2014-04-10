<?php

function main() {
  $alloc = memory_get_usage();
  $usage = memory_get_usage(true);
  $total_alloc = memory_get_allocation();

  if ($alloc < 0 || $usage < 0 || $total_alloc < 0) {
    var_dump('memory stats should never be negative!');
  }

  if ($alloc >= $total_alloc) {
    var_dump(
      'slab allocations should be less than total jemalloc allocations!'
    );
  }

  var_dump($alloc);
  var_dump($usage);
  var_dump($total_alloc);
}

main();
