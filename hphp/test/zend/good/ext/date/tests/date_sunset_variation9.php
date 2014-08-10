<?php
/* Prototype  : mixed date_sunset(mixed time [, int format [, float latitude [, float longitude [, float zenith [, float gmt_offset]]]]])
 * Description: Returns time of sunset for a given day and location 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing date_sunset() : usage variation ***\n";

// GMT is zero for the timezone
date_default_timezone_set("Asia/Calcutta");
//Initialise the variables
$latitude = 38.4;
$longitude = -9;
$zenith = 90;
$gmt_offset = 1;

echo "\n-- Testing date_sunset() function by passing float 12.3456789000e10 value to time --\n";
$time = 12.3456789000e10;
var_dump( date_sunset($time, SUNFUNCS_RET_STRING, $latitude, $longitude, $zenith, $gmt_offset) );
var_dump( date_sunset($time, SUNFUNCS_RET_DOUBLE, $latitude, $longitude, $zenith, $gmt_offset) );
var_dump( date_sunset($time, SUNFUNCS_RET_TIMESTAMP, $latitude, $longitude, $zenith, $gmt_offset) );

echo "\n-- Testing date_sunset() function by passing float -12.3456789000e10 value to time --\n";
$time = -12.3456789000e10;
var_dump( date_sunset($time, SUNFUNCS_RET_STRING, $latitude, $longitude, $zenith, $gmt_offset) );
var_dump( date_sunset($time, SUNFUNCS_RET_DOUBLE, $latitude, $longitude, $zenith, $gmt_offset) );
var_dump( date_sunset($time, SUNFUNCS_RET_TIMESTAMP, $latitude, $longitude, $zenith, $gmt_offset) );

?>
===DONE===
