<?hh
/* Prototype  : int strncasecmp ( string $str1, string $str2, int $len );
 * Description: Binary safe case-insensitive string comparison of the first n characters
 * Source code: Zend/zend_builtin_functions.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strncasecmp() function: error conditions ***\n";
$str1 = 'string_val';
$str2 = 'string_val';
$len = 10;
$extra_arg = 10;

echo "\n-- Testing strncasecmp() function with Zero arguments --";
try { var_dump( strncasecmp() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strncasecmp() function with less than expected number of arguments --";
try { var_dump( strncasecmp($str1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump( strncasecmp($str1, $str2) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strncasecmp() function with more than expected number of arguments --";
try { var_dump( strncasecmp($str1, $str2, $len, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strncasecmp() function with invalid argument --";
$len = -10;
var_dump( strncasecmp($str1, $str2, $len) );
echo "*** Done ***\n";
}
