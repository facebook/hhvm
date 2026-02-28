<?hh
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c */
<<__EntryPoint>> function main(): void {
/*
* Testing how preg_replace reacts to being passed the wrong type of regex argument
*/
echo "*** Testing preg_replace() : error conditions***\n";
$regex_array = vec['abcdef', //Regex without delimiter
'/[a-zA-Z]', //Regex without closing delimiter
'[a-zA-Z]/', //Regex without opening delimiter
'/[a-zA-Z]/F', vec['[a-z]', //Array of Regexes
'[A-Z]', '[0-9]'], '/[a-zA-Z]/', //Regex string
];
$replace = 1;
$subject = 'a';
foreach($regex_array as $regex_value) {
    $text = HH\is_any_array($regex_value) ? 'Array' : $regex_value;
    print "\nArg value is $text\n";
    var_dump(preg_replace($regex_value, $replace, $subject));
}
$regex_value = new stdClass(); //Object
var_dump(preg_replace($regex_value, $replace, $subject));
}
