<?hh
/* Prototype  : string gmdate(string format [, long timestamp])
 * Description: Format a GMT date/time 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gmdate() : error conditions ***\n";

// Initialise all required variables
date_default_timezone_set('UTC');
$format = DATE_ISO8601;
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

// Zero arguments
echo "\n-- Testing gmdate() function with Zero arguments --\n";
try { var_dump( gmdate() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test gmdate with one more than the expected number of arguments
echo "\n-- Testing gmdate() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( gmdate($format, $timestamp, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
