<?php
$list = new SplDoublyLinkedList();
$list->push('oh');
$list->push('hai');
$list->push('thar');
$list->offsetUnset(2);
var_dump($list);
?>