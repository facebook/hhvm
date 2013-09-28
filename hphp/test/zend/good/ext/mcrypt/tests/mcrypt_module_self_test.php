<?php
var_dump(mcrypt_module_self_test(MCRYPT_RIJNDAEL_128));
var_dump(mcrypt_module_self_test(MCRYPT_RC2));
var_dump(mcrypt_module_self_test(''));