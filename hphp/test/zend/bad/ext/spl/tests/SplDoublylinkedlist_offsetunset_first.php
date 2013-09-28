<?php
$list = new SplDoublyLinkedList();
$list->push('oh');
$list->push('hai');
$list->push('thar');
$list->offsetUnset(0);
var_dump($list);
?>