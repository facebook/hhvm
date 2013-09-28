<?php
$td  = mcrypt_module_open('rijndael-128', '', MCRYPT_MODE_ECB, '');
echo mcrypt_enc_get_algorithms_name($td) . "\n";
$td  = mcrypt_module_open(MCRYPT_RIJNDAEL_128, '', MCRYPT_MODE_ECB, '');
echo mcrypt_enc_get_algorithms_name($td) . "\n";
$td  = mcrypt_module_open(MCRYPT_RC2, '', MCRYPT_MODE_CBC, '');
echo mcrypt_enc_get_algorithms_name($td) . "\n";
$td  = mcrypt_module_open(MCRYPT_BLOWFISH, '', MCRYPT_MODE_CBC, '');
echo mcrypt_enc_get_algorithms_name($td) . "\n";
$td  = mcrypt_module_open('des', '', 'ecb', '');
echo mcrypt_enc_get_algorithms_name($td) . "\n";