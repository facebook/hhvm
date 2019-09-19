<?hh
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c */
<<__EntryPoint>> function main(): void {
echo "*** Testing preg_replace() : error conditions ***\n";
//Zero arguments
echo "\n-- Testing preg_replace() function with zero arguments --\n";
try { var_dump(preg_replace()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
//Test preg_replace() with one more than the expected number of arguments
echo "\n-- Testing preg_replace() function with more than expected no. of arguments --\n";
$regex = '/\w/';
$replace = '1';
$subject = 'string_val';
$limit = 10;
$extra_arg = 10;
$count = -1;
try { var_dump(preg_replace_with_count($regex, $replace, $subject, $limit, inout $count, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
//Testing preg_replace() with one less than the expected number of arguments
echo "\n-- Testing preg_replace() function with less than expected no. of arguments --\n";
$regex = '/\w/';
$replace = '1';
try { var_dump(preg_replace($regex, $replace)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
