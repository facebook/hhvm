<?php
$dirname = 'DirectoryIterator_getOwner_basic';
mkdir($dirname);
$dir = new DirectoryIterator($dirname);
$expected = fileowner($dirname);
$actual = $dir->getOwner();
var_dump($expected == $actual);
?>
<?php error_reporting(0); ?>
<?php
$dirname = 'DirectoryIterator_getOwner_basic';
rmdir($dirname);
?>