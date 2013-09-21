<?php
$iv1 = mcrypt_create_iv(mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_ECB), MCRYPT_RAND);
$iv2 = mcrypt_create_iv(mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_ECB), MCRYPT_DEV_URANDOM);
$iv3 = mcrypt_create_iv(mcrypt_get_iv_size(MCRYPT_RIJNDAEL_128, MCRYPT_MODE_ECB), MCRYPT_DEV_RANDOM);

echo strlen($iv1) . "\n";
echo strlen($iv2) . "\n";
echo strlen($iv3) . "\n";