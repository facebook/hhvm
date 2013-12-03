<?php
$dirname = 'DirectoryIterator_getGroup_basic';
mkdir($dirname);
$dir = new DirectoryIterator($dirname);
$expected = filegroup($dirname);
$actual = $dir->getGroup();
var_dump($expected == $actual);
?>
<?php
$dirname = 'DirectoryIterator_getGroup_basic';
rmdir($dirname);
?>