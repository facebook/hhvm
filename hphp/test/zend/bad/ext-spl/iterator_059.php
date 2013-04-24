<?php
class myCachingIterator extends CachingIterator {
	
}
try {
	$it = new myCachingIterator();	
} catch (InvalidArgumentException $e) {
	echo 'InvalidArgumentException thrown';
}
?>