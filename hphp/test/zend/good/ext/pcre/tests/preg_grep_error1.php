<?hh
/*
* proto array preg_grep(string regex, array input [, int flags])
* Function is implemented in ext/pcre/php_pcre.c */
<<__EntryPoint>> function main(): void {
/*
* Testing how preg_grep reacts to being passed bad regexes
*/
echo "*** Testing preg_grep() : error conditions ***\n";
$values = vec['abcdef', //Regex without delimiter
'/[a-zA-Z]', //Regex without closing delimiter
'[a-zA-Z]/', //Regex without opening delimiter
'/[a-zA-Z]/F', vec['[a-z]', //Array of Regexes
'[A-Z]', '[0-9]'], '/[a-zA-Z]/', //Regex string
];
$array = vec[123, 'abc', 'test'];
foreach($values as $value) {
    $text = HH\is_any_array($value) ? 'Array' : $value;
    print "\nArg value is $text\n";
    try { var_dump(preg_grep($value, $array)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
$value = new stdClass(); //Object
try { var_dump(preg_grep($value, $array)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
