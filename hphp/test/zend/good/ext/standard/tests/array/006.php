<?php

array_pop($GLOBALS);

$a = array("foo", "bar", "fubar");
$b = array("3" => "foo", "4" => "bar", "5" => "fubar");
$c = array("a" => "foo", "b" => "bar", "c" => "fubar");

/* simple array */
echo array_pop($a), "\n";
array_push($a, "foobar");
var_dump($a);

/* numerical assoc indices */
echo array_pop($b), "\n";
var_dump($b);

/* assoc indices */
echo array_pop($c), "\n";
var_dump($c);

?>