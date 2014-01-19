<?php
try {
	$p = new PharData(dirname(__FILE__) . '/files/tinylink.tar');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
echo $p['file.txt']->getContent();
echo $p['link.txt']->getContent();
?>
===DONE===