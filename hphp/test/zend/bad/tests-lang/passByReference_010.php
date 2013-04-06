<?php

function f(&$a) {
  var_dump($a);
  $a = "a.changed";
} 

echo "\n\n---> Pass constant assignment by reference:\n";
f($a="a.original");
var_dump($a); 

echo "\n\n---> Pass variable assignment by reference:\n";
unset($a);
$a = "a.original";
f($b = $a);
var_dump($a); 

echo "\n\n---> Pass reference assignment by reference:\n";
unset($a, $b);
$a = "a.original";
f($b =& $a);
var_dump($a); 

echo "\n\n---> Pass concat assignment by reference:\n";
unset($a, $b);
$b = "b.original";
$a = "a.original";
f($b .= $a);
var_dump($a); 

?>