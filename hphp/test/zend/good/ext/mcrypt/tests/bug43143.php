<?php
echo "ECB\n";
$input = 'to be encrypted';
$mkey = hash('sha256', 'secret key', TRUE);
$data = mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $mkey, $input, MCRYPT_MODE_ECB);
echo "CFB\n";
$input = 'to be encrypted';
$mkey = hash('sha256', 'secret key', TRUE);
$data = mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $mkey, $input, MCRYPT_MODE_CFB);
echo "END\n";
?>