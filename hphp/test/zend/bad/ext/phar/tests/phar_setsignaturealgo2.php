<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar';
$p = new Phar($fname);
$p['file1.txt'] = 'hi';
var_dump($p->getSignature());
$p->setSignatureAlgorithm(Phar::MD5);
var_dump($p->getSignature());
$p->setSignatureAlgorithm(Phar::SHA1);
var_dump($p->getSignature());
try {
$p->setSignatureAlgorithm(Phar::SHA256);
var_dump($p->getSignature());
} catch (Exception $e) {
echo $e->getMessage();
}
try {
$p->setSignatureAlgorithm(Phar::SHA512);
var_dump($p->getSignature());
} catch (Exception $e) {
echo $e->getMessage();
}
try {
$config = dirname(__FILE__) . '/files/openssl.cnf';
$config_arg = array('config' => $config);
$private = openssl_get_privatekey(file_get_contents(dirname(__FILE__) . '/files/private.pem'));
$pkey = '';
openssl_pkey_export($private, $pkey, NULL, $config_arg);
$p->setSignatureAlgorithm(Phar::OPENSSL, $pkey);
var_dump($p->getSignature());
} catch (Exception $e) {
echo $e->getMessage();
}
?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar');
?>