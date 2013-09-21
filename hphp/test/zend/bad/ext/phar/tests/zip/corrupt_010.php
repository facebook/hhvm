<?php
try {
	new PharData(dirname(__FILE__) . '/files/disknumber.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===