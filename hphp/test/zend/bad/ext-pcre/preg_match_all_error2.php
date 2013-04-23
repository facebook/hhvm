<?php
/*
* proto int preg_match_all(string pattern, string subject, array subpatterns [, int flags [, int offset]])
* Function is implemented in ext/pcre/php_pcre.c
*/
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_match_all reacts to being passed the wrong type of input argument
*/
echo "*** Testing preg_match_all() : error conditions ***\n";
$regex = '/[a-zA-Z]/';
$value = new stdclass(); //Object
var_dump(preg_match_all($regex, $value, $matches));
var_dump($matches);
$input = array(array('this is', 'a subarray'), 'test',);
foreach($input as $value) {
    print "\nArg value is: $value\n";
    var_dump(preg_match_all($regex, $value, $matches));
    var_dump($matches);
}
echo "Done";
?>