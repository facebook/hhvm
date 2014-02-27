<?php
/* Prototype  : mixed date_sunset(mixed time [, int format [, float latitude
 * [, float longitude [, float zenith [, float gmt_offset]]]]])
 * Description: Returns time of sunrise for a given day and location
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */

echo "*** Testing date_sunset() : usage variation ***\n";

// GMT is zero for the timezone
date_default_timezone_set("Asia/Calcutta");

//Initialise the variables
$time = mktime(0, 0, 0, 1, 26, 2012);
$latitude = 38.4;
$longitude = -9;
$zenith = 90;
$gmt_offset = 1;

echo "\n-- Testing date_sunset() function by passing one parameter --\n";
var_dump( date_sunset($time) );

echo "\n-- Testing date_sunset() function by passing two parameters --\n";
var_dump( date_sunset($time, SUNFUNCS_RET_STRING) );
var_dump( date_sunset($time, SUNFUNCS_RET_DOUBLE) );
var_dump( date_sunset($time, SUNFUNCS_RET_TIMESTAMP) );

echo "\n-- Testing date_sunset() function by passing two parameters --\n";
var_dump( date_sunset($time, SUNFUNCS_RET_STRING, $latitude) );
var_dump( date_sunset($time, SUNFUNCS_RET_DOUBLE, $latitude) );
var_dump( date_sunset($time, SUNFUNCS_RET_TIMESTAMP, $latitude) );

echo "\n-- Testing date_sunset() function by passing three  parameters --\n";
var_dump( date_sunset($time, SUNFUNCS_RET_STRING, $latitude, $longitude) );
var_dump( date_sunset($time, SUNFUNCS_RET_DOUBLE, $latitude, $longitude) );
var_dump( date_sunset($time, SUNFUNCS_RET_TIMESTAMP, $latitude, $longitude) );

echo "\n-- Testing date_sunset() function by passing four parameters --\n";
var_dump( date_sunset($time, SUNFUNCS_RET_STRING,
  $latitude, $longitude, $zenith) );
var_dump( date_sunset($time, SUNFUNCS_RET_DOUBLE,
  $latitude, $longitude, $zenith) );
var_dump( date_sunset($time, SUNFUNCS_RET_TIMESTAMP,
  $latitude, $longitude, $zenith) );

echo "\n-- Testing date_sunset() function by passing five parameters --\n";
var_dump( date_sunset($time, SUNFUNCS_RET_STRING,
  $latitude, $longitude, $zenith, $gmt_offset) );
var_dump( date_sunset($time, SUNFUNCS_RET_DOUBLE,
  $latitude, $longitude, $zenith, $gmt_offset) );
var_dump( date_sunset($time, SUNFUNCS_RET_TIMESTAMP,
  $latitude, $longitude, $zenith, $gmt_offset) );

?>
===DONE===
