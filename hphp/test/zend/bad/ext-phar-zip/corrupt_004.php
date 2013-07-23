<?php
try {
	new PharData(dirname(__FILE__) . '/files/cdir_offset.zip');
} catch (Exception $e) {
	echo $e->getMessage() . "\n";
}
?>
===DONE===