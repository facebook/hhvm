<?php
/* Prototype  : mixed next(array $array_arg)
 * Description: Move array argument's internal pointer to the next element and return it 
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of next()
 */

echo "*** Testing next() : basic functionality ***\n";

$array = array('zero', 'one', 'two');
echo key($array) . " => " . current($array) . "\n";
var_dump(next($array));

echo key($array) . " => " . current($array) . "\n";
var_dump(next($array));

echo key($array) . " => " . current($array) . "\n";
var_dump(next($array));
?>
===DONE===