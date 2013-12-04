<?php
$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';
$fname2 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip';
$fname3 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.3.phar.zip';
$fname4 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.4.phar.zip';
$fname5 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.5.phar.zip';
$fname6 = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.6.phar.zip';
$p = new Phar($fname);
$p['file1.txt'] = 'hi';
var_dump($p->getSignature());
$p->setSignatureAlgorithm(Phar::MD5);

copy($fname, $fname2);
$p = new Phar($fname2);
var_dump($p->getSignature());

$p->setSignatureAlgorithm(Phar::SHA1);

copy($fname2, $fname3);
$p = new Phar($fname3);
var_dump($p->getSignature());

try {
$p->setSignatureAlgorithm(Phar::SHA256);
copy($fname3, $fname4);
$p = new Phar($fname4);
var_dump($p->getSignature());
} catch (Exception $e) {
echo $e->getMessage();
}
try {
$p->setSignatureAlgorithm(Phar::SHA512);
copy($fname4, $fname5);
$p = new Phar($fname5);
var_dump($p->getSignature());
} catch (Exception $e) {
echo $e->getMessage();
}
try {
$config = dirname(__FILE__) . '/../files/openssl.cnf';
$config_arg = array('config' => $config);
$keys=openssl_pkey_new($config_arg);
openssl_pkey_export($keys, $privkey, NULL, $config_arg);
$pubkey=openssl_pkey_get_details($keys);
$p->setSignatureAlgorithm(Phar::OPENSSL, $privkey);

copy($fname5, $fname6);
file_put_contents($fname6 . '.pubkey', $pubkey['key']);
$p = new Phar($fname6);
var_dump($p->getSignature());
} catch (Exception $e) {
echo $e->getMessage();
}
?>
===DONE===
<?php
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.2.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.3.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.4.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.5.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.6.phar.zip');
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.6.phar.zip.pubkey');
?>