<?php
/* Prototype  : DateTime date_modify  ( DateTime $object  , string $modify  )
 * Description: Alter the timestamp of a DateTime object by incrementing or decrementing in a format accepted by strtotime(). 
 * Source code: ext/date/php_date.c
 * Alias to functions: public DateTime DateTime::modify()
 */

//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing date_modify() : error conditions ***\n";

echo "\n-- Testing date_modify() function with zero arguments --\n";
var_dump( date_modify() );

// Create a date object
$datetime = date_create("2009-01-30 19:34:10");

echo "\n-- Testing date_modify() function with less than expected no. of arguments --\n";
var_dump( date_modify($datetime) );

echo "\n-- Testing date_modify() function with more than expected no. of arguments --\n";
$modify = "+1 day";
$extra_arg = 99;
var_dump( date_modify($datetime, $modify, $extra_arg) );

echo "\n-- Testing date_modify() function with an invalid values for \$object argument --\n";
$invalid_obj = new stdClass();
var_dump( date_modify($invalid_obj, $modify) );  
$invalid_obj = 10;
var_dump( date_modify($invalid_obj, $modify) );
$invalid_obj = null;
var_dump( date_modify($invalid_obj, $modify) );  

?>
===DONE===