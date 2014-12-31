<?php
$filename = __DIR__ . "/SplFileInfo_getOwner_basic";
touch($filename);
$fileInfo = new SplFileInfo($filename);
$expected = fileowner($filename);
$actual = $fileInfo->getOwner();
var_dump($expected == $actual);
?>
<?php error_reporting(0); ?>
<?php
$filename = __DIR__ . "/SplFileInfo_getOwner_basic";
unlink($filename);
?>