<?php
/*
* proto array preg_split(string pattern, string subject [, int limit [, int flags]])
* Function is implemented in ext/pcre/php_pcre.c
*/
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_split reacts to being passed the wrong type of input argument
*/
echo "*** Testing preg_split() : error conditions ***\n";
$regex = '/[a-zA-Z]/';
$input = array(array('this is', 'a subarray'),);
foreach($input as $value) {
    print "\nArg value is: $value\n";
    var_dump(preg_split($regex, $value));
}
$value = new stdclass(); //Object
var_dump(preg_split($regex, $value));
echo "Done";
?>