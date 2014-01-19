<?php
/* Prototype  : array gettimeofday([bool get_as_float])
 * Description: Returns the current time as array 
 * Source code: ext/standard/microtime.c
 * Alias to functions: 
 */

echo "*** Testing gettimeofday() : basic functionality ***\n";

date_default_timezone_set("Asia/Calcutta");

// Initialise all required variables
$get_as_float = true;

// Calling gettimeofday() with all possible arguments
var_dump( gettimeofday($get_as_float) );

// Calling gettimeofday() with mandatory arguments
var_dump( gettimeofday() );

// Initialise all required variables
$get_as_float = false;

// Calling gettimeofday() with all possible arguments
var_dump( gettimeofday($get_as_float) );

?>
===DONE===