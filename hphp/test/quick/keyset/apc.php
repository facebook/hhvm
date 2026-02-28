<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function get_count() :mixed{
  $count = __hhvm_intrinsics\apc_fetch_no_check("count");
  if (!$count) {
    $count = 0;
  }
  $count++;
  apc_store("count", $count);
  return $count;
}

function read() :mixed{
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val1"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val2"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val3"));
  var_dump(__hhvm_intrinsics\apc_fetch_no_check("val4"));
}

function write($count) :mixed{
  apc_store("val1", keyset[]);
  apc_store("val2", keyset[1, 2, 3]);
  apc_store("val3", keyset["a", "b", "C", "D"]);
  apc_store("val4", keyset[$count, $count+1]);
}

<<__EntryPoint>> function main(): void {
  $count = get_count();
  echo "Count: $count\n";
  read();
  write($count);
  read();
}
