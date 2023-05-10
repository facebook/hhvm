<?hh

<<__EntryPoint>>
function main_duplicate_keys_no_leak() {
  // Prime the JSON engine.
  json_decode('{"a":"1","a":"2"}', true, 512, JSON_FB_LOOSE);

  // This test is really noisy.  The intent is to look for leaks in the JSON
  // decoding. The current min limit at this time is 215744 so we'll make it
  // somewhat higher to account for variance.
  $limit = memory_get_usage(true) + 300000;
  ini_set('memory_limit', $limit);

  // Run enough iterations that if we leak even 1 byte per loop we'll break the
  // limit.
  for ($i = 0; $i < 1000000; $i++) {
    $x = json_decode('{"a":"1","a":"2"}', true, 512, JSON_FB_LOOSE);
    // Force memory manager to update accounting.
    __hhvm_intrinsics\memory_manager_stats();
    __hhvm_intrinsics\launder_value($x);
  }
  var_dump($x);
}
