<?php
try {
	$phar = new PharData(dirname(__FILE__) . '/files/P1-1.0.0.tgz');
} catch (Exception $e) {
	echo $e->getMessage()."\n";
}

?>
===DONE===