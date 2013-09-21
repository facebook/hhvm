<?php
$link = __DIR__ . '/test_link';
symlink(__FILE__, $link );
$fileInfo = new SplFileInfo($link);

if ($fileInfo->isLink()) {
	echo $fileInfo->getLinkTarget() == __FILE__ ? 'same' : 'different',PHP_EOL;
}
var_dump(unlink($link));
?>