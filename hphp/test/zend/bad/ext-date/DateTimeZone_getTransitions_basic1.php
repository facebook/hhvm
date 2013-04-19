<?php
/* Prototype  : array DateTimeZone::getTransitions  ()
 * Description: Returns all transitions for the timezone
 * Source code: ext/date/php_date.c
 * Alias to functions: timezone_transitions_get()
 */

echo "*** Testing DateTimeZone::getTransitions() : basic functionality ***\n";

//Set the default time zone 
date_default_timezone_set("Europe/London");

// Create a DateTimeZone object
$tz = new DateTimeZone("Europe/London");

$tran = $tz->getTransitions();

if (!is_array($tran)) {
	echo "TEST FAILED: Expected an array\n";
}

echo "\n-- Total number of transitions: " . count($tran). " --\n"; 

echo "\n-- Format a sample entry for Spring 1963 --\n";
var_dump( $tran[97] );	

?>
===DONE===