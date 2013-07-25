<?php
/*
* proto array preg_grep(string regex, array input [, int flags])
* Function is implemented in ext/pcre/php_pcre.c
*/
error_reporting(E_ALL&~E_NOTICE);
/*
* Testing how preg_grep reacts to being passed bad regexes
*/
echo "*** Testing preg_grep() : error conditions ***\n";
$values = array('abcdef', //Regex without delimiter
'/[a-zA-Z]', //Regex without closing delimiter
'[a-zA-Z]/', //Regex without opening delimiter
'/[a-zA-Z]/F', array('[a-z]', //Array of Regexes
'[A-Z]', '[0-9]'), '/[a-zA-Z]/', //Regex string
);
$array = array(123, 'abc', 'test');
foreach($values as $value) {
    print "\nArg value is $value\n";
    var_dump(preg_grep($value, $array));
}
$value = new stdclass(); //Object
var_dump(preg_grep($value, $array));
echo "Done"
?>