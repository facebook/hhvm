<?php
/* 
 *  proto int preg_match(string pattern, string subject [, array subpatterns [, int flags [, int offset]]])
 * Function is implemented in ext/pcre/php_pcre.c
*/
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_match reacts to being passed the wrong type of subject argument
*/
echo "*** Testing preg_match() : error conditions ***\n";
$regex = '/[a-zA-Z]/';
$input = array('this is a string', array('this is', 'a subarray'),);
foreach($input as $value) {
    print "\nArg value is: $value\n";
    var_dump(preg_match($regex, $value));
}
$value = new stdclass(); //Object
var_dump(preg_match($regex, $value));
echo "Done";
?>