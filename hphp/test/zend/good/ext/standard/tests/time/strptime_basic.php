<?php
/* Prototype  : array strptime  ( string $date  , string $format  )
 * Description:  Parse a time/date generated with strftime()
 * Source code: ext/standard/datetime.c
*/

$orig = setlocale(LC_ALL, 'C');
date_default_timezone_set("GMT"); 

echo "*** Testing strptime() : basic functionality ***\n";

$input = "10:00:00 AM July 2 1963";
$tstamp = strtotime($input);
 
$str = strftime("%r %B%e %Y %Z", $tstamp);
var_dump(strptime($str, '%H:%M:%S %p %B %d %Y'));

$str = strftime("%T %D", $tstamp);
var_dump(strptime($str, '%H:%M:%S %m/%d/%y'));

$str = strftime("%A %B %e %R", $tstamp);
var_dump(strptime($str, '%A %B %e %R'));

setlocale(LC_ALL, $orig);
?>
===DONE===