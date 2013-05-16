<?php
var_dump(mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_CBC));
var_dump(mcrypt_get_iv_size(MCRYPT_3DES, MCRYPT_MODE_CBC));
var_dump(mcrypt_get_iv_size(MCRYPT_WAKE, MCRYPT_MODE_STREAM));
var_dump(mcrypt_get_iv_size(MCRYPT_XTEA, MCRYPT_MODE_STREAM));