<?php
try {
	new RecursiveTreeIterator();
} catch (InvalidArgumentException $e) {
	echo "InvalidArgumentException thrown\n";
}
?>
===DONE===