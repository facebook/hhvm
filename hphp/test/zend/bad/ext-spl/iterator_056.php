<?php
class myFilterIterator extends FilterIterator {
	function accept() {
		
	}
}
try {
	$it = new myFilterIterator();	
} catch (InvalidArgumentException $e) {
	echo 'InvalidArgumentException thrown';
}
?>