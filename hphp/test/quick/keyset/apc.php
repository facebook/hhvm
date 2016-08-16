<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function get_count() {
  $count = apc_fetch("count");
  if (!$count) {
    $count = 0;
  }
  $count++;
  apc_store("count", $count);
  return $count;
}

function read() {
  var_dump(apc_fetch("val1"));
  var_dump(apc_fetch("val2"));
  var_dump(apc_fetch("val3"));
  var_dump(apc_fetch("val4"));
}

function write($count) {
  apc_store("val1", keyset[]);
  apc_store("val2", keyset[1, 2, 3]);
  apc_store("val3", keyset["a", "b", "C", "D"]);
  apc_store("val4", keyset[$count, $count+1]);
}

function main() {
  $count = get_count();
  echo "Count: $count\n";
  read();
  write($count);
  read();
}
main();
