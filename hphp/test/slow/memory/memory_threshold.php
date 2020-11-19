<?hh

<<__Memoize>>
function getCount() : int {
  $success = false;
  $count = apc_fetch("count", inout $success);
  if (!$success) {
    $count = 0;
  } else {
    $count++;
  }
  apc_store("count", $count);
  return $count;
}

function forceRefreshStats() : void {
  // We only refresh stats upon allocation of a new slab.
  $bigVal = "";
  for ($i = 0; $i < 10000; $i++) {
    $bigVal .= "asdfasdfasdfadsfasdfadsfasasdfadsfadsfadsfadsfafdasfasdfasd";
  }
}

function memCallback() : void {
  echo "  Mem callback fired UHOH: " . memory_get_usage(true) . "\n";
}

<<__EntryPoint>>
function main() : void {
  $int_max = 0x7FFFFFFFFFFFFFFF;
  HH\set_mem_threshold_callback($int_max, memCallback<>);
  echo "Iteration " . getCount() . "\n";
  if (getCount() < 20) {
    // Store garbage.
    $bigVal = "";
    for ($i = 0; $i < 10000; $i++) {
      $bigVal .= "asdfasdfasdfadsfasdfadsfasasdfadsfadsfadsfadsfafdasfasdfasd";
    }
    apc_store((string)rand(), $bigVal);
    echo "  Memory usage " . memory_get_usage(true) . "\n";
  } else {
    // Purge
    apc_clear_cache();
    echo "  Memory usage " . memory_get_usage(true) . "\n";
  }
  forceRefreshStats();
}
