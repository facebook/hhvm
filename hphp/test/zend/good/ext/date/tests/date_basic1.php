<?php
/* Prototype  : string date  ( string $format  [, int $timestamp  ] )
 * Description: Format a local time/date.
 * Source code: ext/date/php_date.c
 */

//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing date() : basic functionality ***\n";

$timestamp = mktime(10, 44, 30, 2, 27, 2009);

var_dump( date("F j, Y, g:i a", $timestamp) );     
var_dump( date("m.d.y", $timestamp) );                         
var_dump( date("j, n, Y", $timestamp) );             
var_dump( date("Ymd", $timestamp) );      
var_dump( date('h-i-s, j-m-y, it is w Day', $timestamp) );    
var_dump( date('\i\t \i\s \t\h\e jS \d\a\y.', $timestamp) );
var_dump( date("D M j G:i:s T Y", $timestamp) );
var_dump( date('H:m:s \m \i\s\ \m\o\n\t\h', $timestamp) );  
var_dump( date("H:i:s", $timestamp) );

?>
===DONE===