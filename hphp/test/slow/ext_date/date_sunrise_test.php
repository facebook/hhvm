<?hh
/* Prototype  : mixed date_sunrise(mixed time [, int format [, float latitude
 * [, float longitude [, float zenith [, float gmt_offset]]]]])
 * Description: Returns time of sunrise for a given day and location
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing date_sunrise() : usage variation ***\n";

// GMT is zero for the timezone
date_default_timezone_set("Asia/Calcutta");

//Initialise the variables
$time = mktime(0, 0, 0, 12, 26, 2012);
$latitude = 38.4;
$longitude = -9.0;
$zenith = 90.0;
$gmt_offset = 1.0;

echo "\n-- Testing date_sunrise() function by passing one parameter --\n";
var_dump( date_sunrise($time) );

echo "\n-- Testing date_sunrise() function by passing two parameters --\n";
var_dump( date_sunrise($time, SUNFUNCS_RET_STRING) );
var_dump( round( date_sunrise($time, SUNFUNCS_RET_DOUBLE), 4) );
var_dump( date_sunrise($time, SUNFUNCS_RET_TIMESTAMP) );

echo "\n-- Testing date_sunrise() function by passing two parameters --\n";
var_dump( date_sunrise($time, SUNFUNCS_RET_STRING, $latitude) );
var_dump( round( date_sunrise($time, SUNFUNCS_RET_DOUBLE, $latitude), 4) );
var_dump( date_sunrise($time, SUNFUNCS_RET_TIMESTAMP, $latitude) );

echo "\n-- Testing date_sunrise() function by passing three  parameters --\n";
var_dump( date_sunrise($time, SUNFUNCS_RET_STRING, $latitude, $longitude) );
var_dump( round( date_sunrise($time, SUNFUNCS_RET_DOUBLE, $latitude, $longitude), 4) );
var_dump( date_sunrise($time, SUNFUNCS_RET_TIMESTAMP, $latitude, $longitude) );

echo "\n-- Testing date_sunrise() function by passing four parameters --\n";
var_dump( date_sunrise($time, SUNFUNCS_RET_STRING,
  $latitude, $longitude, $zenith) );
var_dump( round( date_sunrise($time, SUNFUNCS_RET_DOUBLE,
  $latitude, $longitude, $zenith), 4) );
var_dump( date_sunrise($time, SUNFUNCS_RET_TIMESTAMP,
  $latitude, $longitude, $zenith) );

echo "\n-- Testing date_sunrise() function by passing five parameters --\n";
var_dump( date_sunrise($time, SUNFUNCS_RET_STRING,
  $latitude, $longitude, $zenith, $gmt_offset) );
var_dump( round( date_sunrise($time, SUNFUNCS_RET_DOUBLE,
  $latitude, $longitude, $zenith, $gmt_offset), 4) );
var_dump( date_sunrise($time, SUNFUNCS_RET_TIMESTAMP,
  $latitude, $longitude, $zenith, $gmt_offset) );

echo "===DONE===\n";
}
