--TEST--
Imagick::setResourceLimit test
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc'); ?>
--FILE--
<?php

Imagick::setResourceLimit(Imagick::RESOURCETYPE_MEMORY, 64);

$imagick = new Imagick();
$imagick->setResourceLimit(Imagick::RESOURCETYPE_MEMORY, 64);

echo 'success';

?>
--EXPECTF--
success
