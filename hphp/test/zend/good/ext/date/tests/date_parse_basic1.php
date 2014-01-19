<?php
/* Prototype  : array date_parse  ( string $date  ) 
 * Description: Returns associative array with detailed info about given date.
 * Source code: ext/date/php_date.c
 */
 
//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing date_parse() : basic functionality ***\n";

var_dump( date_parse("2009-02-27 10:00:00.5") );
var_dump( date_parse("10:00:00.5") );
var_dump( date_parse("2009-02-27") );

?>
===DONE===