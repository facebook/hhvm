<?php
/* Prototype  : array getdate([int timestamp])
 * Description: Get date/time information 
 * Source code: ext/date/php_date.c
 */

echo "*** Testing getdate() : basic functionality ***\n";

//Set the default time zone 
date_default_timezone_set("Asia/Calcutta");

// Initialise all required variables
$timestamp = 10;

// Calling getdate() with all possible arguments
var_dump( getdate($timestamp) );

// Calling getdate() with mandatory arguments
var_dump( getdate() );

?>
===DONE===