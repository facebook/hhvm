<?hh
/* Prototype  : bool checkdate  ( int $month  , int $day  , int $year  )
 * Description: Validate a Gregorian date
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing checkdate() : error conditions ***\n";

//Set the default time zone 
date_default_timezone_set("America/Chicago");

$arg_0 = 1;
$arg_1 = 1;
$arg_2 = 1;
$extra_arg = 1;

echo "\n-- Testing checkdate() function with more than expected no. of arguments --\n";
try { var_dump (checkdate($arg_0, $arg_1, $arg_2, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing checkdate() function with less than expected no. of arguments --\n";
try { var_dump (checkdate()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump (checkdate($arg_0)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump (checkdate($arg_0, $arg_1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE=== ";
}
