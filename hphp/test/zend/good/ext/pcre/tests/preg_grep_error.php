<?php
/*
* proto array preg_grep(string regex, array input [, int flags])
* Function is implemented in ext/pcre/php_pcre.c
*/
echo "*** Testing preg_grep() : error conditions ***\n";
// Zero arguments
echo "\n-- Testing preg_grep() function with Zero arguments --\n";
var_dump(preg_grep());
//Test preg_grep with one more than the expected number of arguments
echo "\n-- Testing preg_grep() function with more than expected no. of arguments --\n";
$regex = '/\d/';
$input = array(1, 2);
$flags = 0;
$extra_arg = 10;
var_dump(preg_grep($regex, $input, $flags, $extra_arg));
// Testing preg_grep withone less than the expected number of arguments
echo "\n-- Testing preg_grep() function with less than expected no. of arguments --\n";
$regex = 'string_val';
var_dump(preg_grep($regex));
echo "Done"
?>