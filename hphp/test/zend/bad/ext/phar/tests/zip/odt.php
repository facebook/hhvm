<?php
$a = new PharData(dirname(__FILE__) . '/files/odt.odt');
foreach (new RecursiveIteratorIterator($a, RecursiveIteratorIterator::LEAVES_ONLY) as $b) {
	if ($b->isDir()) {
		echo "dir " . $b->getPathName() . "\n";
	} else {
		echo $b->getPathName() . "\n";
	}
}
// this next line is for increased code coverage
try {
	$b = new Phar(dirname(__FILE__) . '/files/odt.odt');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===