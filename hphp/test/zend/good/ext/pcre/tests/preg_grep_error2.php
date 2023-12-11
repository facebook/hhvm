<?hh
/*
* proto array preg_grep(string regex, array input [, int flags])
* Function is implemented in ext/pcre/php_pcre.c */
<<__EntryPoint>> function main(): void {
/*
* Testing how preg_grep reacts to being passed the wrong type of input argument
*/
echo "*** Testing preg_grep() : error conditions ***\n";
$regex = '/[a-zA-Z]/';
$input = vec['this is a string', vec['this is', 'a subarray'],];
foreach($input as $value) {
    $text = HH\is_any_array($value) ? 'Array' : $value;
    print "\nArg value is: $text\n";
    try { var_dump(preg_grep($regex, $value)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
$value = new stdClass(); //Object
try { var_dump(preg_grep($regex, $value)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
