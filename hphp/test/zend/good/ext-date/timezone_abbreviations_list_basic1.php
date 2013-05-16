<?php
/* Prototype  : array timezone_abbreviations_list  ( void  )
 * Description: Returns associative array containing dst, offset and the timezone name
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTimeZone::listAbbreviations
 */

echo "*** Testing timezone_abbreviations_list() : basic functionality ***\n";

//Set the default time zone 
date_default_timezone_set("GMT");

$abbr = timezone_abbreviations_list();

var_dump( gettype($abbr) );
var_dump( count($abbr) );

echo "\n-- Format a sample entry --\n";
var_dump( $abbr["acst"] );	

?>
===DONE===