<?php
try {
	$phar = new PharData(dirname(__FILE__) . '/files/Net_URL-1.0.15.tgz');
	var_dump($phar->getStub());
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

?>
===DONE===