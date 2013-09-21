<?php
try {
	new PharData(dirname(__FILE__) . '/files/stdin.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===