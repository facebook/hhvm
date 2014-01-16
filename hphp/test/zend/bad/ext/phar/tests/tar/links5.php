<?php
try {
	$p = new PharData(dirname(__FILE__) . '/files/subdirlink.tar');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
echo $p['hi/test.txt']->getContent();
echo $p['hi/link.txt']->getContent();
?>
===DONE===