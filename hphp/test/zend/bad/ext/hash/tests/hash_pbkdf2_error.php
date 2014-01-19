<?php

/* {{{ proto string hash_pbkdf2(string algo, string password, string salt, int iterations [, int length = 0, bool raw_output = false])
Generate a PBKDF2 hash of the given password and salt
Returns lowercase hexbits by default */

echo "*** Testing hash_pbkdf2() : error conditions ***\n";

$password = 'password';
$salt = 'salt';

echo "\n-- Testing hash_pbkdf2() function with less than expected no. of arguments --\n";
var_dump(@hash_pbkdf2());
echo $php_errormsg . "\n";
var_dump(@hash_pbkdf2('crc32'));
echo $php_errormsg . "\n";
var_dump(@hash_pbkdf2('crc32', $password));
echo $php_errormsg . "\n";
var_dump(@hash_pbkdf2('crc32', $password, $salt));
echo $php_errormsg . "\n";

echo "\n-- Testing hash_pbkdf2() function with more than expected no. of arguments --\n";
var_dump(@hash_pbkdf2('crc32', $password, $salt, 10, 10, true, 'extra arg'));
echo $php_errormsg . "\n";

echo "\n-- Testing hash_pbkdf2() function with invalid hash algorithm --\n";
var_dump(@hash_pbkdf2('foo', $password, $salt, 1));
echo $php_errormsg . "\n";

echo "\n-- Testing hash_pbkdf2() function with invalid iterations --\n";
var_dump(@hash_pbkdf2('md5', $password, $salt, 0));
echo $php_errormsg . "\n";
var_dump(@hash_pbkdf2('md5', $password, $salt, -1));
echo $php_errormsg . "\n";

echo "\n-- Testing hash_pbkdf2() function with invalid length --\n";
var_dump(@hash_pbkdf2('md5', $password, $salt, 1, -1));
echo $php_errormsg . "\n\n";

?>
===Done===