<?php
try {
	new PharData(dirname(__FILE__) . '/files/truncfilename.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===