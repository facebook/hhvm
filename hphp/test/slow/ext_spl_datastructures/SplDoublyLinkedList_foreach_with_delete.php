<?php
$stack = new SplDoublyLinkedList();

$stack->setIteratorMode(SplDoublyLinkedList::IT_MODE_LIFO|SplDoublyLinkedList::IT_MODE_DELETE);

$stack->push(13);
$stack->push(17);
$stack->push(19);

foreach($stack as $item)
{
    var_dump($item);
}

var_dump(count($stack));

foreach($stack as $item)
{
    var_dump($item);
}

var_dump(count($stack));
