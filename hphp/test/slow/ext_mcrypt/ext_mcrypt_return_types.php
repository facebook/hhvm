<?php

$td = mcrypt_module_open(MCRYPT_DES, '', MCRYPT_MODE_ECB, '');
mcrypt_generic_init($td, '', 'a');

var_dump(mcrypt_enc_get_algorithms_name($td));
var_dump(mcrypt_enc_get_block_size($td));
var_dump(mcrypt_enc_get_iv_size($td));
var_dump(mcrypt_enc_get_key_size($td));
var_dump(mcrypt_enc_get_modes_name($td));
var_dump(mcrypt_enc_get_supported_key_sizes($td));
var_dump(mcrypt_enc_self_test($td));
var_dump(mcrypt_generic_init($td, '', 'a'));
