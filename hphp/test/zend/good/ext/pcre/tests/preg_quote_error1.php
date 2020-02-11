<?hh
/*
* proto string preg_quote(string str [, string delim_char])
* Function is implemented in ext/pcre/php_pcre.c */
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_quote reacts to being passed the wrong type of input argument
*/
echo "*** Testing preg_quote() : error conditions ***\n";
$input = varray['this is a string', varray['this is', 'a subarray'],];
foreach($input as $value) {
    print "\nArg value is: $value\n";
    try { var_dump(preg_quote($value)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
$value = new stdclass(); //Object
try { var_dump(preg_quote($value)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
