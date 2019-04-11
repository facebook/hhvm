<?php
// Similar case, but for a builtin (array_multisort).
function blarg2(&$a1, &$a2) {}

// sscanf is a builtin that takes all additional args by ref.
function main3() {
  sscanf("foo 12 bar", "%s %d %s", &$a, &$b, &$c);
  echo "Foo: $a $b $c\n";
}

// array_multisort is weird.  Some arguments are literals.
function main4() {
  $x = array(1, 54, 3, 23, 5, 2);
  $y = array("a", "b", "c", "d", "e", "f");
  var_dump($x, $y);
  array_multisort(&$x, &$y);
  var_dump($x, $y);
  $desc = SORT_DESC;
  array_multisort(&$x, &$desc, &$y);
  var_dump($x, $y);
}

function main5() {
  $params = array(array(3,2,1),array(4,6,5),array(7,9,8));
  array_multisort(...$params);
  var_dump($params);
}



// Tests a case where we are passing more args than a function takes
// to its reffiness guard.
<<__EntryPoint>>
function main_ref_args() {
error_reporting(0);
main3();
main4();
main5();
}
