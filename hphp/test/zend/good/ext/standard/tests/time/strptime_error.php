<?hh
/* Prototype  : array strptime  ( string $date  , string $format  )
 * Description: Parse a time/date generated with strftime()
 * Source code: ext/standard/datetime.c
 * Alias to functions:
 */

//Set the default time zone
<<__EntryPoint>> function main(): void {
date_default_timezone_set("Europe/London");
echo "*** Testing strptime() : error conditions ***\n";

echo "\n-- Testing strptime() function with Zero arguments --\n";
try { var_dump( strptime() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strptime() function with less than expected no. of arguments --\n";
$format = '%b %d %Y %H:%M:%S';
$timestamp = mktime(8, 8, 8, 8, 8, 2008);
$date = strftime($format, $timestamp);
try { var_dump( strptime($date) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strptime() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( strptime($date, $format, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
