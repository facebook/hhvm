<?php
class myParentIterator extends ParentIterator {
	
}
try {
	$it = new myParentIterator();	
} catch (InvalidArgumentException $e) {
	echo 'InvalidArgumentException thrown';
}
?>