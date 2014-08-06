<?php
try {
	new RecursiveTreeIterator(new ArrayIterator(array()));
} catch (InvalidArgumentException $e) {
	echo "InvalidArgumentException thrown\n";
}
?>
===DONE===