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
 /* Prototype  : string mcrypt_ecb(int cipher, string key, string data, int mode, string iv)
 * Description: ECB crypt/decrypt data using key key with cipher cipher starting with iv 
 * Source code: ext/mcrypt/mcrypt.c
 * Alias to functions: 
 */

echo "*** Testing mcrypt : Rijndael128 functionality ***\n";

$cipher = MCRYPT_RIJNDAEL_128;
$mode = MCRYPT_MODE_CBC;
$data = b'This is the secret message which must be encrypted';

// keys : 20 bytes, 24 bytes, 30 Bytes, 32 Bytes, 40 Bytes
$keys = array(
              b'12345678901234567890', 
              b'123456789012345678901234', 
              b'123456789012345678901234567890',
              b'12345678901234567890123456789012',
              b'1234567890123456789012345678901234567890'
 );
// rijndael128 is a block cipher of 128 bits (16 bytes)
$iv = b'1234567890123456';

echo "\n--- testing different key lengths\n";
foreach ($keys as $key) {
   echo "\nkey length=".strlen($key)."\n";
   $res = mcrypt_encrypt($cipher, $key, $data, MCRYPT_MODE_CBC, $iv);
   var_dump(bin2hex($res));
   $res = mcrypt_decrypt($cipher, $key, $res, MCRYPT_MODE_CBC, $iv);
   var_dump(bin2hex($res));   
}
?>
===DONE===