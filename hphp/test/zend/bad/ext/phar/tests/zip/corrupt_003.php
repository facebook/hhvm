<?php
try {
	new PharData(dirname(__FILE__) . '/files/filecomment.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===