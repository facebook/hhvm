<?hh
/*
* proto array preg_grep(string regex, array input [, int flags])
* Function is implemented in ext/pcre/php_pcre.c */
<<__EntryPoint>> function main(): void {
echo "*** Testing preg_grep() : error conditions ***\n";
// Zero arguments
echo "\n-- Testing preg_grep() function with Zero arguments --\n";
try { var_dump(preg_grep()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
//Test preg_grep with one more than the expected number of arguments
echo "\n-- Testing preg_grep() function with more than expected no. of arguments --\n";
$regex = '/\d/';
$input = vec[1, 2];
$flags = 0;
$extra_arg = 10;
try { var_dump(preg_grep($regex, $input, $flags, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
// Testing preg_grep withone less than the expected number of arguments
echo "\n-- Testing preg_grep() function with less than expected no. of arguments --\n";
$regex = 'string_val';
try { var_dump(preg_grep($regex)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
