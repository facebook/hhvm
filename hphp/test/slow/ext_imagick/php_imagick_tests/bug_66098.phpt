--TEST--
Bug #66098  Segfault in zval_addref_p
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc');
--FILE--
<?php

$img = new Imagick();
echo $img->foobar;

echo "OK";

?>
--EXPECT--
OK
