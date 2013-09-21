<?php
try {
	new PharData(dirname(__FILE__) . '/files/extralen_toolong.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===