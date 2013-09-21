<?php
/* Prototype  : string mcrypt_encrypt(string cipher, string key, string data, string mode, string iv)
 * Description: OFB crypt/decrypt data using key key with cipher cipher starting with iv 
 * Source code: ext/mcrypt/mcrypt.c
 * Alias to functions: 
 */

echo "*** Testing mcrypt_encrypt() : error conditions ***\n";


//Test mcrypt_encrypt with one more than the expected number of arguments
echo "\n-- Testing mcrypt_encrypt() function with more than expected no. of arguments --\n";
$cipher = 'string_val';
$key = 'string_val';
$data = 'string_val';
$mode = 'string_val';
$iv = 'string_val';
$extra_arg = 10;
var_dump( mcrypt_encrypt($cipher, $key, $data, $mode, $iv, $extra_arg) );

// Testing mcrypt_encrypt with one less than the expected number of arguments
echo "\n-- Testing mcrypt_encrypt() function with less than expected no. of arguments --\n";
$cipher = 'string_val';
$key = 'string_val';
$data = 'string_val';
var_dump( mcrypt_encrypt($cipher, $key, $data) );

?>
===DONE===