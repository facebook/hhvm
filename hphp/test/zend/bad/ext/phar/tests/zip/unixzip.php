<?php
$a = new PharData(dirname(__FILE__) . '/files/zip.zip');
foreach ($a as $b) {
	if ($b->isDir()) {
		echo "dir " . $b->getPathName() . "\n";
	} else {
		echo $b->getPathName(), "\n";
		echo file_get_contents($b->getPathName()), "\n";
	}
}
if (isset($a['notempty/hi.txt'])) {
	echo $a['notempty/hi.txt']->getPathName() . "\n";
}
?>
===DONE===