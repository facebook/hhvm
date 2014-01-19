<?php
try {
	$p = new PharData(dirname(__FILE__) . '/files/biglink.tar');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
echo $p['file.txt']->getContent();
echo $p['my/file']->getContent();
?>
===DONE===