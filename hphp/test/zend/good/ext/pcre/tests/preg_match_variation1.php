<?hh
/*
 *  proto int preg_match(string pattern, string subject [, array subpatterns [, int flags [, int offset]]])
 * Function is implemented in ext/pcre/php_pcre.c
*/
<<__EntryPoint>> function main(): void {
//test passing in the same variable where 1 is by value, the other is a different
//type and by reference so should be updated to the new type.
$string = "-1";
preg_match_with_matches('/[\-\+]?[0-9\.]*/', $string, inout $string);
var_dump($string);
echo "===Done===";
}
