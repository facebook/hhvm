<?hh
function handler($errno, $errstr, $errfile, $errline, $errcontext='',
                 $errtrace = array()) {

  if ($errstr === "Hack Array Compat: Comparing PHP array with Hack array") {
    HackArrCompatCompare::$got_notice_hack_array = true;
  }
  if ($errstr === "Hack Array Compat: Comparing PHP array with non any-array") {
    HackArrCompatCompare::$got_notice_non_any_array = true;
  }
}

function do_compare_hack_array($cmp) {
  HackArrCompatCompare::$got_notice_hack_array = false;
  $cmp();
  return HackArrCompatCompare::$got_notice_hack_array;
}

function do_compare_non_any_array($cmp) {
  HackArrCompatCompare::$got_notice_non_any_array = false;
  $cmp();
  return HackArrCompatCompare::$got_notice_non_any_array;
}

function exn_wrap($cmp) {
  try { $cmp(); } catch (Exception $e) {}
}

function do_compares($a, $b, $cmp) {
  echo "=========================== Notice Compare =======================\n";
  var_dump($a);
  var_dump($b);
  echo ($cmp(() ==> exn_wrap(() ==> $a < $b)) ? 'T' : 'F');
  echo " " . ($cmp(() ==> exn_wrap(() ==> $a <= $b)) ? 'T' : 'F');
  echo " " . ($cmp(() ==> exn_wrap(() ==> $a > $b)) ? 'T' : 'F');
  echo " " . ($cmp(() ==> exn_wrap(() ==> $a >= $b)) ? 'T' : 'F');
  echo " " . ($cmp(() ==> exn_wrap(() ==> $a <=> $b)) ? 'T' : 'F');
  echo " " . ($cmp(() ==> exn_wrap(() ==> $a == $b)) ? 'T' : 'F');
  echo " " . ($cmp(() ==> exn_wrap(() ==> $a != $b)) ? 'T' : 'F');
  echo " " . ($cmp(() ==> exn_wrap(() ==> $a === $b)) ? 'T' : 'F');
  echo " " . ($cmp(() ==> exn_wrap(() ==> $a !== $b)) ? 'T' : 'F');
  echo "\n==================================================================\n";
}

function main() {
  set_error_handler(fun('handler'));

  $x1 = [
    [],
    [1, 2, [3, 4]],
    ['a' => 'b', 'c' => 'd']
  ];
  $x2_non_hack_arrays = [
    true,
    false,
    null,
    123,
    4.567,
    'abc',
    new stdclass,
    imagecreate(1, 1),
    [1, [2, 5], [3, 4]],
    ['a' => [], 'c' => [1, 2]],
  ];
  $x2_hack_arrays = [
    vec[],
    vec[1, 2, 3],
    dict[],
    dict['a' => 'b', 'c' => 'd'],
    keyset[],
    keyset['a', 'b', 'c'],
    [1, 2, vec[3, 4]],
  ];

  $do_compare_hack_array_listener
    = $thunk ==> do_compare_hack_array($thunk);
  $do_compare_non_any_array_listener
    = $thunk ==> do_compare_non_any_array($thunk);

  foreach ($x1 as $a) {
    foreach ($x2_non_hack_arrays as $b) {
      do_compares($a, $b, $do_compare_non_any_array_listener);
      do_compares($b, $a, $do_compare_non_any_array_listener);
    }
    foreach ($x2_hack_arrays as $b) {
      do_compares($a, $b, $do_compare_hack_array_listener);
      do_compares($b, $a, $do_compare_hack_array_listener);
    }
  }

  do_compares(null, null, $do_compare_hack_array_listener);
  do_compares(true, false, $do_compare_hack_array_listener);
  do_compares(1, 2, $do_compare_hack_array_listener);

  do_compares(null, null, $do_compare_non_any_array_listener);
  do_compares(true, false, $do_compare_non_any_array_listener);
  do_compares(1, 2, $do_compare_non_any_array_listener);
}

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_compare() {
$got_notice = false;

main();
}

abstract final class HackArrCompatCompare {
  public static $got_notice_hack_array;
  public static $got_notice_non_any_array;
}
