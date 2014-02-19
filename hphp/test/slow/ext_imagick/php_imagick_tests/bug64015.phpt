--TEST--
Test PHP bug #64015
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php
$im = new Imagick(dirname(__FILE__) . '/php.gif');
var_dump($im->getImageLength());

// Both should return filesize in bytes.

var_dump($im->getImageLength());

// All cases below now return 0;
$cloned_im = clone $im;
var_dump($im->getImageLength());

echo "OK" , PHP_EOL;


?>
--EXPECTF--
int(2523)
int(2523)
int(2523)
OK
