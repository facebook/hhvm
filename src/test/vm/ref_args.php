<?php

// Tests a case where we are passing more args than a function takes
// to its reffiness guard.
error_reporting(0);
function blarg($a1, $a2, $a3, $a4, $a5, $a6, $a7, $a8, $a9, $a10, $a11, $a12,
               $a13, $a14, $a15, $a16, $a17, $a18, $a19, $a20, $a21, $a22,
               $a23, $a24, $a25, $a26, $a27, $a28, $a29, $a30, $a31, $a32,
               $a33, $a34, $a35, $a36, $a37, $a38, $a39, $a40, $a41, $a42,
               $a43, $a44, $a45, $a46, $a47, $a48, $a49, $a50, $a51, $a52,
               $a53, $a54, $a55, $a56, $a57, $a58, $a59, $a60, $a61, $a62,
               $a63, $a64, &$a65) {}
function nop() {}
function do_call($fn) {
  $fn($a1, $a2, $a3, $a4, $a5, $a6, $a7, $a8, $a9, $a10, $a11, $a12, $a13,
      $a14, $a15, $a16, $a17, $a18, $a19, $a20, $a21, $a22, $a23, $a24, $a25,
      $a26, $a27, $a28, $a29, $a30, $a31, $a32, $a33, $a34, $a35, $a36, $a37,
      $a38, $a39, $a40, $a41, $a42, $a43, $a44, $a45, $a46, $a47, $a48, $a49,
      $a50, $a51, $a52, $a53, $a54, $a55, $a56, $a57, $a58, $a59, $a60, $a61,
      $a62, $a63, $a64, $a65);
  var_dump(array_key_exists('a65', get_defined_vars()));
}
function main1() {
  do_call('blarg');
  do_call('nop');
}
main1();

// Similar case, but for a builtin (array_multisort).
function blarg2(&$a1, $a2) {}
function do_call2($fn) {
  $a2 = array();
  $fn($a1, $a2[0]);
  var_dump($a2);
}
function main2() {
  do_call2('blarg2');
  do_call2('array_multisort');
}
main2();

// sscanf is a builtin that takes all additional args by ref.
function main3() {
  sscanf("foo 12 bar", "%s %d %s", $a, $b, $c);
  echo "Foo: $a $b $c\n";
}
main3();

// array_multisort is weird.  Some arguments are literals.
function main4() {
  $x = array(1, 54, 3, 23, 5, 2);
  $y = array("a", "b", "c", "d", "e", "f");
  var_dump($x, $y);
  array_multisort($x, $y);
  var_dump($x, $y);
  array_multisort($x, SORT_DESC, $y);
  var_dump($x, $y);
}
main4();

function main5() {
  $params = array(array(3,2,1),array(4,6,5),array(7,9,8));
  call_user_func_array('array_multisort', $params);
  var_dump($params);
}
main5();

