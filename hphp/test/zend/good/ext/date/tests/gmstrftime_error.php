<?hh
/* Prototype  : string gmstrftime(string format [, int timestamp])
 * Description: Format a GMT/UCT time/date according to locale settings 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gmstrftime() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing gmstrftime() function with Zero arguments --\n";
try { var_dump( gmstrftime() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test gmstrftime with one more than the expected number of arguments
echo "\n-- Testing gmstrftime() function with more than expected no. of arguments --\n";
$format = '%b %d %Y %H:%M:%S';
$timestamp = gmmktime(8, 8, 8, 8, 8, 2008);
$extra_arg = 10;
try { var_dump( gmstrftime($format, $timestamp, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
