<?php
try {
	new PharData(dirname(__FILE__) . '/files/nozipend.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===
