<?hh
/* Prototype  : string date  ( string $format  [, int $timestamp  ] )
 * Description: Format a local time/date.
 * Source code: ext/date/php_date.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing date() : error conditions ***\n";

//Set the default time zone
date_default_timezone_set("America/Chicago");

$format = "m.d.y";
$timestamp = mktime(10, 44, 30, 2, 27, 2009);

echo "\n-- Testing date function with no arguments --\n";
try { var_dump (date()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE=== ";
}
