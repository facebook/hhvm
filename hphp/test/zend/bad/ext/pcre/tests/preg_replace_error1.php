<?php
/*
* proto string preg_replace(mixed regex, mixed replace, mixed subject [, int limit [, count]])
* Function is implemented in ext/pcre/php_pcre.c
*/
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_replace reacts to being passed the wrong type of regex argument
*/
echo "*** Testing preg_replace() : error conditions***\n";
$regex_array = array('abcdef', //Regex without delimiter
'/[a-zA-Z]', //Regex without closing delimiter
'[a-zA-Z]/', //Regex without opening delimiter
'/[a-zA-Z]/F', array('[a-z]', //Array of Regexes
'[A-Z]', '[0-9]'), '/[a-zA-Z]/', //Regex string
);
$replace = 1;
$subject = 'a';
foreach($regex_array as $regex_value) {
    print "\nArg value is $regex_value\n";
    var_dump(preg_replace($regex_value, $replace, $subject));
}
$regex_value = new stdclass(); //Object
var_dump(preg_replace($regex_value, $replace, $subject));
?>