<?php
$td = mcrypt_module_open(MCRYPT_DES, '', MCRYPT_MODE_ECB, '');
echo mcrypt_generic($td,'aaaaaaaa');
print "I'm alive!\n";
?>