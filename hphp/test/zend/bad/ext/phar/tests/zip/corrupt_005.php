<?php
try {
	new PharData(dirname(__FILE__) . '/files/encrypted.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===