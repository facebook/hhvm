<?php
class myAppendIterator extends AppendIterator {}
try {
	$it = new myAppendIterator();
	echo "no exception";
} catch (InvalidArgumentException $e) {
	echo 'InvalidArgumentException thrown';
}
?>