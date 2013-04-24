<?php
$list = new SplDoublyLinkedList();
$a = $list->offsetExists();
if(is_null($a)) {
	echo 'PASS';
}
?>