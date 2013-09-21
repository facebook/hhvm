<?php
try {
	new PharData(dirname(__FILE__) . '/files/count1.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
try {
	new PharData(dirname(__FILE__) . '/files/count2.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===