<?php

var_dump(bin2hex(mcrypt_encrypt(MCRYPT_TRIPLEDES, "key", "data", MCRYPT_MODE_ECB)));
var_dump(bin2hex(mcrypt_encrypt(MCRYPT_TRIPLEDES, "key", "data", MCRYPT_MODE_ECB, "a")));
var_dump(bin2hex(mcrypt_encrypt(MCRYPT_TRIPLEDES, "key", "data", MCRYPT_MODE_ECB, "12345678")));

?>