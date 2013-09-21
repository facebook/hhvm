<?php
/*
* proto array preg_grep(string regex, array input [, int flags])
* Function is implemented in ext/pcre/php_pcre.c
*/
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_grep reacts to being passed the wrong type of input argument
*/
echo "*** Testing preg_grep() : error conditions ***\n";
$regex = '/[a-zA-Z]/';
$input = array('this is a string', array('this is', 'a subarray'),);
foreach($input as $value) {
    print "\nArg value is: $value\n";
    var_dump(preg_grep($regex, $value));
}
$value = new stdclass(); //Object
var_dump(preg_grep($regex, $value));
echo "Done";
?>