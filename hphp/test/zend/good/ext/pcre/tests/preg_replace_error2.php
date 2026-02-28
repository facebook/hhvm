<?hh
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c */
<<__EntryPoint>> function main(): void {
/*
* Testing how preg_replace reacts to being passed the wrong type of replacement argument
*/
echo "*** Testing preg_replace() : error conditions ***\n";
$regex = '/[a-zA-Z]/';
$replace = vec['this is a string', vec['this is', 'a subarray'],];
$subject = 'test';
foreach($replace as $value) {
    $text = HH\is_any_array($value) ? 'Array' : $value;
    print "\nArg value is: $text\n";
    var_dump(preg_replace($regex, $value, $subject));
}
$value = new stdClass(); //Object
var_dump(preg_replace($regex, $value, $subject));
echo "Done";
}
