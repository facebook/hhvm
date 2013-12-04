<?php
$a = new Phar(dirname(__FILE__) . '/files/sha1.phar');
$r = $a->getSignature();
var_dump($r['hash_type']);
$a = new Phar(dirname(__FILE__) . '/files/sha512.phar');
$r = $a->getSignature();
var_dump($r['hash_type']);
$a = new Phar(dirname(__FILE__) . '/files/sha256.phar');
$r = $a->getSignature();
var_dump($r['hash_type']);
$a = new Phar(dirname(__FILE__) . '/files/md5.phar');
$r = $a->getSignature();
var_dump($r['hash_type']);
$a = new Phar(dirname(__FILE__) . '/files/openssl.phar');
$r = $a->getSignature();
var_dump($r['hash_type']);
?>
===DONE===