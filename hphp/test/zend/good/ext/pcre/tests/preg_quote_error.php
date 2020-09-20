<?hh
/*
* proto string preg_quote(string str [, string delim_char])
* Function is implemented in ext/pcre/php_pcre.c */
<<__EntryPoint>> function main(): void {
echo "*** Testing preg_quote() : error conditions ***\n";
// Zero arguments
echo "\n-- Testing preg_quote() function with Zero arguments --\n";
try { var_dump(preg_quote()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
//Test preg_quote with one more than the expected number of arguments
echo "\n-- Testing preg_quote() function with more than expected no. of arguments --\n";
$str = 'string_val';
$delim_char = '/';
$extra_arg = 10;
try { var_dump(preg_quote($str, $delim_char, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
