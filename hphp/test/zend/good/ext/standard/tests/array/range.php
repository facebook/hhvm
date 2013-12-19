<?php

echo "*** Testing range() function on basic operations ***\n";

echo "\n-- Integers as Low and High --\n";
echo "-- An array of elements from low to high --\n";
var_dump( range(1, 10) );
echo "\n-- An array of elements from high to low --\n";
var_dump( range(10, 1) );

echo "\n-- Numeric Strings as Low and High --\n";	
echo "-- An array of elements from low to high --\n";
var_dump( range("1", "10") );
echo "\n-- An array of elements from high to low --\n";
var_dump( range("10", "1") );

echo "\n-- Chars as Low and High --\n";	
echo "-- An array of elements from low to high --\n";
var_dump( range("a", "z") );
echo "\n-- An array of elements from high to low --\n";
var_dump( range("z", "a") );

echo "\n-- Low and High are equal --\n";
var_dump( range(5, 5) );
var_dump( range("q", "q") );

echo "\n-- floats as Low and High --\n";	
var_dump( range(5.1, 10.1) );
var_dump( range(10.1, 5.1) );

var_dump( range("5.1", "10.1") );
var_dump( range("10.1", "5.1") );

echo "\n-- Passing step with Low and High --\n";	
var_dump( range(1, 2, 0.1) );
var_dump( range(2, 1, 0.1) );

var_dump( range(1, 2, "0.1") );
var_dump( range("1", "2", 0.1) ); 

echo "\n-- Testing basic string with step --\n";
var_dump( range("abcd", "mnop", 2) );

echo "Done\n";
?>