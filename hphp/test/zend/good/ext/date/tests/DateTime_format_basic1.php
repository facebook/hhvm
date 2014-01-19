<?php
/* Prototype  : public string DateTime::format  ( string $format  )
 * Description: Returns date formatted according to given format
 * Source code: ext/date/php_date.c
 * Alias to functions: date_format
 */
 
//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing DateTime::format() : basic functionality ***\n";
$date = new DateTime("2005-07-14 22:30:41");

var_dump( $date->format( "F j, Y, g:i a") );                 
var_dump( $date->format( "m.d.y") );                         
var_dump( $date->format( "j, n, Y") );                       
var_dump( $date->format( "Ymd") );                          
var_dump( $date->format( 'h-i-s, j-m-y, it is w Day') );     
var_dump( $date->format( '\i\t \i\s \t\h\e jS \d\a\y.') );   
var_dump( $date->format( "D M j G:i:s T Y") );               
var_dump( $date->format( 'H:m:s \m \i\s\ \m\o\n\t\h') );     
var_dump( $date->format( "H:i:s") );                         

?>
===DONE===