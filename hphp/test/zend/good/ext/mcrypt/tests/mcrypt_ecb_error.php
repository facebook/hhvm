<?php
error_reporting(E_ALL & ~E_DEPRECATED);

/* Prototype  : string mcrypt_ecb(int cipher, string key, string data, int mode, string iv)
 * Description: ECB crypt/decrypt data using key key with cipher cipher starting with iv 
 * Source code: ext/mcrypt/mcrypt.c
 * Alias to functions: 
 */

echo "*** Testing mcrypt_ecb() : error conditions ***\n";


//Test mcrypt_ecb with one more than the expected number of arguments
echo "\n-- Testing mcrypt_ecb() function with more than expected no. of arguments --\n";
$cipher = 10;
$key = 'string_val';
$data = 'string_val';
$mode = 10;
$iv = 'string_val';
$extra_arg = 10;
var_dump( mcrypt_ecb($cipher, $key, $data, $mode, $iv, $extra_arg) );

// Testing mcrypt_ecb with one less than the expected number of arguments
echo "\n-- Testing mcrypt_ecb() function with less than expected no. of arguments --\n";
$cipher = 10;
$key = 'string_val';
$data = 'string_val';
var_dump( mcrypt_ecb($cipher, $key, $data) );

?>
===DONE===