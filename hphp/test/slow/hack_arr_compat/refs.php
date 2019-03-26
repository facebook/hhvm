<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$d) {}

function ref_param($name, $orig) {
  echo "========== ref_param ($name) ===============================\n";
  $a = $orig;
  pass_by_ref(&$a[3]);
  $a = $orig;
  pass_by_ref(&$a[4]);
  $a = $orig;
  pass_by_ref(&$a[10]);
  $a = $orig;
  pass_by_ref(&$a["key1"]);
  $a = $orig;
  pass_by_ref(&$a["key2"]);
}

function ref_unserialize() {
  echo "========== ref_unserialize =================================\n";
  $ref_str = "a:2:{s:3:\"foo\";D:1:{s:1:\"a\";s:1:\"b\";}s:3:\"bar\";R:2;}";
  unserialize($ref_str);
}

function run($name, $a) {
  ref_param($name, $a);
}

function main() {
  run("empty", []);
  run("packed", [1, 2, 3, 4]);
  run("mixed", [0 => 1, 1 => 2, 3 => 4, "key1" => "abc"]);

  // Make the array values unpredictable so that we don't get a static array
  // here. We want to get a APC local array when we retrieve it, which requires
  // storing a non-static array.
  $apc = [];
  $apc[0] = mt_rand();
  $apc[1] = mt_rand();
  $apc[3] = mt_rand();
  $apc["key1"] = "abc";
  apc_store("some_apc_array", $apc);
  run("apc", apc_fetch("some_apc_array"));

  ref_unserialize();
}

<<__EntryPoint>>
function main_refs() {
main();
}
