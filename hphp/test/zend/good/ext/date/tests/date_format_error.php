<?hh
/* Prototype  : string date_format  ( DateTime $object  , string $format  )
 * Description: Returns date formatted according to given format
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTime::format
 */

//Set the default time zone
<<__EntryPoint>> function main(): void {
date_default_timezone_set("Europe/London");
echo "*** Testing date_format() : error conditions ***\n";

echo "\n-- Testing date_create() function with zero arguments --\n";
try { var_dump( date_format() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$date = date_create("2005-07-14 22:30:41");

echo "\n-- Testing date_create() function with less than expected no. of arguments --\n";
try { var_dump( date_format($date) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing date_create() function with more than expected no. of arguments --\n";
$format = "F j, Y, g:i a";
$extra_arg = 10;
try { var_dump( date_format($date, $format, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing date_create() function with an invalid values for \$object argument --\n";
$invalid_obj = new stdClass();
var_dump( date_format($invalid_obj, $format) );
$invalid_obj = 10;
try { var_dump( date_format($invalid_obj, $format) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$invalid_obj = null;
try { var_dump( date_format($invalid_obj, $format) );     } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
