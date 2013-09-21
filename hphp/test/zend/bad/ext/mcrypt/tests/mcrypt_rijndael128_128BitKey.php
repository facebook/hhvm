<?php
/* Prototype  : string mcrypt_encrypt(string cipher, string key, string data, string mode, string iv)
 * Description: OFB crypt/decrypt data using key key with cipher cipher starting with iv 
 * Source code: ext/mcrypt/mcrypt.c
 * Alias to functions: 
 */
 /* Prototype  : string mcrypt_decrypt(string cipher, string key, string data, string mode, string iv)
 * Description: OFB crypt/decrypt data using key key with cipher cipher starting with iv 
 * Source code: ext/mcrypt/mcrypt.c
 * Alias to functions: 
 */
 /* Prototype  : string mcrypt_cbc(int cipher, string key, string data, int mode, string iv)
 * Description: CBC crypt/decrypt data using key key with cipher cipher starting with iv 
 * Source code: ext/mcrypt/mcrypt.c
 * Alias to functions: 
 */

echo "*** Testing mcrypt : Rijndael128 functionality ***\n";

$cipher = MCRYPT_RIJNDAEL_128;
$mode = MCRYPT_MODE_CBC;
$data = b'This is the secret message which must be encrypted';

// keys upto 128 bits (16 bytes)
$keys = array(
   null, 
   '', 
   b'12345678', 
   b'1234567890123456'
);
// rijndael128 is a block cipher of 128 bits (16 bytes)
$ivs = array(
   null, 
   '', 
   b'12345678', 
   b'1234567890123456', 
   b'12345678901234567'
);


$iv = b'1234567890123456';
echo "\n--- testing different key lengths\n";
foreach ($keys as $key) {
   echo "\nkey length=".strlen($key)."\n";
   $res = mcrypt_encrypt($cipher, $key, $data, MCRYPT_MODE_CBC, $iv);
   var_dump(bin2hex($res));
   $res = mcrypt_cbc($cipher, $key, $res, MCRYPT_DECRYPT, $iv);
   var_dump(bin2hex($res));   
}

$key = b'1234567890123456';  
echo "\n--- testing different iv lengths\n";
foreach ($ivs as $iv) {
   echo "\niv length=".strlen($iv)."\n";
   $res = mcrypt_cbc($cipher, $key, $data, $mode, $iv);
   var_dump(bin2hex($res));
   $res = mcrypt_decrypt($cipher, $key, $res, MCRYPT_MODE_CBC, $iv);
   var_dump(bin2hex($res));   
}

?>
===DONE===