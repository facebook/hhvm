<?php
class myNoRewindIterator extends NoRewindIterator  {}
try {
	$it = new myNoRewindIterator();
} catch (InvalidArgumentException $e) {
	echo 'InvalidArgumentException thrown';
}
?>