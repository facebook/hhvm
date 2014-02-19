--TEST--
Test countable interface
--SKIPIF--
<?php require_once(dirname(__FILE__) . '/skipif.inc');

if (!extension_loaded ('spl'))
  die ('skip SPL is needed');
?>
--FILE--
<?php


$imagick = new Imagick(array (
            'magick:rose',
            'magick:rose',
            'magick:rose',
));

echo count ($imagick) . PHP_EOL;
echo 'done' . PHP_EOL;
?>
--EXPECT--
3
done
