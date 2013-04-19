<?php
/* Prototype  : array DateTimeZone::listAbbreviations  ( void  )
 * Description: Returns associative array containing dst, offset and the timezone name
 * Source code: ext/date/php_date.c
 * Alias to functions: timezone_abbreviations_list
 */

echo "*** Testing DateTimeZone::listAbbreviations() : basic functionality ***\n";

//Set the default time zone 
date_default_timezone_set("GMT");

$abbr = DateTimeZone::listAbbreviations();

var_dump( gettype($abbr) );
var_dump( count($abbr) );

echo "\n-- Format a sample entry --\n";
var_dump( $abbr["acst"] );	

?>
===DONE===