<?hh

<<__EntryPoint>>
function main_duplicate_keys_no_leak() {
  json_decode('{"a":"1","a":"2"}', true, 512, JSON_FB_LOOSE);
  $limit = memory_get_usage(true) + 100000;
  ini_set('memory_limit', $limit);
  for ($i = 0; $i < 100000; $i++) {
    $x = json_decode('{"a":"1","a":"2"}', true, 512, JSON_FB_LOOSE);
    __hhvm_intrinsics\memory_manager_stats(); // Force memory manager
                                              // to update accounting.
    __hhvm_intrinsics\launder_value($x);
  }
  var_dump($x);
}
