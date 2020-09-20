<?hh
/* Prototype  : int timezone_offset_get  ( DateTimeZone $object  , DateTime $datetime  )
 * Description: Returns the timezone offset from GMT
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTimeZone::getOffset
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing timezone_offset_get() : basic functionality ***\n";

//Set the default time zone 
date_default_timezone_set("GMT");

$tz = timezone_open("Europe/London");
$date = date_create("GMT");

var_dump(timezone_offset_get($tz, $date));

$tz = timezone_open("America/New_York");
var_dump(timezone_offset_get($tz, $date));

$tz = timezone_open("America/Los_Angeles");
var_dump(timezone_offset_get($tz, $date));

echo "===DONE===\n";
}
