<?php

class X implements arrayaccess {
function offsetGet($n) {
 return $n;
 }
function offsetSet($n, $v) {
 var_dump($n);
 }
function offsetExists($n) {
 var_dump($n);
 return true;
 }
function offsetUnset($n) {
 var_dump($n);
 }
function __toString() {
 return 'baz';
 }
}
$a = new X;
echo "sets\n";
$a[true] = 5;
$a[NULL] = 57;
$a[3.2] = 25;
$a[array(3)] = 30;
$a[$a] = 32;
$a['57'] = 5;
$a['6.5'] = 7;
echo "gets\n";
var_dump($a[true]);
var_dump($a[NULL]);
var_dump($a[3.2]);
var_dump($a[array(3)]);
var_dump($a[$a]);
var_dump($a['57']);
var_dump($a['6.5']);
echo "unsets\n";
unset($a[true]);
unset($a[NULL]);
unset($a[3.2]);
unset($a[array(3)]);
unset($a[$a]);
unset($a['57']);
unset($a['6.5']);
echo "issets\n";
isset($a[true]);
isset($a[NULL]);
isset($a[3.2]);
isset($a[array(3)]);
isset($a[$a]);
isset($a['57']);
isset($a['6.5']);
