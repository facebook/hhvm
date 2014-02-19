--TEST--
Test PHP emalloc/erealloc/efree
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc');

if (!Imagick::USE_ZEND_MM)
  die ('skip Only active when compiled with --enable-imagick-zend-mm');
?>
--FILE--
<?php
ini_set ('memory_limit', '1M');

$im = new imagick ();
for ($i = 0; $i < 100; $i++)
  $im->readImage ('magick:rose');


?>
--EXPECTF--
Fatal error: Allowed memory size of %d bytes exhausted (tried to allocate %d bytes) in %s on line %d
