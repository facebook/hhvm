<?php
class myLimitIterator extends LimitIterator {
	
}
try {
	$it = new myLimitIterator();
} catch (InvalidArgumentException $e) {
	echo 'InvalidArgumentException thrown';
}
?>