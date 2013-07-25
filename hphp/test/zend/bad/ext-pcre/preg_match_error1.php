<?php
/* 
 *  proto int preg_match(string pattern, string subject [, array subpatterns [, int flags [, int offset]]])
 * Function is implemented in ext/pcre/php_pcre.c
*/
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_match reacts to being passed the wrong type of regex argument
*/
echo "*** Testing preg_match() : error conditions ***\n";
$regex_array = array('abcdef', //Regex without delimiter
'/[a-zA-Z]', //Regex without closing delimiter
'[a-zA-Z]/', //Regex without opening delimiter
'/[a-zA-Z]/F', array('[a-z]', //Array of Regexes
'[A-Z]', '[0-9]'), '/[a-zA-Z]/', //Regex string
);
$subject = 'this is a test';
foreach($regex_array as $regex_value) {
    print "\nArg value is $regex_value\n";
    var_dump(preg_match($regex_value, $subject));
}
$regex_value = new stdclass(); //Object
var_dump(preg_match($regex_value, $subject));
?>