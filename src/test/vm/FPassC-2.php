<?php
error_reporting(-1);

if (1) {
  function g(&$x) {
    var_dump($x);
    $x = 123;
  }
}

// doesn't fatal or warn

$x = null;
g(include "doesnotexist.php");
var_dump($x);

g(new stdclass);

$x = new stdclass;
g(clone $x);
var_dump($x);

echo "==============================\n";

// doesn't fatal, raises strict standards warning

$x = null;
g(eval('return $x;'));
var_dump($x);

$x = null;
g($x = 'abc');
var_dump($x);

$x = null;
g($x += 'abc');
var_dump($x);

$x = null;
g(++$x);
var_dump($x);

$x = null;
g(@$x);
var_dump($x);

echo "==============================\n";

// For list assignment expressions, if the RHS is a simple variable
// then it will be passed by reference via FPassH

$x = null;
$innerArr = array(1);
$arr = array(&$innerArr);
g(list($x) = $arr); // array(&array(1))
var_dump($x); // array(1)
var_dump($arr); // 123
var_dump($innerArr); // array(1)
unset($x);
unset($arr);
unset($innerArr);

echo "==============================\n";

// For binding assignment expressions, the RHS will be passed
// by reference via FPassV

$x = null;
g($y =& $x);
var_dump($x);
var_dump($y);

echo "==============================\n";

// fatals

$x = null;
g($x++);
var_dump($x);

