<?php
$file = dirname(__FILE__) . '/src.wbmp';

$im2 = imagecreatefromwbmp($file);
echo 'test create from wbmp: ';
echo imagecolorat($im2, 3,3) == 0x0 ? 'ok' : 'failed';
echo "\n";
?>
