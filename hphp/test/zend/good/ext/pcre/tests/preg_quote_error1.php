<?php
/*
* proto string preg_quote(string str [, string delim_char])
* Function is implemented in ext/pcre/php_pcre.c
*/
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_quote reacts to being passed the wrong type of input argument
*/
echo "*** Testing preg_quote() : error conditions ***\n";
$input = array('this is a string', array('this is', 'a subarray'),);
foreach($input as $value) {
    print "\nArg value is: $value\n";
    var_dump(preg_quote($value));
}
$value = new stdclass(); //Object
var_dump(preg_quote($value));
echo "Done";
?>