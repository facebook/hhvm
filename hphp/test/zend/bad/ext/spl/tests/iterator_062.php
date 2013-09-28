<?php
class myRecursiveIteratorIterator extends RecursiveIteratorIterator {
	
}

try {
	$it = new myRecursiveIteratorIterator();
} catch (InvalidArgumentException $e) {
	echo 'InvalidArgumentException thrown';
}
?>