<?php
/* Prototype  : string date_format  ( DateTime $object  , string $format  )
 * Description: Returns date formatted according to given format
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTime::format
 */
 
//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing date_format() : basic functionality ***\n";
$date = date_create("2005-07-14 22:30:41");

var_dump( date_format($date, "F j, Y, g:i a") );                 
var_dump( date_format($date, "m.d.y") );                         
var_dump( date_format($date, "j, n, Y") );                       
var_dump( date_format($date, "Ymd") );                          
var_dump( date_format($date, 'h-i-s, j-m-y, it is w Day') );     
var_dump( date_format($date, '\i\t \i\s \t\h\e jS \d\a\y.') );   
var_dump( date_format($date, "D M j G:i:s T Y") );               
var_dump( date_format($date, 'H:m:s \m \i\s\ \m\o\n\t\h') );     
var_dump( date_format($date, "H:i:s") );                         

?>
===DONE===