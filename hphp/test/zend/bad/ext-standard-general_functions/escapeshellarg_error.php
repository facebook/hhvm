<?php

/* Prototype  : string escapeshellarg  ( string $arg  )
 * Description:  Escape a string to be used as a shell argument.
 * Source code: ext/standard/exec.c
 */

/*
 * Pass an incorrect number of arguments to escapeshellarg() to test behaviour
 */

echo "*** Testing escapeshellarg() : error conditions ***\n";


echo "\n-- Testing escapeshellarg() function with no arguments --\n";
var_dump( escapeshellarg() );

echo "\n-- Testing escapeshellarg() function with more than expected no. of arguments --\n";
$arg = "Mr O'Neil";
$extra_arg = 10;
var_dump( escapeshellarg($arg, $extra_arg) );

echo "\n-- Testing escapeshellarg() function with a object supplied for argument --\n";

class classA
{
}

$arg = new classA();
var_dump( escapeshellarg($arg));

echo "\n-- Testing escapeshellarg() function with a resource supplied for argument --\n";
$fp = fopen(__FILE__, "r");
var_dump( escapeshellarg($fp));
fclose($fp);

echo "\n-- Testing escapeshellarg() function with a array supplied for argument --\n";
$arg = array(1,2,3); 
var_dump( escapeshellarg($arg));

?>
===Done===