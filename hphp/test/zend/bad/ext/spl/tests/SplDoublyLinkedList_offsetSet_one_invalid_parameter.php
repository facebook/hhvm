<?php
$list = new SplDoublyLinkedList();
$a = $list->offsetSet(2);
if(is_null($a)) {
	echo 'PASS';
}
?>