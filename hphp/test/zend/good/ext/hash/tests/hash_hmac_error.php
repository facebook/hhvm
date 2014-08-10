<?php
/*
* proto string hash_hmac ( string algo, string data, string key [, bool raw_output] )
* Function is implemented in ext/hash/hash.c
*/

echo "*** Testing hash_hmac() : error conditions ***\n";

$data = "This is a sample string used to test the hash_hmac function with various hashing algorithms";
$key = 'secret';

echo "\n-- Testing hash_hmac() function with less than expected no. of arguments --\n";
var_dump(hash_hmac());
var_dump(hash_hmac('crc32'));
var_dump(hash_hmac('crc32', $data));

echo "\n-- Testing hash_hmac() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump(hash_hmac('crc32', $data, $key, TRUE, $extra_arg));

echo "\n-- Testing hash_hmac() function with invalid hash algorithm --\n";
var_dump(hash_hmac('foo', $data, $key));

?>
===Done===
