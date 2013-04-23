<?php
$td  = mcrypt_module_open('rijndael-256', '', 'ecb', '');
$var = mcrypt_enc_get_supported_key_sizes($td);
var_dump($var);