<?php
/* Prototype  : string mcrypt_encrypt(string cipher, string key, string data, string mode, string iv)
 * Description: OFB crypt/decrypt data using key key with cipher cipher starting with iv 
 * Source code: ext/mcrypt/mcrypt.c
 * Alias to functions: 
 */

echo "*** Testing mcrypt_encrypt() : TripleDES functionality ***\n";

//test CBC, ECB modes
//test encrypt decrypt
//test tripledes, aes
//test different lengths of key, iv
//test no iv being passed on CBC, ECB 
//test upto 32 bytes with unlimited strength

$cipher = MCRYPT_TRIPLEDES;
$mode = MCRYPT_MODE_CBC;
$data = b'This is the secret message which must be encrypted';

// tripledes uses keys upto 192 bits (24 bytes)
$keys = array(
   b'12345678', 
   b'12345678901234567890', 
   b'123456789012345678901234', 
   b'12345678901234567890123456'
);
// tripledes is a block cipher of 64 bits (8 bytes)
$ivs = array(
   b'1234', 
   b'12345678', 
   b'123456789'
);


$iv = b'12345678';
echo "\n--- testing different key lengths\n";
foreach ($keys as $key) {
   echo "\nkey length=".strlen($key)."\n";
   var_dump(bin2hex(mcrypt_encrypt($cipher, $key, $data, $mode, $iv)));
}

$key = b'1234567890123456';  
echo "\n--- testing different iv lengths\n";
foreach ($ivs as $iv) {
   echo "\niv length=".strlen($iv)."\n";
   var_dump(bin2hex(mcrypt_encrypt($cipher, $key, $data, $mode, $iv)));
}

?>
===DONE===