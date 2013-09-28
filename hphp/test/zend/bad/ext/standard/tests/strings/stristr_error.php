<?php

/* Prototype: string stristr  ( string $haystack  , mixed $needle  [, bool $before_needle  ] )
   Description: Case-insensitive strstr()
*/
echo "*** Testing stristr() : error conditions ***\n";

echo "\n-- Testing stristr() function with no arguments --\n";
var_dump( stristr() );
var_dump( stristr("") );

echo "\n-- Testing stristr() function with no needle --\n";
var_dump( stristr("Hello World") );  // without "needle"

echo "\n-- Testing stristr() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( stristr("Hello World",  "World", true, $extra_arg) );

echo "\n-- Testing stristr() function with empty haystack --\n";
var_dump( stristr(NULL, "") );

echo "\n-- Testing stristr() function with empty needle --\n";
var_dump( stristr("Hello World", "") );

?>
===DONE===