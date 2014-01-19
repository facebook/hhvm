<?php
   $td = mcrypt_module_open(MCRYPT_DES, '', MCRYPT_MODE_ECB, '');
   mcrypt_generic_init($td, 'aaaaaaaa', 'aaaaaaaa');
   mcrypt_generic_deinit($td);
   echo mcrypt_generic($td, 'aaaaaaaa');
?>