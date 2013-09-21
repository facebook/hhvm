<?php
$td = mcrypt_module_open(MCRYPT_RIJNDAEL_256, '', MCRYPT_MODE_ECB, '');
var_dump(mcrypt_enc_is_block_algorithm_mode($td));
$td = mcrypt_module_open(MCRYPT_RIJNDAEL_256, '', MCRYPT_MODE_CBC, '');
var_dump(mcrypt_enc_is_block_algorithm_mode($td));
$td = mcrypt_module_open(MCRYPT_WAKE, '',  MCRYPT_MODE_STREAM, '');
var_dump(mcrypt_enc_is_block_algorithm_mode($td));