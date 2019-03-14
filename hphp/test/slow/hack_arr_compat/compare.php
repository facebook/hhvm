<?hh
function handler($errno, $errstr, $errfile, $errline, $errcontext='',
                 $errtrace = array()) {

  if ($errstr === "Hack Array Compat: Comparing array with non-array") {
    HackArrCompatCompare::$got_notice = true;
  }
}

function do_compare($cmp) {

  HackArrCompatCompare::$got_notice = false;
  $cmp();
  return HackArrCompatCompare::$got_notice;
}

function exn_wrap($cmp) {
  try { $cmp(); } catch (Exception $e) {}
}

function do_compares($a, $b) {
  echo "=========================== Notice Compare =======================\n";
  var_dump($a);
  var_dump($b);
  echo (do_compare(() ==> exn_wrap(() ==> $a < $b)) ? 'T' : 'F');
  echo " " . (do_compare(() ==> exn_wrap(() ==> $a <= $b)) ? 'T' : 'F');
  echo " " . (do_compare(() ==> exn_wrap(() ==> $a > $b)) ? 'T' : 'F');
  echo " " . (do_compare(() ==> exn_wrap(() ==> $a >= $b)) ? 'T' : 'F');
  echo " " . (do_compare(() ==> exn_wrap(() ==> $a <=> $b)) ? 'T' : 'F');
  echo " " . (do_compare(() ==> exn_wrap(() ==> $a == $b)) ? 'T' : 'F');
  echo " " . (do_compare(() ==> exn_wrap(() ==> $a != $b)) ? 'T' : 'F');
  echo " " . (do_compare(() ==> exn_wrap(() ==> $a === $b)) ? 'T' : 'F');
  echo " " . (do_compare(() ==> exn_wrap(() ==> $a !== $b)) ? 'T' : 'F');
  echo "\n==================================================================\n";
}

function main() {
  set_error_handler('handler');

  $x1 = [
    [],
    [1, 2, [3, 4]],
    ['a' => 'b', 'c' => 'd']
  ];
  $x2 = [
    true,
    false,
    null,
    123,
    4.567,
    'abc',
    vec[],
    vec[1, 2, 3],
    dict[],
    dict['a' => 'b', 'c' => 'd'],
    keyset[],
    keyset['a', 'b', 'c'],
    new stdclass,
    imagecreate(1, 1),
    [1, [2, 5], [3, 4]],
    [1, 2, vec[3, 4]],
    ['a' => [], 'c' => [1, 2]]
  ];

  foreach ($x1 as $b) {
    foreach ($x2 as $a) {
      do_compares($a, $b);
      do_compares($b, $a);
    }
  }

  foreach ($x1 as $a) {
    foreach ($x2 as $b) {
      do_compares($a, $b);
    }
  }

  do_compares(null, null);
  do_compares(true, false);
  do_compares(1, 2);
}

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_compare() {
$got_notice = false;

main();
}

abstract final class HackArrCompatCompare {
  public static $got_notice;
}
