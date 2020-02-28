<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function basic_getters($arr, $keys) {
  echo "======================== get =================================\n";
  foreach ($keys as $key) { $arr[$key]; }

  echo "======================== null coalesce =======================\n";
  foreach ($keys as $key) { $arr[$key] ?? "FOO"; }

  echo "======================== idx =================================\n";
  foreach ($keys as $key) { idx($arr, $key, "FOO"); }

  echo "======================== isset ===============================\n";
  foreach ($keys as $key) { isset($arr[$key]); }

  echo "======================== array key exists ====================\n";
  foreach ($keys as $key) { array_key_exists($key, $arr); }

  echo "======================== empty ===============================\n";
  foreach ($keys as $key) { !($arr[$key] ?? false); }
}

function basic_setters($arr, $keys) {
  echo "======================== set =================================\n";
  foreach ($keys as $key) { $arr[$key] = 123; }

  echo "======================== set-op ==============================\n";
  foreach ($keys as $key) { $arr[$key] += 123; }

  echo "======================== unset ===============================\n";
  foreach ($keys as $key) { unset($arr[$key]); }
}

function member_ops($arr, $keys) {
  $copy = $arr;

  echo "======================== base-elem ===========================\n";
  foreach ($keys as $key) { $arr[$key][0]; }

  echo "======================== base-elem no-warn ===================\n";
  foreach ($keys as $key) { $arr[$key][0] ?? "FOO"; }

  echo "======================== base-define =========================\n";
  foreach ($keys as $key) { $arr[$key][0] = 123; }

  echo "======================== base-unset ==========================\n";
  foreach ($keys as $key) { unset($arr[$key][0]); }

  $arr = $copy;

  echo "======================== dim-elem ============================\n";
  foreach ($keys as $key) { $arr[0][$key][0]; }

  echo "======================== dim-elem no-warn ====================\n";
  foreach ($keys as $key) { $arr[0][$key][0] ?? "FOO"; }

  echo "======================== dim-define ==========================\n";
  foreach ($keys as $key) { $arr[0][$key][0] = 123; }

  echo "======================== dim-unset ===========================\n";
  foreach ($keys as $key) { unset($arr[0][$key][0]); }

  $arr = $copy;

  echo "======================== fini-get ============================\n";
  foreach ($keys as $key) { $arr[0][1][$key]; }

  echo "======================== fini-get no-warn ====================\n";
  foreach ($keys as $key) { $arr[0][1][$key] ?? "FOO"; }

  echo "======================== fini-isset ==========================\n";
  foreach ($keys as $key) { isset($arr[0][1][$key]); }

  echo "======================== fini-empty ==========================\n";
  foreach ($keys as $key) { !($arr[0][1][$key] ?? false); }

  echo "======================== fini-set ============================\n";
  foreach ($keys as $key) { $arr[0][1][$key] = 123; }

  echo "======================== fini-set-op =========================\n";
  foreach ($keys as $key) { $arr[0][1][$key] += 123; }

  echo "======================== fini-unset ==========================\n";
  foreach ($keys as $key) { unset($arr[0][1][$key]); }
}


<<__EntryPoint>>
function main_implicit_key() {
$sub = __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[0 => 100, 1 => 100, "" => 100]);
$arr = __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[
  0 => __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[
    0 => $sub,
    1 => $sub,
    "" => $sub
  ]),
  1 => __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[
    0 => $sub,
    1 => $sub,
    "" => $sub
  ]),
  "" => __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[
    0 => $sub,
    1 => $sub,
    "" => $sub
  ])
]);
$keys = __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[null, true, false, 1.5]);
basic_getters($arr, $keys);
basic_setters($arr, $keys);
member_ops($arr, $keys);
}
