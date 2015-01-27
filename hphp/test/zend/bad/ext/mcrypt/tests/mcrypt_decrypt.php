<?php
$key      = "0123456789012345";
$secret   = "PHP Testfest 2008";
$mode     = MCRYPT_MODE_CBC;
$cipher   = MCRYPT_RIJNDAEL_128;

$iv       = mcrypt_create_iv(mcrypt_get_iv_size($cipher, $mode), MCRYPT_RAND);
$enc_data = mcrypt_encrypt($cipher, $key, $secret, $mode, $iv);

// we have to trim as AES rounds the blocks and decrypt doesnt detect that
echo trim(mcrypt_decrypt($cipher, $key, $enc_data, $mode, $iv)) . "\n";

// a warning must be issued if we don't use a IV on a AES cipher, that usually requires an IV
var_dump(mcrypt_decrypt($cipher, $key, $enc_data, MCRYPT_MODE_CBC));

var_dump(mcrypt_decrypt(MCRYPT_BLOWFISH, "FooBar", $enc_data, MCRYPT_MODE_CBC, $iv));
