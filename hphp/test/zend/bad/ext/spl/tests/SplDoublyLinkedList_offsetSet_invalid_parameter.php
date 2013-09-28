<?php
$list = new SplDoublyLinkedList();
$a = $list->offsetSet();
if(is_null($a)) {
	echo 'PASS';
}
?>