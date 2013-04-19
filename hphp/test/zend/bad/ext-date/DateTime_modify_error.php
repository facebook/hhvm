<?php
/* Prototype  : public DateTime DateTime::modify  ( string $modify  )
 * Description: Alter the timestamp of a DateTime object by incrementing or decrementing in a format accepted by strtotime(). 
 * Source code: ext/date/php_date.c
 * Alias to functions: public date_modify()
 */

//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing DateTime::modify() : error conditions ***\n";

// Create a date object
$object = new DateTime("2009-01-30 19:34:10");

echo "\n-- Testing DateTime::modify() function with less than expected no. of arguments --\n";
var_dump( $object->modify() );

echo "\n-- Testing DateTime::modify() function with more than expected no. of arguments --\n";
$modify = "+1 day";
$extra_arg = 99;
var_dump( $object->modify($modify, $extra_arg) );

?>
===DONE===