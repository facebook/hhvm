<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function pass_by_ref(&$d) {}

function &ret_by_ref($d, $key) { return $d[$key]; }

function ref_param($name, $orig) {
  echo "========== ref_param ($name) ===============================\n";
  $a = $orig;
  pass_by_ref($a[3]);
  $a = $orig;
  pass_by_ref($a[4]);
  $a = $orig;
  pass_by_ref($a[10]);
  $a = $orig;
  pass_by_ref($a["key1"]);
  $a = $orig;
  pass_by_ref($a["key2"]);
}

function elem_ref($name, $orig) {
  echo "========== elem_ref ($name) ================================\n";
  $a = $orig;
  $elem = &$a[3];
  $a = $orig;
  $elem = &$a[4];
  $a = $orig;
  $elem = &$a[10];
  $a = $orig;
  $elem = &$a["key1"];
  $a = $orig;
  $elem = &$a["key2"];
}

function append_ref($name, $a) {
  echo "========== append_ref ($name) ==============================\n";
  $value = "some-value";
  $a[] = &$value;
}

function set_ref($name, $orig) {
  echo "========== set_ref ($name) =================================\n";
  $value = "some-value";
  $a = $orig;
  $a[3] = &$value;
  $a = $orig;
  $a[4] = &$value;
  $a = $orig;
  $a[10] = &$value;
  $a = $orig;
  $a["key1"] = &$value;
  $a = $orig;
  $a["key2"] = &$value;
}

function ref_return($name, $orig) {
  echo "========== ref_return ($name) ==============================\n";
  $a = $orig;
  ret_by_ref($a, 3);
  $a = $orig;
  ret_by_ref($a, 4);
  $a = $orig;
  ret_by_ref($a, 10);
  $a = $orig;
  ret_by_ref($a, "key1");
  $a = $orig;
  ret_by_ref($a, "key2");
}

function iterate_by_ref($name, $orig) {
  echo "========== iterate_by_ref ($name) ==========================\n";
  $a = $orig;
  foreach ($a as &$value) { }
  $a = $orig;
  foreach ($a as $key => &$value) { }
}

function ref_unserialize() {
  echo "========== ref_unserialize =================================\n";
  $ref_str = "a:2:{s:3:\"foo\";D:1:{s:1:\"a\";s:1:\"b\";}s:3:\"bar\";R:2;}";
  unserialize($ref_str);
}

function run($name, $a) {
  ref_param($name, $a);
  elem_ref($name, $a);
  append_ref($name, $a);
  set_ref($name, $a);
  ref_return($name, $a);
  iterate_by_ref($name, $a);
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
main();
