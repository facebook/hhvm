<?php
class myRecursiveCachingIterator  extends RecursiveCachingIterator  {}
try {
	$it = new myRecursiveCachingIterator();
} catch (InvalidArgumentException $e) {
	echo 'InvalidArgumentException thrown';
}
?>