<?php
/* Prototype  : bool isset  ( mixed $var  [, mixed $var  [,  $...  ]] )
 * Description:  Determine if a variable is set and is not NULL
 */		

class foo {}

echo "*** Testing isset() : basic functionality ***\n";

$i = 10;
$f = 10.5;
$s = "Hello";
$a = array(1,2,3,4,5);
$b = true;
$n = NULL;
$obj = new foo;
$res = fopen(__FILE__, "r");

echo "Integer test: " . (isset($i) ? "YES": "NO")  . "\n";
echo "Float test: " . (isset($f) ? "YES": "NO") . "\n";
echo "String test: " . (isset($s) ? "YES": "NO") . "\n";
echo "Array test: " . (isset($a) ? "YES": "NO") . "\n";
echo "Boolean test: " . (isset($b) ? "YES": "NO") . "\n";
echo "Null test: " . (isset($n) ? "YES": "NO") . "\n";
echo "Object test: " . (isset($obj) ? "YES": "NO") . "\n";
echo "Resource test: " . (isset($res) ? "YES": "NO") . "\n";

echo "\n\nUnset the variables\n\n";
unset($i, $f, $s, $a, $b, $n, $obj, $res);

echo "Integer test: " . (isset($i) ? "YES": "NO")  . "\n";
echo "Float test: " . (isset($f) ? "YES": "NO") . "\n";
echo "String test: " . (isset($s) ? "YES": "NO") . "\n";
echo "Array test: " . (isset($a) ? "YES": "NO") . "\n";
echo "Boolean test: " . (isset($b) ? "YES": "NO") . "\n";
echo "Null test: " . (isset($n) ? "YES": "NO") . "\n";
echo "Object test: " . (isset($obj) ? "YES": "NO") . "\n";
echo "Resource test: " . (isset($res) ? "YES": "NO") . "\n";
?>
===DONE===