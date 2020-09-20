<?hh
/* Prototype  : string strftime(string format [, int timestamp])
 * Description: Format a local time/date according to locale settings 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing strftime() : error conditions ***\n";

date_default_timezone_set("Asia/Calcutta");
//Test strftime with one more than the expected number of arguments
echo "\n-- Testing strftime() function with more than expected no. of arguments --\n";
$format = '%b %d %Y %H:%M:%S';
$timestamp = mktime(8, 8, 8, 8, 8, 2008);
$extra_arg = 10;
try { var_dump( strftime($format, $timestamp, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
