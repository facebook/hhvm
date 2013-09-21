<?php
$list = new SplDoublyLinkedList();
$list->push('oh');
$list->push('hai');
$list->push('thar');
echo $list->bottom() . "\n";
$list->offsetUnset(0);
echo $list->bottom() . "\n";
?>